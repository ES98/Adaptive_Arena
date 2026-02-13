#pragma once

#include <filesystem>
#include <fstream>

namespace AdaptiveArena 
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief  세션 데이터(학습 결과 등)를 파일로 저장하고 복원하는 관리자입니다.
     */
    class PersistenceManager 
    {
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Public Methods
    public:
        /**
         * @brief  데이터를 파일로 저장합니다 (바이너리).
         * @param  path  저장할 경로
         * @param  data  저장할 예측 사이즈 값
         * @return bool  성공 여부
         */
        static bool Save(const std::filesystem::path& path, size_t data) 
        {
            std::ofstream file(path, std::ios::binary);
            if (!file.is_open()) 
            {
                return false;
            }

            file.write(reinterpret_cast<const char*>(&data), sizeof(data));
            return true;
        }

        /**
         * @brief  파일에서 데이터를 불러옵니다.
         * @param  path  로드할 경로
         * @param  outData  복원된 데이터를 담을 변수
         * @return bool  성공 여부
         */
        static bool Load(const std::filesystem::path& path, size_t& outData) 
        {
            if (!std::filesystem::exists(path)) 
            {
                return false;
            }

            std::ifstream file(path, std::ios::binary);
            if (!file.is_open()) 
            {
                return false;
            }

            file.read(reinterpret_cast<char*>(&outData), sizeof(outData));
            return true;
        }
    };

} // namespace AdaptiveArena
