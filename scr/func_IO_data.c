#include <stdio.h>
int main() {
    int a=1;
    double b=1.12;
    printf("input the values of a and b: \n");
    scanf("a=%d,b=%lf", &a, &b);
    printf("a=%d,b=%5.2f", a, b);
    return 0;
}
