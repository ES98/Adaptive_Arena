#include "LearningEngine.h"
#include <algorithm>

namespace AdaptiveArena 
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructor
    LearningEngine::LearningEngine(double alpha) 
        : m_alpha(std::clamp(alpha, 0.0, 1.0))
        , m_predictedSize(0)
        , m_predictedSlots(4) // 최소 4개 슬롯에서 시작
    {
        // 초기화 시 예측값은 0으로 시작
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Update
    void LearningEngine::Update(size_t currentPeak) 
    {
        // 지수 이동 평균 수식 적용: S_next = α * P_current + (1 - α) * S_current
        // 데이터가 처음 들어올 때는 현재 피크값을 그대로 초기값으로 사용
        if (m_predictedSize == 0) 
        {
            m_predictedSize = currentPeak;
        }
        else 
        {
            m_predictedSize = static_cast<size_t>(m_alpha * currentPeak + (1.0 - m_alpha) * m_predictedSize);
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // GetPredictedSize
    size_t LearningEngine::GetPredictedSize() const 
    {
        return m_predictedSize;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // SetState
    void LearningEngine::SetState(size_t size) 
    {
        m_predictedSize = size;
        // 슬롯 수는 세션 피크 메모리로부터 유추하거나 별도 저장 가능하지만,
        // 여기서는 단순함을 위해 사이즈에 비례하거나 기본값을 유지합니다.
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // UpdateJitter
    void LearningEngine::UpdateJitter(size_t currentLag) 
    {
        // 지터 대응 EMA: 지격(Lag)의 피크를 학습
        // 슬롯 개수는 정수여야 하므로 반올림 또는 올림 처리
        // 예측 슬롯 = α * 현재지격 + (1 - α) * 이전예측
        double nextSlots = m_alpha * static_cast<double>(currentLag) + (1.0 - m_alpha) * static_cast<double>(m_predictedSlots);
        
        m_predictedSlots = std::max(size_t(4), static_cast<size_t>(nextSlots + 0.5));
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // GetPredictedSlotCount
    size_t LearningEngine::GetPredictedSlotCount() const 
    {
        return m_predictedSlots;
    }

} // namespace AdaptiveArena
