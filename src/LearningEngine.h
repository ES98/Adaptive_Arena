// Added by Gemini3, on 26.02.11
// Edited by PJS, on 26.02.11
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <cstddef>

namespace AdaptiveArena 
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief  사용자의 메모리 사용 패턴을 지수 이동 평균(EMA) 알고리즘으로 학습하는 엔진입니다.
     */
    class LearningEngine 
    {
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Constructor and Destructor
    public:
        /**
         * @brief  LearningEngine을 생성합니다.
         * @param  alpha  지속성 가중치 (0.0 ~ 1.0, 클수록 최신 데이터 반영률이 높음)
         */
        explicit LearningEngine(double alpha = 0.5);
        ~LearningEngine() = default;

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Public Methods
    public:
        /**
         * @brief  새로운 세션의 피크 사용량을 입력받아 가중 평균을 업데이트합니다.
         * @param  currentPeak  이번 세션의 최대 메모리 사용량 (Bytes)
         */
        void Update(size_t currentPeak);

        /**
         * @brief  학습된 예측 풀 크기를 반환합니다.
         * @return size_t  다음 세션에서 예약할 권장 메모리 크기
         */
        size_t GetPredictedSize() const;

        /**
         * @brief  학습 상태를 수동으로 설정합니다 (파일 로드 시 사용).
         * @param  size  복원할 학습 데이터
         */
        void SetState(size_t size);

    private:
        double m_alpha;
        size_t m_predictedSize;
    };

} // namespace AdaptiveArena
