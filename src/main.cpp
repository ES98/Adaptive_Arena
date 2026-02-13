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
#include "AdaptiveArena.h"
#include "Visualizer.h"
#include "../src/UltrasoundArena.h" // For InitializeRing specialized API
#include <iostream>
#include <vector>
#include <thread>
#include <atomic>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  초음파 RF 데이터 처리 모드(Zero-Copy & Jitter-Adaptive) 시뮬레이션입니다.
 */
int main()
{
    try 
    {
        // 1. Arena Initialization (Ultrasound Mode)
        auto arena = AdaptiveArena::Builder()
                        .SetKey("UltrasoundRF_Key")
                        .SetPath("./ultrasound_session.bin")
                        .SetMode(AdaptiveArena::ArenaMode::UltrasoundRF)
                        .SetGpuDirect(true)
                        .Build();

        // Ultrasound 전용 API 사용을 위한 다운캐스트
        auto ultrasound = dynamic_cast<AdaptiveArena::UltrasoundArena*>(arena.get());
        if (ultrasound) 
        {
            // 헤더 512B, 페이로드 4MB (예시), 초기 슬롯 8개
            ultrasound->InitializeRing(512, 1024 * 1024 * 4, 8);
        }

        // 2. Visualizer Initialization
        AdaptiveArena::Visualizer viz("🏟️ Adaptive Arena Dashboard - Ultrasound Mode", 1280, 800);

        // 3. Simulation Variables
        std::atomic<bool> running{true};
        static int lag_simulation_ms = 0;
        
        // 4. Main GUI Loop
        while (!viz.ShouldClose()) 
        {
            viz.StartFrame();

            // Render Dashboard
            viz.RenderDashboard(arena.get());

            // 5. Ultrasound Simulator Controls
            if (ImGui::Begin("🛠️ RF Stream Simulator")) 
            {
                ImGui::Text("Simulation Controls:");
                ImGui::Separator();
                
                if (ImGui::Button("Push RF Frame (Producer)", ImVec2(-1, 50))) 
                {
                    if (ultrasound) ultrasound->GetNextWriteIndex();
                }

                ImGui::Spacing();
                ImGui::SliderInt("Processing Delay (ms)", &lag_simulation_ms, 0, 100);
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), 
                                   "Simulates Jitter by slowing down the Consumer.\nWatch 'Current Lag' graph on the left.");

                if (ImGui::Button("Burst 10 Frames", ImVec2(-1, 30))) 
                {
                    for(int i=0; i<10; ++i) if(ultrasound) ultrasound->GetNextWriteIndex();
                }

                ImGui::Spacing();
                ImGui::Separator();
                
                // Background Consumer Simulation logic (in-loop for simplicity)
                static auto last_process_time = std::chrono::steady_clock::now();
                auto now = std::chrono::steady_clock::now();
                int elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_process_time).count();
                
                // 보통 30fps로 처리한다고 가정(33ms), 하지만 Slider에 의한 딜레이 추가
                if (elapsed >= (33 + lag_simulation_ms)) 
                {
                    if (ultrasound && ultrasound->GetCurrentLag() > 0) 
                    {
                        ultrasound->GetNextReadIndex();
                    }
                    last_process_time = now;
                }
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
