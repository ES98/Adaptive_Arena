// Added by Gemini3, on 26.02.13
// Edited by PJS, on 26.02.13
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "UltrasoundArena.h"
#define NOMINMAX
#include <windows.h> // For VirtualAlloc (Pinned Memory simulation)
#include <algorithm>

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
    {
        m_lastAdaptTime = std::chrono::steady_clock::now();
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
        size_t w = m_writeIndex.load();
        size_t r = m_readIndex.load();
        return (w > r) ? (w - r) : 0;
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

        // 너무 자주 확장하지 않도록 1초 간격 체크
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - m_lastAdaptTime).count() >= 1) 
        {
            size_t predicted = m_learningEngine.GetPredictedSlotCount();
            if (predicted > m_slotCount) 
            {
                // 실시간 확장: 새로운 슈퍼페이지 할당 및 슬롯 추가
                // Note: 실제 고성능 환경에서는 인덱스 계산을 위해 Mutex 없이 
                // 이중 버퍼링이나 원자적 포인터 교체를 사용하지만, 
                // 여기서는 안전한 확장을 위해 Mutex를 활용합니다.
                std::lock_guard<std::mutex> lock(m_mutex);
                
                size_t additional = predicted - m_slotCount;
                for (size_t i = 0; i < additional; ++i) 
                {
                    m_headers.push_back(::operator new(m_headerSize));
                    m_payloads.push_back(AllocatePinned(m_payloadSize));
                }
                
                m_slotCount.store(predicted);
                m_lastAdaptTime = now;

                std::cout << "[Ultrasound] Ring expanded. New total slots: " << predicted 
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

        // Windows: VirtualAlloc을 사용하여 Page-Locked (Pinned) 메모리 시뮬레이션
        // MEM_COMMIT | MEM_RESERVE를 사용하여 물리 메모리에 즉시 바인딩
        void* ptr = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        
        if (ptr && m_gpuDirect) 
        {
            // 실제 GPUDirect 환경이라면 여기서 드라이버 함수를 호출하여 
            // 버퍼를 GPU에 등록(Registration)하는 과정이 필요합니다.
        }

        return ptr;
    }

    void UltrasoundArena::FreePinned(void* p, size_t size) 
    {
        if (p) 
        {
            VirtualFree(p, 0, MEM_RELEASE);
        }
    }

} // namespace AdaptiveArena
