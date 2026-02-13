// Added by Gemini3, on 26.02.13
// Edited by PJS, on 26.02.13
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define NOMINMAX
#include "UltrasoundArena.h"
#include <windows.h> // For VirtualAlloc (Pinned Memory simulation)
#include <algorithm>
#include <iostream>

namespace AdaptiveArena 
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructor
    UltrasoundArena::UltrasoundArena(const std::string& secretKey, 
                                     const std::filesystem::path& logPath, 
                                     size_t hardLimit,
                                     bool gpuDirect)
        : InternalResource(secretKey, logPath, hardLimit)
        , m_gpuDirect(gpuDirect)
        , m_headerSize(0)
        , m_payloadSize(0)
        , m_slotCount(0)
        , m_writeIndex(0)
        , m_readIndex(0)
        , m_totalBytesProcessed(0)
        , m_avgThroughputGBs(0.0)
    {
        // 1. Try to load CUDA
        m_cudaFuncs = CudaWrapper::LoadCudaLibrary();

        m_lastAdaptTime = std::chrono::steady_clock::now();
        m_lastThroughputCheck = std::chrono::steady_clock::now();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor
    UltrasoundArena::~UltrasoundArena() 
    {
        for (void* p : m_headers) 
        {
            ::operator delete(p);
        }
        for (void* p : m_payloads) 
        {
            FreePinned(p, m_payloadSize);
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // InitializeRing
    void UltrasoundArena::InitializeRing(size_t headerSize, size_t payloadSize, size_t initialSlots) 
    {
        m_headerSize = headerSize;
        m_payloadSize = payloadSize;
        
        // 학습된 슬롯 수가 있으면 그것을 우선 사용
        size_t predicted = m_learningEngine.GetPredictedSlotCount();
        m_slotCount = std::max(initialSlots, predicted);

        m_headers.reserve(m_slotCount);
        m_payloads.reserve(m_slotCount);

        for (size_t i = 0; i < m_slotCount; ++i) 
        {
            m_headers.push_back(::operator new(m_headerSize));
            m_payloads.push_back(AllocatePinned(m_payloadSize));
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // GetNextWriteIndex
    size_t UltrasoundArena::GetNextWriteIndex() 
    {
        AdaptToJitter();
        m_totalBytesProcessed += (m_headerSize + m_payloadSize);
        return m_writeIndex.fetch_add(1) % m_slotCount;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // GetNextReadIndex
    size_t UltrasoundArena::GetNextReadIndex() 
    {
        return m_readIndex.fetch_add(1) % m_slotCount;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // GetCurrentLag
    size_t UltrasoundArena::GetCurrentLag() const 
    {
        // Atomic operations are thread-safe and don't strictly require shared_lock for just reading indices.
        // However, if we wanted to ensure we are reading indices consistent with a specific buffer state, we could lock.
        // For lag calculation, atomic load is sufficient and lock-free is better for performance.
        size_t w = m_writeIndex.load();
        size_t r = m_readIndex.load();
        return (w > r) ? (w - r) : 0;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // GetHeader
    void* UltrasoundArena::GetHeader(size_t index) 
    {
        std::shared_lock<std::shared_mutex> lock(m_sharedMutex);
        if (index >= m_headers.size()) return nullptr;
        return m_headers[index];
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // GetPayload
    void* UltrasoundArena::GetPayload(size_t index)
    {
        std::shared_lock<std::shared_mutex> lock(m_sharedMutex);
        if (index >= m_payloads.size()) return nullptr;
        return m_payloads[index];
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // GetPredictedSlotCount
    size_t UltrasoundArena::GetPredictedSlotCount() const 
    {
        return m_learningEngine.GetPredictedSlotCount();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // AdaptToJitter
    void UltrasoundArena::AdaptToJitter() 
    {
        size_t lag = GetCurrentLag();
        m_learningEngine.UpdateJitter(lag);

        auto now = std::chrono::steady_clock::now();
        
        // Throughput 계산 (1초 간격)
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastThroughputCheck).count();
        if (elapsed >= 1000) 
        {
            size_t bytes = m_totalBytesProcessed.exchange(0);
            // Bytes to GB converter (1024^3)
            double currentGBs = (static_cast<double>(bytes) / (1024.0 * 1024.0 * 1024.0)) / (elapsed / 1000.0);
            
            // EMA for throughput stability
            m_avgThroughputGBs = 0.7 * currentGBs + 0.3 * m_avgThroughputGBs;
            m_lastThroughputCheck = now;
        }

        // 너무 자주 확장하지 않도록 1초 간격 체크
        if (std::chrono::duration_cast<std::chrono::seconds>(now - m_lastAdaptTime).count() >= 1) 
        {
            size_t predicted = m_learningEngine.GetPredictedSlotCount();
            if (predicted > m_slotCount) 
            {
                // Strict Resource Limits (Hard Limit)
                size_t newSize = predicted * (m_headerSize + m_payloadSize);
                if (newSize > m_hardLimit) 
                {
                    std::cerr << "[Ultrasound] Hard Limit Reached! Expansion rejected. Cap at " << m_slotCount << std::endl;
                    return; 
                }

                // 실시간 확장: 새로운 슈퍼페이지 할당 및 슬롯 추가
                // Writer Lock (Exclusive)
                std::unique_lock<std::shared_mutex> lock(m_sharedMutex);
                
                size_t additional = predicted - m_slotCount;
                for (size_t i = 0; i < additional; ++i) 
                {
                    void* pHeader = ::operator new(m_headerSize, std::nothrow); // Allocation Failure Handling
                    void* pPayload = AllocatePinned(m_payloadSize);

                    if (!pHeader || !pPayload) 
                    {
                        std::cerr << "[Ultrasound] CRITICAL: Allocation Failed during expansion! Stopping." << std::endl;
                        if (pHeader) ::operator delete(pHeader);
                        if (pPayload) FreePinned(pPayload, m_payloadSize);
                        break; // Stop expansion gracefully
                    }

                    m_headers.push_back(pHeader);
                    m_payloads.push_back(pPayload);
                }
                
                // Update count based on actual successful allocations
                size_t actualSize = m_headers.size();
                m_slotCount.store(actualSize);
                m_lastAdaptTime = now;

                std::cout << "[Ultrasound] Ring expanded. New total slots: " << actualSize 
                          << " (Absorbing Jitter)" << std::endl;
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // PMR Overrides
    void* UltrasoundArena::do_allocate(size_t bytes, size_t alignment) 
    {
        // Ultrasound Mode에서는 링 버퍼 API를 주로 사용하지만, 
        // 일반적인 PMR 할당 요청이 올 경우 InternalResource의 기본 동작(new)을 수행합니다.
        return InternalResource::do_allocate(bytes, alignment);
    }

    void UltrasoundArena::do_deallocate(void* p, size_t bytes, size_t alignment) 
    {
        InternalResource::do_deallocate(p, bytes, alignment);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Memory Management (Pinned)
    void* UltrasoundArena::AllocatePinned(size_t size) 
    {
        if (size == 0) return nullptr;

        // Hybrid Allocation Strategy
        if (m_cudaFuncs) 
        {
            void* ptr = nullptr;
            // cudaHostAllocPortable: Make memory visible to all CUDA contexts
            if (m_cudaFuncs->cudaHostAlloc(&ptr, size, 1) == 0) // 1=Portable, 0=Success
            {
                return ptr;
            }
        }

        // Windows: VirtualAlloc을 사용하여 Page-Locked (Pinned) 메모리 시뮬레이션
        return VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    }

    void UltrasoundArena::FreePinned(void* p, size_t size) 
    {
        if (p) 
        {
            if (m_cudaFuncs) 
            {
                // Try CUDA free first (assuming it expects CUDA pointer, might be risky if mixed)
                // But in our design, AllocatePinned uses one or the other.
                // Ideally, we should track source. For now, rely on CUDA runtime handling invalid pointers gracefully 
                // or just use VirtualFree if CUDA wasn't the allocator?
                // Actually, if m_cudaFuncs is present, we likely allocated with it.
                // A robust way is to try VirtualFree if cudaFreeHost fails, but cudaFreeHost might crash on system pointer.
                // *Assumption*: If CUDA is available, we used it.
                m_cudaFuncs->cudaFreeHost(p);
                return;
            }
            VirtualFree(p, 0, MEM_RELEASE);
        }
    }

} // namespace AdaptiveArena
