// Added by Gemini3, on 26.02.11
// Edited by PJS, on 26.02.11
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "../include/AdaptiveArena.h"
#include "LearningEngine.h"
#include "PersistenceManager.h"
#include <atomic>
#include <mutex>
#include <iostream>

namespace AdaptiveArena 
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief  Adaptive Arena의 실제 작동을 담당하는 내부 리소스 구현체입니다.
     */
    class InternalResource : public Resource 
    {
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Constructor and Destructor
    public:
        explicit InternalResource(const std::string& secretKey, const std::filesystem::path& logPath, size_t hardLimit) 
            : m_secretKey(secretKey)
            , m_logPath(logPath)
            , m_hardLimit(hardLimit)
            , m_currentUsage(0)
            , m_peakUsage(0)
            , m_learningEngine(0.5) // Alpha default 0.5
        {
            // 1. 기존 세션 데이터 복원
            size_t savedSize = 0;
            if (PersistenceManager::Load(m_logPath, savedSize)) 
            {
                m_learningEngine.SetState(savedSize);
                std::cout << "[Internal] Session loaded. Predicted peak: " << savedSize << " bytes." << std::endl;
            }
            
            // TODO: m_learningEngine.GetPredictedSize()만큼 초기 풀 예약 로직 추가 예정
        }

        virtual ~InternalResource() 
        {
            // 세션 종료 시 마지막 통계 저장
            SaveStatistics();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Resource Interface Implementation
    public:
        void ResetLearning() override 
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_learningEngine.SetState(0);
        }

        void SaveStatistics() override 
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            // 이번 세션의 피크를 학습 엔진에 반영
            m_learningEngine.Update(m_peakUsage);
            
            // 파일로 저장
            if (PersistenceManager::Save(m_logPath, m_learningEngine.GetPredictedSize())) 
            {
                std::cout << "[Internal] Statistics saved. New predicted peak: " 
                          << m_learningEngine.GetPredictedSize() << " bytes." << std::endl;
            }
        }

        size_t GetCurrentUsage() const override { return m_currentUsage; }
        size_t GetPeakUsage() const override { return m_peakUsage; }
        size_t GetPredictedSize() const override { return m_learningEngine.GetPredictedSize(); }

    protected:
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /**
         * @brief  std::pmr::memory_resource 실제 할당 로직
         */
        void* do_allocate(size_t bytes, size_t alignment) override 
        {
            // 1. 실제 할당 (업스트림: 기본 new 사용 - 테스트용)
            void* ptr = ::operator new(bytes, std::align_val_t{alignment});

            if (ptr) 
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                
                // 2. Telemetry: 현재 및 피크 사용량 업데이트
                m_currentUsage += bytes;
                if (m_currentUsage > m_peakUsage) 
                {
                    m_peakUsage = m_currentUsage;
                }
            }

            return ptr;
        }

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /**
         * @brief  std::pmr::memory_resource 실제 해제 로직
         */
        void do_deallocate(void* p, size_t bytes, size_t alignment) override 
        {
            if (!p) return;

            // 1. 실제 해제
            ::operator delete(p, std::align_val_t{alignment});

            // 2. Telemetry: 현재 사용량 감소
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_currentUsage >= bytes) 
            {
                m_currentUsage -= bytes;
            }
        }

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /**
         * @brief  두 리소스가 동일한지 확인
         */
        bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override 
        {
            return this == &other;
        }

    private:
        std::string m_secretKey;
        std::filesystem::path m_logPath;
        size_t m_hardLimit;

        std::mutex m_mutex;
        size_t m_currentUsage;
        size_t m_peakUsage;

        LearningEngine m_learningEngine;
    };

} // namespace AdaptiveArena
