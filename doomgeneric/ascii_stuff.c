#include <inttypes.h>

int get(int i, uint8_t image_average) {
    int min_val = 175, max_val = 255;
    int temp = (i - image_average + min_val) * (max_val - min_val)/max_val + min_val;
    if (temp <= 175)
        return 58;
    if (temp <= 177)
        return 48;
    if (temp <= 183)
        return 37;
    if (temp <= 188)
        return 40;
    if (temp <= 189)
        return 43;
    if (temp <= 190)
        return 53;
    if (temp <= 192)
        return 22;
    if (temp <= 193)
        return 11;
    if (temp <= 194)
        return 13;
    if (temp <= 196)
        return 59;
    if (temp <= 197)
        return 84;
    if (temp <= 198)
        return 17;
    if (temp <= 199)
        return 42;
    if (temp <= 200)
        return 8;
    if (temp <= 201)
        return 4;
    if (temp <= 202)
        return 6;
    if (temp <= 203)
        return 16;
    if (temp <= 204)
        return 10;
    if (temp <= 205)
        return 0;
    if (temp <= 206)
        return 2;
    if (temp <= 207)
        return 5;
    if (temp <= 208)
        return 28;
    if (temp <= 210)
        return 3;
    if (temp <= 212)
        return 35;
    if (temp <= 213)
        return 24;
    if (temp <= 214)
        return 31;
    if (temp <= 215)
        return 27;
    if (temp <= 217)
        return 21;
    if (temp <= 218)
        return 1;
    if (temp <= 219)
        return 83;
    if (temp <= 221)
        return 7;
    if (temp <= 223)
        return 81;
    if (temp <= 224)
        return 80;
    if (temp <= 225)
        return 70;
    if (temp <= 226)
        return 19;
    if (temp <= 228)
        return 62;
    if (temp <= 229)
        return 68;
    if (temp <= 230)
        return 85;
    if (temp <= 232)
        return 69;
    if (temp <= 234)
        return 75;
    if (temp <= 236)
        return 87;
    if (temp <= 237)
        return 93;
    if (temp <= 240)
        return 74;
    if (temp <= 242)
        return 76;
    else
        return 89;
}

char get_char(int i, uint8_t image_average) {
    char arr[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!\"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~ ";
    return arr[get(i, image_average)];
}