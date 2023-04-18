#include <stdio.h>
#include <stdlib.h>
#include <string.h>
# define MAXCHAR 1000
void main() {
    char fname[100] = "D:/kNN_MOF_cp/data/global_para.txt";
    char row[MAXCHAR];
    FILE *fp;
    char *token;
    int i;
    if ((fp=fopen(fname, "r")) == NULL) {
        printf("cannot open global parameter file\n");
        exit(0);
    }
    while (!feof(fp))
    {
        // the fgets() function comes from <stdbool.h>
        // Reads characters from stream and stores them as a C string
        fgets(row, MAXCHAR, fp); 
        if (row != NULL && strlen(row) > 1) {
            if (row[0] != '#') {
                /* remove all the characters after # */
                for (i=0;i<strlen(row);i++) {
                    if (row[i] == '#') {
                        row[i] = '\0';
                    }
                }
                // printf("it is a valid row!\n");
                // printf("Row: %s", row);
                token = strtok(row, ","); 
                printf("field: %s\n", token); 
                token = strtok(NULL, ",");
                printf("value: %s", token); 
            }
        }
        
    }
    
}
