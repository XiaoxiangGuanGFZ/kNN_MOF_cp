#include <stdio.h>
#include "func_IO_data.c"

int main() {
    float a=1.0;
    float b=1.12;
    float c;
    // float max(float a, float b);
    printf("input the values of a and b: \n");
    scanf("a=%f,b=%f", &a, &b);
    c = max(a,b);
    printf("a=%5.2f,b=%5.2f", a, b);
    printf("\nthe max among a and b is: %5.2f", c);
    return 0;
}

