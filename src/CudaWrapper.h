#pragma once

#include <windows.h>
#include <string>
#include <iostream>
#include <optional>
#include <functional>

namespace AdaptiveArena 
{
    // CUDA 함수 포인터 정의 (필요한 함수만 서명)
    using cudaError_t = int;
    constexpr int cudaSuccess = 0;
    
    // cudaHostAlloc Flags
    constexpr unsigned int cudaHostAllocDefault = 0x00;
    constexpr unsigned int cudaHostAllocPortable = 0x01;
    constexpr unsigned int cudaHostAllocMapped = 0x02;
    constexpr unsigned int cudaHostAllocWriteCombined = 0x04;

    using PFN_cudaHostAlloc = cudaError_t(*)(void** pHost, size_t size, unsigned int flags);
    using PFN_cudaFreeHost = cudaError_t(*)(void* pHost);
    using PFN_cudaGetErrorString = const char*(*)(cudaError_t error);

    struct CudaFunctions 
    {
        PFN_cudaHostAlloc cudaHostAlloc;
        PFN_cudaFreeHost cudaFreeHost;
        PFN_cudaGetErrorString cudaGetErrorString;
    };

    class CudaWrapper 
    {
    public:
        static std::optional<CudaFunctions> LoadCudaLibrary() 
        {
            // 1. Try loading nvcuda.dll or cudart.dll
            HMODULE hModule = LoadLibraryA("cudart64_110.dll"); // Try specific version first or generalize
            if (!hModule) hModule = LoadLibraryA("cudart64_120.dll");
            if (!hModule) hModule = LoadLibraryA("nvcuda.dll"); // Driver API (might differ in signature)

            if (!hModule) 
            {
                std::cout << "[AdaptiveArena] CUDA library not found. Falling back to OS Pinned Memory." << std::endl;
                return std::nullopt;
            }

            // 2. Resolve Functions
            CudaFunctions funcs;
            funcs.cudaHostAlloc = (PFN_cudaHostAlloc)GetProcAddress(hModule, "cudaHostAlloc");
            funcs.cudaFreeHost = (PFN_cudaFreeHost)GetProcAddress(hModule, "cudaFreeHost");
            funcs.cudaGetErrorString = (PFN_cudaGetErrorString)GetProcAddress(hModule, "cudaGetErrorString");

            if (!funcs.cudaHostAlloc || !funcs.cudaFreeHost) 
            {
                std::cout << "[AdaptiveArena] CUDA found but missing required symbols. Falling back." << std::endl;
                FreeLibrary(hModule);
                return std::nullopt;
            }

            std::cout << "[AdaptiveArena] CUDA Detected! Enabling Zero-Copy (GPU Direct)." << std::endl;
            // Note: We intentionally leak hModule here for the lifetime of the app, 
            // or we could wrap it in a singleton to free on exit.
            return funcs; 
        }
    };
}
