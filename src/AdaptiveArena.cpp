// Added by Gemini3, on 26.02.11
// Edited by PJS, on 26.02.11
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "AdaptiveArena.h"
#include "../src/InternalResource.h"
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

        // InternalResource를 생성하여 반환 (Sweet Case First)
        // std::unique_ptr<InternalResource>는 std::unique_ptr<Resource>로 암시적 변환 가능
        return std::make_unique<InternalResource>(m_secretKey, m_logPath, m_hardLimit);
    }

} // namespace AdaptiveArena
