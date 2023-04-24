#include <stdio.h>
#include <time.h>
#include <stdlib.h>

double get_random() { return ((double)rand() / (double)RAND_MAX); }

int main()
{
    double n = 0;
    srand(time(NULL)); // randomize seed
    int i, runs = 10;
    for (i=0; i<runs; i++) {
        
        n = get_random(); // call the function to get a different value of n every time
        printf("%6.4f, %d \n", n, (int)(n*10));  // print your number
    }
    return 0;
}
