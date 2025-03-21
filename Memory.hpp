#pragma once
#include <cstdint>
#include <windows.h>
#include <vector>
#include <string_view>
#include <array>
#include <optional>
#include <type_traits>
#include <stdexcept>
#include <cassert>

namespace Memory
{
    class MemoryError : public std::runtime_error 
    {
    public:
        explicit MemoryError(const std::string& message) : std::runtime_error(message) {}
    };

    class ScopedMemoryProtection
    {
    private:
        void* address;
        std::size_t size;
        DWORD oldProtect;
        bool committed;

    public:
        ScopedMemoryProtection(void* address, std::size_t size, DWORD newProtect) : address(address), size(size), oldProtect(0), committed(false)
        {
            if (!VirtualProtect(address, size, newProtect, &oldProtect)) {
                throw MemoryError("Failed to change memory protection");
            }
            committed = true;
        }

        ~ScopedMemoryProtection()
        {
            if (committed) {
                DWORD unused;
                VirtualProtect(address, size, oldProtect, &unused);
            }
        }

        // Prevent copying and moving
        ScopedMemoryProtection(const ScopedMemoryProtection&) = delete;
        ScopedMemoryProtection& operator=(const ScopedMemoryProtection&) = delete;
        ScopedMemoryProtection(ScopedMemoryProtection&&) = delete;
        ScopedMemoryProtection& operator=(ScopedMemoryProtection&&) = delete;
        //
    };

    void PatchBytes(std::uintptr_t address, const void* data, std::size_t size);
    std::vector<int> PatternToBytes(std::string_view signature);
    std::optional<std::uint8_t*> PatternScan(void* module, std::string_view signature);
    HMODULE GetThisDllHandle();
}
