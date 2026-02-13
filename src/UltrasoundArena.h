// Added by Gemini3, on 26.02.13
// Edited by PJS, on 26.02.13
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "InternalResource.h"
#include <vector>
#include <atomic>
#include <chrono>

namespace AdaptiveArena 
{
    /**
     * @brief 초음파 RF 데이터의 메타데이터를 담는 헤더 구조체 (SoA의 Header 영역)
     */
    struct PacketHeader 
    {
        uint64_t timestamp;
        uint32_t frameIndex;
        uint32_t channelCount;
        uint32_t sampleDepth;
        uint32_t flags;
    };

    /**
     * @brief  초음파 RF 데이터 처리에 최적화된 고성능 아레나입니다.
     *         Header-Payload Separation (SoA), Jitter-Adaptive Ring Buffer, Zero-Copy를 지원합니다.
     */
    class UltrasoundArena : public InternalResource 
    {
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Constructor and Destructor
    public:
        UltrasoundArena(const std::string& secretKey, 
                         const std::filesystem::path& logPath, 
                         size_t hardLimit,
                         bool gpuDirect);
        ~UltrasoundArena() override;

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Specialized API for Ultrasound
    public:
        /**
         * @brief  데이터 스트림 처리를 위한 링 버퍼 슬롯을 예약합니다.
         * @param  headerSize   헤더의 크기 (SoA Header 영역)
         * @param  payloadSize  RF 데이터의 크기 (SoA Payload 영역)
         * @param  initialSlots 초기 슬롯 개수
         */
        void InitializeRing(size_t headerSize, size_t payloadSize, size_t initialSlots);

        /**
         * @brief  다음 쓰기 슬롯의 위치를 반환합니다 (Producer).
         */
        size_t GetNextWriteIndex();

        /**
         * @brief  다음 읽기 슬롯의 위치를 반환합니다 (Consumer).
         */
        size_t GetNextReadIndex();

        /**
         * @brief  현재 큐에 쌓여있는 지연(Lag) 프레임 수를 반환합니다.
         */
        size_t GetCurrentLag() const;

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Telemetry Overrides
    public:
        size_t GetRingBufferSize() const override { return m_slotCount; }
        size_t GetRingBufferOccupancy() const override { return GetCurrentLag(); }
        size_t GetPredictedSlotCount() const override;

        double GetAverageThroughputGBs() const override { return m_avgThroughputGBs; }
        bool IsPoolWarmedUp() const override { return m_slotCount >= 4; }

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // PMR Overrides (Internal Logic)
    protected:
        void* do_allocate(size_t bytes, size_t alignment) override;
        void do_deallocate(void* p, size_t bytes, size_t alignment) override;

    private:
        /**
         * @brief  버퍼 지격을 모니터링하고 필요시 링 버퍼를 확장합니다.
         */
        void AdaptToJitter();

        /**
         * @brief  Pinned Memory (Page-Locked) 할당을 수행합니다.
         */
        void* AllocatePinned(size_t size);
        void FreePinned(void* p, size_t size);

    private:
        bool m_gpuDirect;
        
        // SoA Pools
        std::vector<void*> m_headers;   // CPU-side cached headers
        std::vector<void*> m_payloads;  // GPU-side pinned payloads (Super-pages)
        
        size_t m_headerSize;
        size_t m_payloadSize;
        
        std::atomic<size_t> m_slotCount;
        std::atomic<size_t> m_writeIndex;
        std::atomic<size_t> m_readIndex;

        // Monitoring
        std::atomic<size_t> m_totalBytesProcessed;
        double m_avgThroughputGBs;
        std::chrono::steady_clock::time_point m_lastThroughputCheck;

        // Monitoring thread or point-in-time check
        std::chrono::steady_clock::time_point m_lastAdaptTime;
    };

} // namespace AdaptiveArena
