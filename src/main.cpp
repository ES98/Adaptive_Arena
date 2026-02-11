// Added by Gemini3, on 26.02.11
// Edited by PJS, on 26.02.11
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "AdaptiveArena.h"
#include <iostream>
#include <vector>
#include <memory_resource>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  Adaptive Arena 시스템 초기화 및 인터페이스 검증을 위한 메인 진입점입니다.
 * @return int 실행 결과 코드
 */
int main()
{
    std::cout << "🏟️ [Adaptive Arena] Initializing Project..." << std::endl;

    try 
    {
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // 1. Interface Test: Builder Pattern
        std::cout << ">> Testing Builder Interface..." << std::endl;
        
        auto arena = AdaptiveArena::Builder()
                        .SetKey("MySecretKey123!")
                        .SetPath("./session.data")
                        .SetHardLimit(1024 * 1024 * 256) // 256MB
                        .Build();

        std::cout << ">> Builder validation passed (Resource ptr ready)." << std::endl;

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // 2. Future Integration: std::pmr::vector with Adaptive Arena
        // std::pmr::vector<int> data(arena.get());
        
        std::cout << ">> System initialization sequence completed." << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "!! Initialization Failed: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
