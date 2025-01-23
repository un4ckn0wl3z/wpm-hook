#pragma once

#include "windows.h"
#include "ios"
#include "fstream"

#include "MinHook.h"


// Function pointer for the original WriteProcessMemory
typedef BOOL(WINAPI* WriteProcessMemory_t)(
    HANDLE  hProcess,
    LPVOID  lpBaseAddress,
    LPCVOID lpBuffer,
    SIZE_T  nSize,
    SIZE_T* lpNumberOfBytesWritten
    );

// Pointer to store the original function
WriteProcessMemory_t OriginalWriteProcessMemory = nullptr;

// Hooked function
BOOL WINAPI HookedWriteProcessMemory(
    HANDLE  hProcess,
    LPVOID  lpBaseAddress,
    LPCVOID lpBuffer,
    SIZE_T  nSize,
    SIZE_T* lpNumberOfBytesWritten
) {
    // Open a file to log the parameters
    std::ofstream logFile("WriteProcessMemoryLog.txt", std::ios::app);
    if (logFile.is_open()) {
        logFile << "WriteProcessMemory Hooked!\n";
        logFile << "hProcess: " << hProcess << "\n";
        logFile << "lpBaseAddress: " << lpBaseAddress << "\n";
        logFile << "lpBuffer: " << lpBuffer << "\n";
        logFile << "nSize: " << nSize << "\n";

        // Dump the buffer content if applicable
        if (lpBuffer && nSize > 0) {
            logFile << "Buffer Content (Hex): ";
            auto buffer = static_cast<const BYTE*>(lpBuffer);
            for (SIZE_T i = 0; i < nSize; ++i) {
                logFile << std::hex << (int)buffer[i] << " ";
            }
            logFile << "\n";
        }
        logFile.close();
    }

    // Call the original WriteProcessMemory function
    return OriginalWriteProcessMemory(hProcess, lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesWritten);
}

// DLL Entry Point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ulReasonForCall, LPVOID lpReserved) {
    if (ulReasonForCall == DLL_PROCESS_ATTACH) {
        // Initialize MinHook
        if (MH_Initialize() != MH_OK) {
            return FALSE;
        }

        // Create a hook for WriteProcessMemory
        if (MH_CreateHook(
            &WriteProcessMemory,
            &HookedWriteProcessMemory,
            reinterpret_cast<LPVOID*>(&OriginalWriteProcessMemory)) != MH_OK) {
            return FALSE;
        }

        // Enable the hook
        if (MH_EnableHook(&WriteProcessMemory) != MH_OK) {
            return FALSE;
        }
    }
    else if (ulReasonForCall == DLL_PROCESS_DETACH) {
        // Disable the hook
        MH_DisableHook(&WriteProcessMemory);

        // Uninitialize MinHook
        MH_Uninitialize();
    }
    return TRUE;
}
