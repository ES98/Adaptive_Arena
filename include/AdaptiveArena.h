// Added by Gemini3, on 26.02.11
// Edited by PJS, on 26.02.11
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <memory>
#include <string>
#include <filesystem>
#include <memory_resource>

namespace AdaptiveArena 
{
    class InternalResource; // Forward declaration

    /**
     * @brief  아레나의 작동 모드를 정의합니다.
     */
    enum class ArenaMode 
    {
        Generic,       ///< 일반적인 용도의 메모리 할당 (PMR 호환)
        UltrasoundRF   ///< 초음파 RF 데이터 처리 전용 (SoA, Zero-Copy, Ring Buffer)
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Resource Class
    // std::pmr::memory_resource를 래핑하거나 상속받아 지능형 메모리 풀 기능을 제공하는 주체입니다.
    class Resource : public std::pmr::memory_resource
    {
    public:
        virtual ~Resource() = default;

        // Resource 특유의 추가 기능 인터페이스 (예: Telemetry 제어)
        virtual void ResetLearning() = 0;
        virtual void SaveStatistics() = 0;

        // Telemetry Getters
        virtual size_t GetCurrentUsage() const = 0;
        virtual size_t GetPeakUsage() const = 0;
        virtual size_t GetPredictedSize() const = 0;

        // Ultrasound Mode Specific Telemetry (Optional for Generic)
        virtual size_t GetRingBufferSize() const { return 0; }
        virtual size_t GetRingBufferOccupancy() const { return 0; }
        virtual size_t GetPredictedSlotCount() const { return 0; }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Builder Class
    // Resource 생성을 위한 설정을 담당하며, 유효성 검증을 통해 시스템의 안정성을 보장합니다.
    class Builder 
    {
    public:
        Builder() : m_hardLimit(1024 * 1024 * 1024), m_mode(ArenaMode::Generic), m_gpuDirect(false) {}
        ~Builder() = default;

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /**
         * @brief  무결성 검증을 위한 사용자 비밀키를 설정합니다.
         * @param  key  사용자 정의 비밀 문자열
         * @return Builder& (Chaining 지원)
         */
        Builder& SetKey(const std::string& key)
        {
            m_secretKey = key;
            return *this;
        }

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /**
         * @brief  세션 데이터를 저장할 경로를 지정합니다.
         * @param  path  저장 파일 경로
         * @return Builder& (Chaining 지원)
         */
        Builder& SetPath(const std::filesystem::path& path)
        {
            m_logPath = path;
            return *this;
        }

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /**
         * @brief  메모리 풀이 사용할 물리적 상한선을 설정합니다.
         * @param  limitBytes  바이트 단위 상한선
         * @return Builder& (Chaining 지원)
         */
        Builder& SetHardLimit(size_t limitBytes)
        {
            m_hardLimit = limitBytes;
            return *this;
        }

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /**
         * @brief  아레나의 작동 모드를 설정합니다.
         * @param  mode  작동 모드 (Generic, UltrasoundRF 등)
         * @return Builder& (Chaining 지원)
         */
        Builder& SetMode(ArenaMode mode)
        {
            m_mode = mode;
            return *this;
        }

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /**
         * @brief  GPU Zero-Copy 기술(GPUDirect 등) 활성화 여부를 설정합니다.
         * @param  enable  활성화 여부
         * @return Builder& (Chaining 지원)
         */
        Builder& SetGpuDirect(bool enable)
        {
            m_gpuDirect = enable;
            return *this;
        }

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /**
         * @brief  설정된 파라미터를 기반으로 Resource 객체를 생성합니다.
         * @return std::unique_ptr<Resource> 생성된 자원 객체
         * @throw  std::runtime_error 필수 설정 누락 시 발생
         */
        std::unique_ptr<Resource> Build();

    private:
        std::string m_secretKey;
        std::filesystem::path m_logPath = "arena_session.bin";
        size_t m_hardLimit;
        ArenaMode m_mode;
        bool m_gpuDirect;
    };

} // namespace AdaptiveArena
