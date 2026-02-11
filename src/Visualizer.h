// Added by Gemini3, on 26.02.11
// Edited by PJS, on 26.02.11
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "AdaptiveArena.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <vector>
#include <string>

namespace AdaptiveArena 
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief  Adaptive Arena의 상태를 실시간으로 시각화하는 대시보드 클래스입니다.
     */
    class Visualizer 
    {
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Constructor and Destructor
    public:
        Visualizer(const std::string& title, int width, int height);
        ~Visualizer();

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Public Methods
    public:
        /**
         * @brief  윈도우가 닫혀야 하는지 확인합니다.
         */
        bool ShouldClose() const;

        /**
         * @brief  프레임을 시작하고 이벤트를 처리합니다.
         */
        void StartFrame();

        /**
         * @brief  Arena 리소스의 상태를 UI로 렌더링합니다.
         */
        void RenderDashboard(Resource* arena);

        /**
         * @brief  프레임을 종료하고 화면을 갱신합니다.
         */
        void EndFrame();

    private:
        GLFWwindow* m_window;
        std::vector<float> m_usageHistory;
        const size_t MAX_HISTORY = 200;
    };

} // namespace AdaptiveArena
