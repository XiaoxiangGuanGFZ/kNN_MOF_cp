#include <stdio.h>
#include <stdlib.h>
#include <string.h>

# define MAXCHAR 4000
struct Date {
    int y;
    int m;
    int d;
};

struct df_rr_h
{
    /* data */
    struct Date date;    
    double (*rr_h)[24];
    double *rr_d;
};

void main() {
    char FP_hourly[]="D:/kNN_MOF_cp/data/rr_obs_hourly.csv";
    FILE *fp_h;
    if ((fp_h=fopen(FP_hourly, "r")) == NULL) {
        printf("Cannot open hourly rr obs data file!\n");
        exit(0);
    }
    struct df_rr_h df_rr_hourly[10000];
    struct df_rr_h *p_df_rr_h;
    
    char *token;
    char row[MAXCHAR];
    int i, j, h, nrow_total, ndays;
    int N_STATION = 134;

    i=0;
    p_df_rr_h = df_rr_hourly;
    while (!feof(fp_h)) {
        fgets(row, MAXCHAR, fp_h);
        if (i%24 == 0) {
            /* %: remainder after division (modulo division)*/
            (p_df_rr_h->date).y = atoi(strtok(row, ","));
            (p_df_rr_h->date).m = atoi(strtok(NULL, ","));
            (p_df_rr_h->date).d = atoi(strtok(NULL, ","));
            p_df_rr_h->rr_h = calloc(N_STATION, sizeof(double) * 24);
        } else {
            token = strtok(row, ","); token = strtok(NULL, ","); token = strtok(NULL, ",");
        }
        h = atoi(strtok(NULL, ","));
        for (j=0; j < N_STATION; j++){
            token = strtok(NULL, ",");
            *(*(p_df_rr_h->rr_h + j) + h) = atof(token);
        }
        if (i%24 == 23) {
            p_df_rr_h++;
        }
        i=i+1;
    }
    nrow_total = i;  // the total number of row in the data file
    ndays = p_df_rr_h - df_rr_hourly;  // the exact size of struct df_rr_hourly array
    /* aggregate the hourly rr into daily scale */
    for (p_df_rr_h = df_rr_hourly;p_df_rr_h<df_rr_hourly + ndays; p_df_rr_h++) {
        p_df_rr_h->rr_d = (double *)malloc(N_STATION*sizeof(double));
        for (j=0;j<N_STATION;j++) {
            *(p_df_rr_h->rr_d+j) = 0;
            for (h=0;h<24;h++) {
                *(p_df_rr_h->rr_d+j) += p_df_rr_h->rr_h[j][h];
            }
        }
    }
    // check the results
    printf("the total rows: %d\n", nrow_total);
    printf("total obs days: %d\n", ndays);
    p_df_rr_h = df_rr_hourly + 11;
    printf("date: %d-%d-%d\n", p_df_rr_h->date.y, p_df_rr_h->date.m, p_df_rr_h->date.d);
    
    for (h=0; h < 24; h++) {
        printf("H: %d\n", h);
        for (j=0; j < N_STATION; j++){
            printf("%3.1f\t", p_df_rr_h->rr_h[j][h]);
        }
        printf("\n");
    }
    for (j=0;j<N_STATION;j++) {
        printf("%3.1f\t", p_df_rr_h->rr_d[j]);
    }

}
