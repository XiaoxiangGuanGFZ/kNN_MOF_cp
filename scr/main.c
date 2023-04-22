#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "func_IO_data.c"

# define MAXCHAR 4000
struct Date {
    int y;
    int m;
    int d;
};
struct df_rr_d
{
    /* data */
    struct Date date;    
    double *p_rr;
};
struct Para_global
    {
        /* global parameters */
        char FP_DAILY[100];
        char FP_CP[100];
        char FP_HOURLY[100];
        int START_YEAR;
        int START_MONTH;
        int START_DAY;
        int N_STATION;
        char T_CP[10];
        char SEASON[10];
        int CONTINUITY;
    };

void main(int argc, char * argv[]) {
    /*
    int argc: the number of parameters of main() function;
    char *argv[]: pointer array
    */
    /* char fname[100] = "D:/kNN_MOF_cp/data/global_para.txt";
        this should be the only extern input for this program */
    struct Para_global Para_df = {
        "D:/kNN_MOD_cp/data/test_rr_daily.txt",
        "D:/kNN_MOF_cp/data/test_cp_series.txt",
        "D:/kNN_MOF_cp/data/test_rr_obs_hourly.txt",
        1970,
        5,
        21,
        15,
        "TRUE",
        "TRUE",
        1
    };  // define the parameter structure and initialization
    struct Para_global * pp;
    pp = &Para_df;
    void import_global(char fname[], struct Para_global *pp);  // function declaration
    /* import the global parameters 
    parameter from main() function, pointer array
    argv[0]: pointing to the first string from command line (the executable file)
    argv[1]: pointing to the second string (parameter): file path and name of global parameter file.
    */
    import_global(*(++argv), pp);  
    /******* import circulation pattern series *********/
    if (strcmp(pp->T_CP, "TRUE") == 0) {
        int df_cp[100000][4];
        int nrow_cp, ncol_cp = 4;  // the number of CP data columns: 4 (y, m, d, cp)
        nrow_cp = import_df_cp(Para_df.FP_CP, ncol_cp, df_cp);
        printf("------ Import CP data series (Done) ------ \n");
        printf("* the first row: %d,%d,%d,%d \n", df_cp[0][0], df_cp[0][1], df_cp[0][2], df_cp[0][3]);
        printf("* number of CP data rows: %d\n", nrow_cp);
    } else {
        int df_cp=0;
        printf("------ Disaggregation conditioned only on seasonality (12 months) ------ \n");
    }
    /****** import daily rainfall data (to be disaggregated) *******/
    int import_dfrr_d(
        char FP_daily[], 
        int N_STATION,
        struct df_rr_d *p_rr_d
    );  // declare
    struct df_rr_d df_rr_daily[10000];
    int nrow_rr_d;
    nrow_rr_d = import_dfrr_d(
        Para_df.FP_DAILY, 
        Para_df.N_STATION,
        df_rr_daily
    );
    printf("------ Import daily rr data (Done) ------ \n");
    printf("* the total rows: %d\n", nrow_rr_d);
    printf("* the first day: %d-%d-%d\n", df_rr_daily[0].date.y,df_rr_daily[0].date.m,df_rr_daily[0].date.d);
    printf(
        "* the last day: %d-%d-%d\n", 
        df_rr_daily[nrow_rr_d-1].date.y,df_rr_daily[nrow_rr_d-1].date.m,df_rr_daily[nrow_rr_d-1].date.d
    );
    
    // int i,j;
    // for (i=0;i < 10;i++) {
    //     printf("\ndate: %d-%d-%d: \n", df_rr_daily[i].date.y, df_rr_daily[i].date.m, df_rr_daily[i].date.d);
    //     for (j=0; j < Para_df.N_STATION; j++) {
    //         printf("%3.1f,", *((df_rr_daily[i]).p_rr+j));
    //     }
    // }

    /****** import hourly rainfall data (obs as fragments) *******/

}

void import_global(
    char fname[], struct Para_global *pp
){
    /**************
     * import the global parameters into memory for disaggregation algorithm
     * 
     * -- Parameters:
     *      fname: a string (1-D character array), file path and name of the global parameters
     * -- Output:
     *      return a structure containing the key fields
     * ********************/
    
    char row[MAXCHAR];
    FILE *fp;
    char *token;
    char *token2;
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
            /*non-empty row(string)*/
            if (row[0] != '#') {
                /* the first character of row should not be # */
                for (i=0;i<strlen(row);i++) {
                    /* remove all the characters after # */
                    if (row[i] == '#') {
                        row[i] = '\0';
                    }
                }
                // printf("it is a valid row!\n");
                // printf("Row: %s", row);
                /*assign the values to the parameter structure: key-value pairs*/
                token = strtok(row, ",");   // the first column: key
                token2 = strtok(NULL, ",\n");  // the second column: value
                // printf("token: %s\n", token);
                if (strcmp(token, "FP_DAILY") == 0) {
                    strcpy(pp->FP_DAILY, token2);
                } else if (strcmp(token, "FP_CP") == 0) {
                    strcpy(pp->FP_CP, token2);
                } else if (strcmp(token, "FP_HOURLY") == 0) {
                    strcpy(pp->FP_HOURLY, token2);
                } else if (strcmp(token, "SEASON") == 0) {
                    strcpy(pp->SEASON, token2);
                } else if (strcmp(token, "T_CP") == 0) {
                    strcpy(pp->T_CP, token2);
                } else if (strcmp(token, "N_STATION") == 0) {
                    pp->N_STATION = atoi(token2);
                } else if (strcmp(token, "START_YEAR") == 0) {
                    pp->START_YEAR = atoi(token2);
                } else if (strcmp(token, "START_MONTH") == 0) {
                    pp->START_MONTH = atoi(token2);
                } else if (strcmp(token, "START_DAY") == 0) {
                    pp->START_DAY = atoi(token2);
                } else if (strcmp(token, "CONTINUITY") == 0) {
                    pp->CONTINUITY = atoi(token2);   
                } else {
                    printf(
                        "Error in opening global parameter file: unrecognized parameter field!"
                    );
                    exit(0);
                }
                
            }
        }
    }
    fclose(fp);
    printf("------ Global parameter import completed: -----\n");
    printf(
        "FP_DAILY: %s\nFP_HOULY: %s\nFP_CP: %s\nSTART_YEAR: %d\tSTART_MONTH: %d\tSTART_DAY: %d\n",
        pp->FP_DAILY, pp->FP_HOURLY, pp->FP_CP, pp->START_YEAR, pp->START_MONTH, pp->START_DAY
    );
    printf("------ Disaggregation parameters: -----\n");
    printf(
        "T_CP: %s\nSEASON: %s\nN_STATION: %d\nCONTINUITY: %d\n",
        pp->T_CP, pp->SEASON, pp->N_STATION, pp->CONTINUITY
    );
}

int import_dfrr_d(
    char FP_daily[], 
    int N_STATION,
    struct df_rr_d *p_rr_d
) {
    /******************
    
    *******************/
    // char FP_daily[]="D:/kNN_MOF_cp/data/rr_obs_daily.csv";  // key parameter
    FILE *fp_d;
    if ((fp_d=fopen(FP_daily, "r")) == NULL) {
        printf("Cannot open daily rr obs data file!\n");
        exit(0);
    }
    // struct df_rr_d df_rr_daily[10000];
    char *token;
    char row[MAXCHAR];
    int i, j, nrow;
    // int N_STATION = 134;  // key parameter

    i=0;
    while (!feof(fp_d)) {
        fgets(row, MAXCHAR, fp_d);
        (p_rr_d + i)->date.y = atoi(strtok(row, ","));  //df_rr_daily[i].
        (p_rr_d + i)->date.m = atoi(strtok(NULL, ","));
        (p_rr_d + i)->date.d = atoi(strtok(NULL, ","));
        (p_rr_d + i)->p_rr = (double *)malloc(N_STATION * sizeof(double));
        
        for (j=0; j < N_STATION; j++){
            token = strtok(NULL, ",");
            *((p_rr_d + i)->p_rr + j) = atof(token);
        }
        i=i+1;
    }
    nrow = i;
    return nrow;
}