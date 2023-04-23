#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

# define MAXCHAR 4000
# define MAXrow 20000  // almost 100 years long ts

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
struct df_rr_h
{
    /* data */
    struct Date date;    
    double (*rr_h)[24];
    double *rr_d;
};

struct df_cp
{
    /* data */
    struct Date date;    
    int cp;
};

struct Para_global
    {
        /* global parameters */
        char FP_DAILY[100];
        char FP_CP[100];
        char FP_HOURLY[100];
        char FP_OUT[100];
        char FP_LOG[100];
        int N_STATION;
        char T_CP[10];
        char SEASON[10];
        int CONTINUITY;
        int WD;
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
        "D:/kNN_MOF_cp/output/",
        "D:/kNN_MOF_cp/my_disaggregation.log",
        134,
        "TRUE",
        "TRUE",
        1,
        0
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
    int import_df_cp(
        char fname[],
        struct df_cp *p_df_cp
    );
    if (strcmp(pp->T_CP, "TRUE") == 0) {
        // int df_cps[100000][4];
        struct df_cp df_cps[MAXrow];
        int nrow_cp;  // the number of CP data columns: 4 (y, m, d, cp)
        nrow_cp = import_df_cp(Para_df.FP_CP, df_cps);
        printf("------ Import CP data series (Done) ------ \n");
        printf("* number of CP data rows: %d\n", nrow_cp);
        printf("* the first day: %d-%d-%d \n", df_cps[0].date.y, df_cps[0].date.m, df_cps[0].date.d);
        printf("* the last day: %d-%d-%d \n", 
            df_cps[nrow_cp-1].date.y, df_cps[nrow_cp-1].date.m, df_cps[nrow_cp-1].date.d
        );
        
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
    struct df_rr_d df_rr_daily[MAXrow];
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
    
    /****** import hourly rainfall data (obs as fragments) *******/
    int import_dfrr_h(
        char FP_hourly[], 
        int N_STATION,
        struct df_rr_h *p_rr_h
    );
    int ndays_h;
    struct df_rr_h df_rr_hourly[MAXrow];
    ndays_h = import_dfrr_h(Para_df.FP_HOURLY, Para_df.N_STATION, df_rr_hourly);
    printf("------ Import hourly rr data (Done) ------ \n");
    printf("* total hourly obs days: %d\n", ndays_h);
    printf("* the first day: %d-%d-%d\n", df_rr_hourly[0].date.y, df_rr_hourly[0].date.m, df_rr_hourly[0].date.d);
    printf(
        "* the last day: %d-%d-%d\n", 
        df_rr_hourly[ndays_h-1].date.y, df_rr_hourly[ndays_h-1].date.m, df_rr_hourly[ndays_h-1].date.d
    );
    /****** Disaggregation: kNN_MOF_cp *******/



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
                } else if (strcmp(token, "WD") == 0) {
                    pp->WD = atoi(token2);
                } else if (strcmp(token, "FP_OUT") == 0) {
                    strcpy(pp->FP_OUT, token2);
                } else if (strcmp(token, "FP_LOG") == 0) {
                    strcpy(pp->FP_LOG, token2);
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
        "FP_DAILY: %s\nFP_HOULY: %s\nFP_CP: %s\nFP_OUT: %s\nFP_LOG: %s\n",
        pp->FP_DAILY, pp->FP_HOURLY, pp->FP_CP, pp->FP_OUT, pp->FP_LOG
    );
    printf("------ Disaggregation parameters: -----\n");
    printf(
        "T_CP: %s\nSEASON: %s\nN_STATION: %d\nCONTINUITY: %d\nWD: %d\n",
        pp->T_CP, pp->SEASON, pp->N_STATION, pp->CONTINUITY, pp->WD
    );
}

int import_dfrr_d(
    char FP_daily[], 
    int N_STATION,
    struct df_rr_d *p_rr_d
) {
    /**************
     * Main:
     *  import daily rainfall data (tobe disaggregated) into memory
     * Parameters: 
     *  FP_daily: a string, storing the file path and name of daily rr data file
     *  N_STATION: the number of rainfall stations in disaggrgeation 
     *  p_rr_d: name of structure df_rr_d array
     * Return:
     *  output the number of days (rows)
     * ****************/
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
    fclose(fp_d);
    // int i,j;
    // for (i=0;i < 10;i++) {
    //     printf("\ndate: %d-%d-%d: \n", df_rr_daily[i].date.y, df_rr_daily[i].date.m, df_rr_daily[i].date.d);
    //     for (j=0; j < Para_df.N_STATION; j++) {
    //         printf("%3.1f,", *((df_rr_daily[i]).p_rr+j));
    //     }
    // }
    return nrow;
}

int import_dfrr_h(
    char FP_hourly[], 
    int N_STATION,
    struct df_rr_h *p_rr_h
) {
    /**************
     * Main:
     *  import hourly rainfall observations into memory
     * Parameters: 
     *  FP_hourly: a string, storing the file path and name of hourly rr data file
     *  N_STATION: the number of rainfall stations in disaggrgeation 
     *  p_rr_h: name of structure df_rr_h array
     * Return:
     *  output the number of hourly observation days
     * ****************/
    // char FP_hourly[]="D:/kNN_MOF_cp/data/rr_obs_hourly.csv";
    FILE *fp_h;
    if ((fp_h=fopen(FP_hourly, "r")) == NULL) {
        printf("Cannot open hourly rr data file!\n");
        exit(0);
    }
    // struct df_rr_h df_rr_hourly[10000];
    struct df_rr_h *p_df_rr_h;  // pointer of df_rr_h; for iteration
    
    char *token;
    char row[MAXCHAR];
    int i, j, h, nrow_total, ndays;
    // int N_STATION = 134;
    
    i=0;
    p_df_rr_h = p_rr_h; // initialize
    while (!feof(fp_h)) {
        fgets(row, MAXCHAR, fp_h);
        if (i%24 == 0) {
            /* %: remainder after division (modulo division)*/
            (p_df_rr_h->date).y = atoi(strtok(row, ","));
            (p_df_rr_h->date).m = atoi(strtok(NULL, ","));
            (p_df_rr_h->date).d = atoi(strtok(NULL, ","));
            p_df_rr_h->rr_h = calloc(N_STATION, sizeof(double) * 24);  // allocate memory (stack)
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
    fclose(fp_h);
    nrow_total = i;  // the total number of row in the data file
    ndays = p_df_rr_h - p_rr_h;  // the exact size of struct p_rr_h array
    /**** aggregate the hourly rr into daily scale ****/
    for (p_df_rr_h = p_rr_h; p_df_rr_h<p_rr_h + ndays; p_df_rr_h++) {
        p_df_rr_h->rr_d = (double *)malloc(N_STATION*sizeof(double)); // allocate memory (stack)
        for (j=0;j<N_STATION;j++) {
            *(p_df_rr_h->rr_d+j) = 0;
            for (h=0;h<24;h++) {
                *(p_df_rr_h->rr_d+j) += p_df_rr_h->rr_h[j][h];
            }
        }
    }
    /* check the results */ 
    // printf("the total rows: %d\n", nrow_total);
    // printf("total obs days: %d\n", ndays);
    // p_df_rr_h = p_rr_h + 11;
    // printf("date: %d-%d-%d\n", p_df_rr_h->date.y, p_df_rr_h->date.m, p_df_rr_h->date.d);
    
    // for (h=0; h < 24; h++) {
    //     printf("H: %d\n", h);
    //     for (j=0; j < N_STATION; j++){
    //         printf("%3.1f\t", p_df_rr_h->rr_h[j][h]);
    //     }
    //     printf("\n");
    // }
    // for (j=0;j<N_STATION;j++) {
    //     printf("%3.1f\t", p_df_rr_h->rr_d[j]);
    // }
    return ndays;
}

int import_df_cp(
    char fname[],
    struct df_cp *p_df_cp
) {
    /*********************
    * Main function:
    *     import the circulation pattern classification results
    * Parameters:
    *     fname: the file path, together with the file name of CP data
    * Return:
    *     bring back struct array of cp data to main() function;
    *     the return value of the function: the number of rows in the data file
    *********************/
    FILE *fp_cp;
    char row[MAXCHAR];
    char *token;
    struct df_cp *p;  // struct pointer for iteration
    int j;  
    if ((fp_cp=fopen(fname, "r")) == NULL) {
        printf("Cannot open cp data file\n");
        exit(0);
    }
    j = 0;  // from the first row 
    p = p_df_cp;
    while (!feof(fp_cp))
    {
        // the fgets() function comes from <stdbool.h>
        // Reads characters from stream and stores them as a C string
        fgets(row, MAXCHAR, fp_cp); 
        token = strtok(row, ",");  
        p->date.y = atoi(token);
        p->date.m = atoi(strtok(NULL, ","));
        p->date.d = atoi(strtok(NULL, ","));
        p->cp = atoi(strtok(NULL, ","));
        /*
        strtok() fucntion: https://www.tutorialspoint.com/c_standard_library/c_function_strtok.htm
        */
        p++; j++;
    }
    fclose(fp_cp);
    return j;  // the number of rows
}

