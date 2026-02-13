#include "AdaptiveArena.h"
#include "../src/InternalResource.h"
#include "../src/UltrasoundArena.h"
#include <stdexcept>

namespace AdaptiveArena 
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Builder::Build Implementation
    std::unique_ptr<Resource> Builder::Build() 
    {
        // Zero-Trust Validation
        if (m_secretKey.empty())
        {
            throw std::runtime_error("Secret key is required for integrity verification.");
        }

        if (m_mode == ArenaMode::UltrasoundRF) 
        {
            // 초음파 모드 리소스 생성
            return std::make_unique<UltrasoundArena>(m_secretKey, m_logPath, m_hardLimit, m_gpuDirect);
        }
        else 
        {
            // 일반 모드 리소스 생성
            return std::make_unique<InternalResource>(m_secretKey, m_logPath, m_hardLimit);
        }
    }

} // namespace AdaptiveArena
