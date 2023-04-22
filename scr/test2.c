#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int main(int argc, char const *argv[]) {
    char FP_daily[]="D:/kNN_MOF_cp/data/rr_obs_daily.csv";  // key parameter
    FILE *fp_d;
    if ((fp_d=fopen(FP_daily, "r")) == NULL) {
        printf("Cannot open daily rr obs data file!\n");
        exit(0);
    }
    struct df_rr_d df_rr_daily[10000];
    char *token;
    char row[MAXCHAR];
    int i, j, nrow;
    int N_STATION = 134;  // key parameter

    i=0;
    while (!feof(fp_d)) {
        fgets(row, MAXCHAR, fp_d);
        df_rr_daily[i].date.y = atoi(strtok(row, ","));
        df_rr_daily[i].date.m = atoi(strtok(NULL, ","));
        df_rr_daily[i].date.d = atoi(strtok(NULL, ","));
        df_rr_daily[i].p_rr = (double *)malloc(N_STATION * sizeof(double));
        
        for (j=0; j < N_STATION; j++){
            token = strtok(NULL, ",");
            *(df_rr_daily[i].p_rr + j) = atof(token);
        }
        i=i+1;
    }
    nrow = i;
    // check the results
    printf("the total rows: %d\n", nrow);
    for (i=0;i < 10;i++) {
        printf("\ndate: %d-%d-%d: \n", df_rr_daily[i].date.y, df_rr_daily[i].date.m, df_rr_daily[i].date.d);
        for (j=0; j < N_STATION; j++) {
            printf("%3.1f,", *((df_rr_daily[i]).p_rr+j));
        }
    }
    return 0;
}
