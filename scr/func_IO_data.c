#include<stdio.h>
#include<string.h>
#include<stdbool.h>
#include <stdlib.h>

#define MAXCHAR 1000

int main(){

    char fname[50] = "D:\\kNN_MOF_cp\\data\\cp_series.txt";
    int import_df_cp(char fname[100], int nrow, int ncol, int df[][5]);

    int df_cp[100000][5];
    int nrow=15553, ncol=5;
    import_df_cp(fname, nrow, ncol, df_cp);
    // printf("\nthe rows in data: %d\n", j);
    printf("the first row:\n %d,%d,%d,%d,%d \n", df_cp[0][0], df_cp[0][1],df_cp[0][2],df_cp[0][3],df_cp[0][4]);
    printf("the last row:\n %d", df_cp[nrow-1][ncol-1]);
    return 0;
}

int import_df_cp(
    char fname[100],
    int nrow, 
    int ncol,
    int df[][5] 
) {
    /*
    main function:
        import the circulation pattern classification results as an array
    parameters:
        fname: the file path, together with the file name of CP data
        nrow: the number of rows in the data file
        ncol: the number of columns in the data file
    output:
        return an array (int type) with the size of nrow*ncol
    */
    FILE *fp;
    char row[MAXCHAR];
    char *token;
    int i, j;  // the index for the col and row: for iteration
    // int df[nrow][5];  // predefine the output CP array
    // char fname[50] = "D:\\kNN_MOF_cp\\data\\Book1.csv";
    if ((fp=fopen(fname, "r")) == NULL) {
        printf("cannot open file\n");
        exit(0);
    }
    // fp = fopen(fname, "r");
    j = 0;  // from the first row 
    while (!feof(fp))
    {
        fgets(row, MAXCHAR, fp);  // the fgets() function comes from <stdbool.h>
        if (j==0) {
            printf("Row: %s", row);
        }
        
        token = strtok(row, ",");  
        /*
        strtok() fucntion: https://www.tutorialspoint.com/c_standard_library/c_function_strtok.htm
        */
        for (i=0; i < ncol; i++) {
            df[j][i] = atoi(token);  // the function atoi() comes from <stdlib.h>
            token = strtok(NULL, ",");  // the function strtok() comes from <string.h>
        }
        //while(token != NULL)
        //{
        //    printf("Token: %s\n", token);
        //    token = strtok(NULL, ",");
        // }
        j=j+1;
    }
    fclose(fp);
    return 0;
}
