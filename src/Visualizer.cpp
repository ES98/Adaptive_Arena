// Added by Gemini3, on 26.02.11
// Edited by PJS, on 26.02.11
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Visualizer.h"
#include <stdexcept>

namespace AdaptiveArena 
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructor
    Visualizer::Visualizer(const std::string& title, int width, int height) 
    {
        // 1. GLFW 초기화
        if (!glfwInit()) 
        {
            throw std::runtime_error("Failed to initialize GLFW");
        }

        // GL 3.0 + GLSL 130
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

        // 2. 윈도우 생성
        m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        if (!m_window) 
        {
            glfwTerminate();
            throw std::runtime_error("Failed to create GLFW window");
        }

        glfwMakeContextCurrent(m_window);
        glfwSwapInterval(1); // Enable vsync

        // 3. ImGui 초기화
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(m_window, true);
        ImGui_ImplOpenGL3_Init("#version 130");

        m_usageHistory.reserve(MAX_HISTORY);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor
    Visualizer::~Visualizer() 
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(m_window);
        glfwTerminate();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // ShouldClose
    bool Visualizer::ShouldClose() const 
    {
        return glfwWindowShouldClose(m_window);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // StartFrame
    void Visualizer::StartFrame() 
    {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // RenderDashboard
    void Visualizer::RenderDashboard(Resource* arena) 
    {
        if (!arena) return;

        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);

        if (ImGui::Begin("🏟️ Adaptive Arena Dashboard")) 
        {
            // 0. Mode Indicator
            size_t totalSlots = arena->GetRingBufferSize();
            bool isUltrasound = (totalSlots > 0);
            
            ImGui::TextColored(isUltrasound ? ImVec4(0.2f, 0.8f, 1.0f, 1.0f) : ImVec4(1.0f, 0.8f, 0.2f, 1.0f), 
                              "Mode: %s", isUltrasound ? "Ultrasound RF (Zero-Copy)" : "Generic PMR");
            ImGui::Separator();

            // 1. Statistics Summary
            ImGui::Text("System Status:");
            
            float currentMB = arena->GetCurrentUsage() / (1024.0f * 1024.0f);
            float peakMB = arena->GetPeakUsage() / (1024.0f * 1024.0f);
            float predictedMB = arena->GetPredictedSize() / (1024.0f * 1024.0f);

            ImGui::Columns(2, "StatsColumns");
            ImGui::Text("Current Usage:"); ImGui::NextColumn(); ImGui::Text("%.2f MB", currentMB); ImGui::NextColumn();
            ImGui::Text("Session Peak:"); ImGui::NextColumn(); ImGui::Text("%.2f MB", peakMB); ImGui::NextColumn();
            if (!isUltrasound) {
                ImGui::Text("EMA Prediction:"); ImGui::NextColumn(); ImGui::Text("%.2f MB", predictedMB); ImGui::NextColumn();
            }
            ImGui::Columns(1);

            if (isUltrasound) 
            {
                ImGui::Spacing();
                ImGui::Text("Ring Buffer (Jitter Control):");
                ImGui::Separator();
                
                size_t occupancy = arena->GetRingBufferOccupancy();
                size_t predictedSlots = arena->GetPredictedSlotCount();

                ImGui::Columns(2, "RingColumns");
                ImGui::Text("Total Slots:"); ImGui::NextColumn(); ImGui::Text("%zu", totalSlots); ImGui::NextColumn();
                ImGui::Text("Super-Pages:"); ImGui::NextColumn(); ImGui::Text("%zu", totalSlots); ImGui::NextColumn();
                ImGui::Text("Current Lag:"); ImGui::NextColumn(); 
                ImGui::TextColored(occupancy > totalSlots * 0.8 ? ImVec4(1,0,0,1) : ImVec4(1,1,1,1), "%zu", occupancy); 
                ImGui::NextColumn();
                ImGui::Text("Predicted Slots:"); ImGui::NextColumn(); ImGui::Text("%zu (EMA)", predictedSlots); ImGui::NextColumn();
                ImGui::Columns(1);
                
                // Jitter Graph
                static std::vector<float> jitterHistory;
                if (jitterHistory.size() > 100) jitterHistory.erase(jitterHistory.begin());
                jitterHistory.push_back(static_cast<float>(occupancy));
                
                ImGui::PlotLines("##JitterGraph", jitterHistory.data(), (int)jitterHistory.size(), 0, "Processing Lag (Frames)", 0.0f, static_cast<float>(totalSlots), ImVec2(0, 80));
            }

            // 2. Real-time Graph
            ImGui::Spacing();
            ImGui::Text("Memory Telemetry (Real-time):");
            
            // 히스토리 업데이트
            if (m_usageHistory.size() >= MAX_HISTORY) 
            {
                m_usageHistory.erase(m_usageHistory.begin());
            }
            m_usageHistory.push_back(currentMB);

            ImGui::PlotLines("##UsageGraph", m_usageHistory.data(), (int)m_usageHistory.size(), 0, "Usage (MB)", 0.0f, peakMB * 1.2f + 1.0f, ImVec2(0, 120));

            // 3. Actions
            ImGui::Spacing();
            ImGui::Separator();
            if (ImGui::Button("Reset Learning State", ImVec2(245, 30))) 
            {
                arena->ResetLearning();
            }

            ImGui::SameLine();
            if (ImGui::Button("Force Save Stats", ImVec2(245, 30))) 
            {
                arena->SaveStatistics();
            }
        }
        ImGui::End();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // EndFrame
    void Visualizer::EndFrame() 
    {
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(m_window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(m_window);
    }

} // namespace AdaptiveArena
