// Added by Gemini3, on 26.02.11
// Edited by PJS, on 26.02.11
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "LearningEngine.h"
#include <algorithm>

namespace AdaptiveArena 
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructor
    LearningEngine::LearningEngine(double alpha) 
        : m_alpha(std::clamp(alpha, 0.0, 1.0))
        , m_predictedSize(0)
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
    }

} // namespace AdaptiveArena
