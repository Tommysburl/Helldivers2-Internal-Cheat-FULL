#include "pch.h"
#include "windows.h"
#include "iostream"
#include "string"
#include <Psapi.h>
#include <vector>
#include "xorstr.hpp"

namespace Memory {
    // Function to find a pattern in the memory of a module
    uintptr_t FindPattern(const char* moduleName, const char* pattern) {
        HMODULE moduleHandle = GetModuleHandleA(moduleName);
        if (!moduleHandle) {
            std::cerr << _XOR_("Failed to get module handle.") << std::endl;
            return 0;
        }

        MODULEINFO moduleInfo;
        GetModuleInformation(GetCurrentProcess(), moduleHandle, &moduleInfo, sizeof(moduleInfo));
        uintptr_t moduleBase = reinterpret_cast<uintptr_t>(moduleInfo.lpBaseOfDll);

        // Convert the pattern string to bytes
        std::vector<uint8_t> patternBytes;
        for (size_t i = 0; i < strlen(pattern); i += 3) {
            if (pattern[i] == '?' && pattern[i + 1] == '?') {
                patternBytes.push_back(0x00);  // Use a specific wildcard byte
            }
            else {
                std::string byteString = { pattern[i], pattern[i + 1] };
                patternBytes.push_back(static_cast<uint8_t>(std::stoi(byteString, nullptr, 16)));
            }
        }

        // Search for the pattern in the module's memory
        for (uintptr_t i = 0; i < moduleInfo.SizeOfImage - patternBytes.size(); ++i) {
            bool found = true;
            for (size_t j = 0; j < patternBytes.size(); ++j) {
                if (patternBytes[j] != 0x00 && patternBytes[j] != *reinterpret_cast<uint8_t*>(moduleBase + i + j)) {
                    found = false;
                    break;
                }
            }

            if (found) {
                return (uintptr_t)moduleBase + i;
            }
        }

        std::cerr << _XOR_("Pattern not found.") << std::endl;
        return 0;
    }

    // Function to NOP out a section of memory
    void Nop(LPVOID dst, unsigned int size) {
        DWORD oldprotect;
        VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldprotect);
        memset(dst, 0x90, size);
        VirtualProtect(dst, size, oldprotect, &oldprotect);
    }

    // Function to patch a section of memory with new bytes
    void Patch(LPVOID dst, BYTE* src, unsigned int size) {
        DWORD oldprotect;
        VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldprotect);
        memcpy(dst, src, size);
        VirtualProtect(dst, size, oldprotect, &oldprotect);
    }

    // Function to allocate memory
    LPVOID AllocateMemory(uintptr_t targetAddress, size_t size) {
        return VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    }

    // Function to convert a byte array to a string of hexadecimal characters
    void ByteArrayToByteString(const unsigned char* byteArray, size_t size) {
        for (size_t i = 0; i < size; ++i) {
            printf("%02X", static_cast<int>(byteArray[i]));
        }
        printf("\n");
    }

    // Function to create a trampoline
    void CreateTrampoline(uintptr_t targetAddress, LPVOID trampolineAddress) {
        BYTE jmpInstruction[] = { 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
        ULONG_PTR relativeOffset = reinterpret_cast<ULONG_PTR>(trampolineAddress);
        memcpy(&jmpInstruction[6], &relativeOffset, sizeof(relativeOffset));
        Patch((LPVOID)targetAddress, jmpInstruction, 14);
    }

    // Function to find the index of a pattern in an array
    int FindPatternIndex(const unsigned char* data, size_t dataSize, const unsigned char* pattern, size_t patternSize) {
        for (size_t i = 0; i < dataSize - patternSize; ++i) {
            if (memcmp(&data[i], pattern, patternSize) == 0) {
                return static_cast<int>(i);
            }
        }
        return -1; // Pattern not found
    }

    // Helper function to get the length of an array
    template <typename T, size_t N>
    constexpr size_t ArrayLength(const T(&)[N]) {
        return N;
    }

    // Function to write assembly instructions to allocated memory
    void WriteAssemblyInstructions(uintptr_t allocatedMemory, uintptr_t originalAddress, BYTE* assemblyBytes, size_t assemblyBytesSize) {
        memcpy(&assemblyBytes[assemblyBytesSize - 8], &originalAddress, sizeof(originalAddress));
        Patch((LPVOID)allocatedMemory, assemblyBytes, assemblyBytesSize);
    }
}
