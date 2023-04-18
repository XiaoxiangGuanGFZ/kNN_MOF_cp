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
                /*assign the values to the parameter structure*/
                token = strtok(row, ","); 
                // printf("token: %s\n", token);
                if (strcmp(token, "FP_DAILY") == 0) {
                    strcpy(Para_df.FP_DAILY, strtok(NULL, ","));
                } else if (strcmp(token, "FP_CP") == 0) {
                    strcpy(Para_df.FP_CP, strtok(NULL, ","));
                } else if (strcmp(token, "FP_HOURLY") == 0) {
                    strcpy(Para_df.FP_HOURLY, strtok(NULL, ","));
                } else if (strcmp(token, "SEASON") == 0) {
                    strcpy(Para_df.SEASON, strtok(NULL, ","));
                } else if (strcmp(token, "T_CP") == 0) {
                    strcpy(Para_df.T_CP, strtok(NULL, ","));
                } else if (strcmp(token, "N_STATION") == 0) {
                    Para_df.N_STATION = atoi(strtok(NULL, ","));
                } else if (strcmp(token, "START_YEAR") == 0) {
                    Para_df.START_YEAR = atoi(strtok(NULL, ","));
                } else if (strcmp(token, "START_MONTH") == 0) {
                    Para_df.START_MONTH = atoi(strtok(NULL, ","));
                } else if (strcmp(token, "START_DAY") == 0) {
                    Para_df.START_DAY = atoi(strtok(NULL, ","));
                } else if (strcmp(token, "CONTINUITY") == 0) {
                    Para_df.CONTINUITY = atoi(strtok(NULL, ","));   
                } else {
                    printf(
                        "Error in opening global parameter file: unrecognized parameter field!"
                    );
                    exit(0);
                }
                
            }
        }
    }
    // outputs
    printf("\n------global parameter import completed: -----\n");
    printf(
        "FP_DAILY: %s\nFP_HOULY: %s\nFP_CP: %s\nSTART_YEAR: %d\tSTART_MONTH: %d\tSTART_DAY: %d\n",
        Para_df.FP_DAILY, Para_df.FP_HOURLY, Para_df.FP_CP, Para_df.START_YEAR, Para_df.START_MONTH, Para_df.START_DAY
    );
    printf("----- Disaggregation paras: ------\n");
    printf(
        "T_CP: %s\nSEASON: %s\nN_STATION: %d\nCONTINUITY: %d\n",
        Para_df.T_CP, Para_df.SEASON, Para_df.N_STATION, Para_df.CONTINUITY
    );
    
}
