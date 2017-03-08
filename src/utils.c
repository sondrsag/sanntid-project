#include "utils.h"
#include <math.h>

int str2int(char const * str) {
    int sum = 0;
    int i = 0;
    //First find end of string
    while (str[i+1] != '\0') {
        ++i;
    }
    int exp = 0;
    for (; i>=0; --i) {
        sum += (str[i] - '0' )*pow(10, exp++);
    }
    return sum;
}

