// Added by Gemini3, on 26.02.11
// Edited by PJS, on 26.02.11
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "AdaptiveArena.h"
#include <iostream>
#include <vector>
#include <memory_resource>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "AdaptiveArena.h"
#include "Visualizer.h"
#include <iostream>
#include <vector>
#include <random>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  실시간 시각화 대시보드를 포함한 Adaptive Arena 메인 테스트 루프입니다.
 */
int main()
{
    try 
    {
        // 1. Arena Initialization
        auto arena = AdaptiveArena::Builder()
                        .SetKey("VizTestKey!")
                        .SetPath("./viz_session.bin")
                        .SetHardLimit(1024 * 1024 * 512) // 512MB
                        .Build();

        // 2. Visualizer Initialization
        AdaptiveArena::Visualizer viz("🏟️ Adaptive Arena Dashboard", 1024, 768);

        // 3. Test Data (std::pmr::vector with our arena)
        std::pmr::vector<std::byte> pool_test(arena.get());
        
        // 4. Main GUI Loop
        while (!viz.ShouldClose()) 
        {
            viz.StartFrame();

            // Render Dashboard
            viz.RenderDashboard(arena.get());

            // 5. Interactive Test Controls (Extra Window)
            if (ImGui::Begin("🛠️ Allocation Simulator")) 
            {
                static int alloc_size_kb = 1024;
                ImGui::SliderInt("Alloc Size (KB)", &alloc_size_kb, 64, 1024 * 10);

                if (ImGui::Button("Simulate Allocation", ImVec2(-1, 40))) 
                {
                    // 시뮬레이션을 위해 벡터에 데이터 추가
                    size_t bytes = static_cast<size_t>(alloc_size_kb) * 1024;
                    for(size_t i = 0; i < bytes; ++i) 
                    {
                        pool_test.push_back(std::byte{0});
                    }
                }

                if (ImGui::Button("Clear All Allocations", ImVec2(-1, 40))) 
                {
                    pool_test.clear();
                    pool_test.shrink_to_fit();
                }

                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Arena will learn from the peak usage\nwhen you close the application.");
            }
            ImGui::End();

            viz.EndFrame();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
