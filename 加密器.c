#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char base64_table[] = "Ag9jCXabcJKLV2345WmnopuvwxYZklMhi78NOPrstTUByz0defDEFGHI16+/QRSq";
static char key[] = "AeB&79!ra0(3*)";

void base64_encode(unsigned char *input, int length, unsigned char *output) {
    int i, j;
    for (i = 0, j = 0; i < length;) {
        unsigned int octet_a = i < length ? input[i++] : 0;
        unsigned int octet_b = i < length ? input[i++] : 0;
        unsigned int octet_c = i < length ? input[i++] : 0;

        unsigned int triple = (octet_a << 16) + (octet_b << 8) + octet_c;

        output[j++] = base64_table[(triple >> 18) & 0x3F];
        output[j++] = base64_table[(triple >> 12) & 0x3F];
        output[j++] = (i > length + 1) ? '=' : base64_table[(triple >> 6) & 0x3F];
        output[j++] = (i > length) ? '=' : base64_table[triple & 0x3F];
    }
    output[j] = '\0';
}

void xor_encrypt_decrypt(unsigned char *input, unsigned char *output, char *key, int len) {
    int key_length = strlen(key);
    for (int i = 0; i < len; i++) {
        output[i] = input[i] ^ key[i % key_length];
    }
    output[len] = '\0';
}
int main() {
    unsigned char buf[] = "your shellcode";

    unsigned char encoded[2048];
    unsigned char encrypted[2048];
    xor_encrypt_decrypt(buf, encrypted, key, sizeof(buf) / sizeof(buf[0]) - 1);
    base64_encode(encrypted, sizeof(buf) / sizeof(buf[0]), encoded);
    printf("shllcode密文: %s\n", encoded);
}

