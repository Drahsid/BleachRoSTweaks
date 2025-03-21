#include "Memory.hpp"

namespace Memory
{
    void PatchBytes(std::uintptr_t address, const void* data, std::size_t size)
    {
        if (!data || size == 0) {
            throw MemoryError("Invalid data or size");
        }
        
        try {
            ScopedMemoryProtection protection(
                reinterpret_cast<void*>(address), 
                size, 
                PAGE_EXECUTE_READWRITE
            );
            
            std::memcpy(reinterpret_cast<void*>(address), data, size);
        }
        catch (const MemoryError& e) {
            throw MemoryError(std::string("PatchBytes failed: ") + e.what());
        }
    }

    std::vector<int> PatternToBytes(std::string_view signature)
    {
        auto bytes = std::vector<int>{};
        bytes.reserve(signature.size() / 2); // Approximate capacity

        for (auto i = 0u; i < signature.size(); ++i) {
            if (signature[i] == ' ') {
                continue; // Skip spaces
            }
            
            if (signature[i] == '?') {
                // Handle wildcards
                if (i + 1 < signature.size() && signature[i + 1] == '?') {
                    ++i; // Skip second '?' if present
                }
                bytes.push_back(-1);
            }
            else if (i + 1 < signature.size()) {
                // Parse hex byte
                char hex[3] = { signature[i], signature[i + 1], 0 };
                bytes.push_back(static_cast<int>(std::strtol(hex, nullptr, 16)));
                ++i; // Skip second character of the byte
            }
        }
        
        return bytes;
    }

    std::optional<std::uint8_t*> PatternScan(void* module, std::string_view signature)
    {
        if (!module) {
            return std::nullopt;
        }

        auto dosHeader = static_cast<PIMAGE_DOS_HEADER>(module);
        if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
            return std::nullopt;
        }

        auto ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(
            reinterpret_cast<std::uint8_t*>(module) + dosHeader->e_lfanew
        );
        
        if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) {
            return std::nullopt;
        }

        auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
        auto patternBytes = PatternToBytes(signature);
        auto scanBytes = reinterpret_cast<std::uint8_t*>(module);
        
        const auto patternSize = patternBytes.size();
        const auto patternData = patternBytes.data();
        
        for (auto i = 0ul; i < sizeOfImage - patternSize; ++i) {
            bool found = true;
            
            for (auto j = 0ul; j < patternSize; ++j) {
                if (patternData[j] != -1 && scanBytes[i + j] != patternData[j]) {
                    found = false;
                    break;
                }
            }
            
            if (found) {
                return &scanBytes[i];
            }
        }
        
        return std::nullopt;
    }

    static HMODULE GetThisDllHandle()
    {
        MEMORY_BASIC_INFORMATION info{};
        const auto fnAddress = reinterpret_cast<void*>(GetThisDllHandle);
        
        const size_t len = VirtualQueryEx(GetCurrentProcess(), fnAddress, &info, sizeof(info));
        if (len != sizeof(info)) {
            throw MemoryError("VirtualQueryEx failed");
        }
        
        return static_cast<HMODULE>(info.AllocationBase);
    }
}
