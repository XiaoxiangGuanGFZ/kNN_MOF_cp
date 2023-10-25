#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <time.h>


# define MAXCHAR 4000
# define MAXrow 30000  // almost 100 years long ts

struct Date {
    int y;
    int m;
    int d;
};
struct df_rr_d
{
    /* data
     * data frame for the daily step precipitation,
     * p_rr points to a double-type array, 
     *      with the size equal to the number of stations 
     */
    struct Date date;    
    double *p_rr;
};
struct df_rr_h
{
    /* data
     * dataframe for the hourly step precipitation, 
     * rr_h: pointer array; 
     *      24 double-type pointers; 
     *      each points to an array of hourly precipitation (all rain sites)
     * rr_d: double-type pointer;
     *      pointing to an array of daily precipitation (all rain site) aggregated from rr_h
     */
    struct Date date;    
    double (*rr_h)[24];
    double *rr_d;
};

struct df_cp
{
    /* 
     * circulation pattern classficiation series
     * each day with a CP class
     */
    struct Date date;    
    int cp;
};

struct Para_global
    {
        /* global parameters */
        char FP_DAILY[100];     // file path of daily precipitation data (to be disaggregated)
        char FP_CP[100];        // file path of circulation pattern (CP) classification data series
        char FP_HOURLY[100];    // file path of hourly precipitation data (as fragments)
        char FP_OUT[100];       // file path of output(hourly) precipitation from disaggregation
        char FP_LOG[100];       // file path of log file
        int N_STATION;          // number of stations (rain sites)
        char T_CP[10];          // toggle (flag), whether the CP is considered in the algorithm
        char SEASON[10];        // toggle (flag), whether the seasonality is considered in the algorithm
        int CONTINUITY;         // continuity day
        int WD;                 // the flexibility level of wet-dry status in candidates filtering
    };

/* ---- function declaration ---- */
// data import functions
void import_global(char fname[], struct Para_global *p_gp);  // function declaration
int import_df_cp(
        char fname[],
        struct df_cp *p_df_cp
    );
int import_dfrr_d(
        char FP_daily[], 
        int N_STATION,
        struct df_rr_d *p_rr_d
    );  // declare
int import_dfrr_h(
        char FP_hourly[], 
        int N_STATION,
        struct df_rr_h *p_rr_h
    );


// kNN_MOF_cp algorithm functions
int Toggle_CONTINUITY(
        struct df_rr_h *p_rrh,
        struct df_rr_d *p_rrd,
        struct Para_global *p_gp,
        int ndays_h,
        int pool_cans[],
        int WD
    );

int Toogle_CP(
    struct Date date,
    struct df_cp *p_cp,
    int nrow_cp);

int kNN_sampling(
    struct df_rr_d *p_rrd,
    struct df_rr_h *p_rrh,
    struct Para_global *p_gp,
    int pool_cans[],
    int n_can);

void Fragment_assign(
    struct df_rr_h *p_rrh,
    struct df_rr_h *p_out,
    struct Para_global *p_gp,
    int fragment);

void Write_df_rr_h(
    struct df_rr_h *p_out,
    struct Para_global *p_gp,
    FILE *p_FP_OUT);

double get_random(); // declare

void kNN_MOF(
        struct df_rr_h *p_rrh,
        struct df_rr_d *p_rrd,
        struct df_cp *p_cp,
        struct Para_global *p_gp,
        int nrow_rr_d,
        int ndays_h,
        int nrow_cp
    );

void main(int argc, char * argv[]) {
    /*
    int argc: the number of parameters of main() function;
    char *argv[]: pointer array
    */
    /* char fname[100] = "D:/kNN_MOF_cp/data/global_para.txt";
        this should be the only extern input for this program */
    time_t tm;  //datatype from <time.h>
    time(&tm);
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
        1
    };  // define the global-parameter structure and initialization
    struct Para_global * p_gp;      // give a pointer to global_parameter structure
    p_gp = &Para_df;
    
    /******* import the global parameters ***********
    parameter from main() function, pointer array
    argv[0]: pointing to the first string from command line (the executable file)
    argv[1]: pointing to the second string (parameter): file path and name of global parameter file.
    */
    import_global(*(++argv), p_gp);
    FILE *p_log;  // file pointer pointing to log file
    if ((p_log=fopen(p_gp->FP_LOG, "a+")) == NULL) {
        printf("cannot create / open log file\n");
        exit(0);
    }
    printf("------ Global parameter import completed: %s", ctime(&tm));
    fprintf(p_log, "------ Global parameter import completed: %s", ctime(&tm));

    printf(
        "FP_DAILY: %s\nFP_HOULY: %s\nFP_CP: %s\nFP_OUT: %s\nFP_LOG: %s\n",
        p_gp->FP_DAILY, p_gp->FP_HOURLY, p_gp->FP_CP, p_gp->FP_OUT, p_gp->FP_LOG
    );
    fprintf(p_log, "FP_DAILY: %s\nFP_HOULY: %s\nFP_CP: %s\nFP_OUT: %s\nFP_LOG: %s\n",
        p_gp->FP_DAILY, p_gp->FP_HOURLY, p_gp->FP_CP, p_gp->FP_OUT, p_gp->FP_LOG);

    printf(
        "------ Disaggregation parameters: -----\nT_CP: %s\nSEASON: %s\nN_STATION: %d\nCONTINUITY: %d\nWD: %d\n",
        p_gp->T_CP, p_gp->SEASON, p_gp->N_STATION, p_gp->CONTINUITY, p_gp->WD
    );
    fprintf(
        p_log,
        "------ Disaggregation parameters: -----\nT_CP: %s\nSEASON: %s\nN_STATION: %d\nCONTINUITY: %d\nWD: %d\n",
        p_gp->T_CP, p_gp->SEASON, p_gp->N_STATION, p_gp->CONTINUITY, p_gp->WD
    );
    /******* import circulation pattern series *********/
    
    struct df_cp df_cps[MAXrow];
    int nrow_cp=0;  // the number of CP data columns: 4 (y, m, d, cp)
    if (strcmp(p_gp->T_CP, "TRUE") == 0) {
        nrow_cp = import_df_cp(Para_df.FP_CP, df_cps);
        time(&tm);
        printf("------ Import CP data series (Done): %s", ctime(&tm)); 
        fprintf(p_log, "------ Import CP data series (Done): %s", ctime(&tm));

        printf("* number of CP data rows: %d\n", nrow_cp); 
        fprintf(p_log, "* number of CP data rows: %d\n", nrow_cp);

        printf("* the first day: %d-%02d-%02d \n", df_cps[0].date.y, df_cps[0].date.m, df_cps[0].date.d);
        fprintf(p_log, "* the first day: %d-%02d-%02d \n", df_cps[0].date.y, df_cps[0].date.m, df_cps[0].date.d);
        
        printf("* the last day: %d-%02d-%02d \n", 
            df_cps[nrow_cp-1].date.y, df_cps[nrow_cp-1].date.m, df_cps[nrow_cp-1].date.d
        );
        fprintf(p_log, "* the last day: %d-%02d-%02d \n", 
            df_cps[nrow_cp-1].date.y, df_cps[nrow_cp-1].date.m, df_cps[nrow_cp-1].date.d
        );
    } else {
        time(&tm);
        printf("------ Disaggregation conditioned only on seasonality (12 months): %s", ctime(&tm));
        fprintf(p_log, "------ Disaggregation conditioned only on seasonality (12 months): %s", ctime(&tm));
    }
    /****** import daily rainfall data (to be disaggregated) *******/
    
    struct df_rr_d df_rr_daily[MAXrow];
    int nrow_rr_d;
    nrow_rr_d = import_dfrr_d(
        Para_df.FP_DAILY, 
        Para_df.N_STATION,
        df_rr_daily
    );
    time(&tm);
    printf("------ Import daily rr data (Done): %s", ctime(&tm)); fprintf(p_log, "------ Import daily rr data (Done): %s", ctime(&tm));
    
    printf("* the total rows: %d\n", nrow_rr_d); fprintf(p_log, "* the total rows: %d\n", nrow_rr_d);
    
    printf("* the first day: %d-%02d-%02d\n", df_rr_daily[0].date.y,df_rr_daily[0].date.m,df_rr_daily[0].date.d);
    fprintf(p_log, "* the first day: %d-%02d-%02d\n", df_rr_daily[0].date.y,df_rr_daily[0].date.m,df_rr_daily[0].date.d);

    printf(
        "* the last day: %d-%02d-%02d\n", 
        df_rr_daily[nrow_rr_d-1].date.y,df_rr_daily[nrow_rr_d-1].date.m,df_rr_daily[nrow_rr_d-1].date.d
    );
    fprintf(
        p_log,
        "* the last day: %d-%02d-%02d\n", 
        df_rr_daily[nrow_rr_d-1].date.y,df_rr_daily[nrow_rr_d-1].date.m,df_rr_daily[nrow_rr_d-1].date.d
    );
    
    /****** import hourly rainfall data (obs as fragments) *******/
    
    int ndays_h;
    struct df_rr_h df_rr_hourly[MAXrow];
    ndays_h = import_dfrr_h(Para_df.FP_HOURLY, Para_df.N_STATION, df_rr_hourly);

    time(&tm);
    printf("------ Import hourly rr data (Done): %s", ctime(&tm)); fprintf(p_log, "------ Import hourly rr data (Done): %s", ctime(&tm));
    
    printf("* total hourly obs days: %d\n", ndays_h); fprintf(p_log, "* total hourly obs days: %d\n", ndays_h);
    
    printf("* the first day: %d-%02d-%02d\n", df_rr_hourly[0].date.y, df_rr_hourly[0].date.m, df_rr_hourly[0].date.d);
    fprintf(p_log, "* the first day: %d-%02d-%02d\n", df_rr_hourly[0].date.y, df_rr_hourly[0].date.m, df_rr_hourly[0].date.d);
    
    printf(
        "* the last day: %d-%02d-%02d\n", 
        df_rr_hourly[ndays_h-1].date.y, df_rr_hourly[ndays_h-1].date.m, df_rr_hourly[ndays_h-1].date.d
    );
    fprintf(
        p_log,
        "* the last day: %d-%02d-%02d\n", 
        df_rr_hourly[ndays_h-1].date.y, df_rr_hourly[ndays_h-1].date.m, df_rr_hourly[ndays_h-1].date.d
    );

    /****** Disaggregation: kNN_MOF_cp *******/
    
    kNN_MOF(
        df_rr_hourly,
        df_rr_daily,
        df_cps,
        p_gp,  // the pointer pointing to Para_df structure;
        nrow_rr_d,
        ndays_h,
        nrow_cp
    );
    time(&tm);
    printf("------ Disaggregation daily2hourly (Done): %s", ctime(&tm));
    fprintf(p_log, "------ Disaggregation daily2hourly (Done): %s", ctime(&tm));

}

void import_global(
    char fname[], struct Para_global *p_gp
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
                    strcpy(p_gp->FP_DAILY, token2);
                } else if (strcmp(token, "FP_CP") == 0) {
                    strcpy(p_gp->FP_CP, token2);
                } else if (strcmp(token, "FP_HOURLY") == 0) {
                    strcpy(p_gp->FP_HOURLY, token2);
                } else if (strcmp(token, "SEASON") == 0) {
                    strcpy(p_gp->SEASON, token2);
                } else if (strcmp(token, "T_CP") == 0) {
                    strcpy(p_gp->T_CP, token2);
                } else if (strcmp(token, "N_STATION") == 0) {
                    p_gp->N_STATION = atoi(token2);
                } else if (strcmp(token, "WD") == 0) {
                    p_gp->WD = atoi(token2);
                } else if (strcmp(token, "FP_OUT") == 0) {
                    strcpy(p_gp->FP_OUT, token2);
                } else if (strcmp(token, "FP_LOG") == 0) {
                    strcpy(p_gp->FP_LOG, token2);
                } else if (strcmp(token, "CONTINUITY") == 0) {
                    p_gp->CONTINUITY = atoi(token2);   
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
    nrow = i-1;
    fclose(fp_d);
    
    // for (i=16;i < 17;i++) {
    //     printf("\ndate: %d-%d-%d: \n", (p_rr_d + i)->date.y, (p_rr_d + i)->date.m, (p_rr_d + i)->date.d);
    //     for (j=0; j < N_STATION; j++) {
    //         printf("%3.1f\t", *((p_rr_d + i)->p_rr+j));
    //     }
    // }
    printf("\n");
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
    // p_df_rr_h = p_rr_h + 16;
    // printf("%d-%d-%d: \n", p_df_rr_h->date.y, p_df_rr_h->date.m, p_df_rr_h->date.d);
    // for (h=0; h < 24; h++) {
    //     printf("H: %d\n", h);
    //     for (j=0; j < N_STATION; j++){
    //         printf("%3.1f\t", p_df_rr_h->rr_h[j][h]);
    //     }
    //     printf("\n");
    // }
    // printf("daily: \n");
    // for (j=0; j < N_STATION; j++){
    //         printf("%3.1f\t", p_df_rr_h->rr_d[j]);
    // }
    // printf("\n");

    return ndays-1; // the last is null
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
    return j - 1;  // the number of rows; the last row is null
}

void kNN_MOF(
    struct df_rr_h *p_rrh,
    struct df_rr_d *p_rrd,
    struct df_cp *p_cp,
    struct Para_global *p_gp,
    int nrow_rr_d,
    int ndays_h,
    int nrow_cp
) {
    /*******************
     * Description:
     *  algorithm: k-nearest neighbouring sampling, method-of-fragments, circulation patterns and (or) seasonality
     *  scale: daily2hourly, multiple rain gauges simultaneously
     * Parameters:
     *  nrow_rr_d: the number of rows in daily rr data file
     *  ndays_h: the number of observations of hourly rr data
     *  nrow_cp: the number of rows in cp series
     * *****************/
    int i, j, h, k, Toggle_wd;
    int toggle_cp;
    int skip=0;
    struct df_rr_h df_rr_h_out; // this is a struct variable, not a struct array;
    // struct df_rr_h df_rr_h_candidates[100];
    // p_gp->CONTINUITY: 1, skip = 0;
    // p_gp->CONTINUITY: 3, skip = 1;
    skip = (p_gp->CONTINUITY - 1) / 2;
    int pool_cans[MAXrow];  // the index of the candidates (a pool); the size is sufficient 
    int n_cans_c=0;  // the number of candidates after continuity filtering
    int n_can; //the number of candidates after all conditioning (cp and seasonality)
    int fragment; // the index of df_rr_h structure with the final chosed fragments
    
    FILE *p_FP_OUT;
    if ((p_FP_OUT=fopen(p_gp->FP_OUT, "w")) == NULL) {
        printf("Program terminated: cannot create or open output file\n");
        exit(0);
    }
    for (i=skip; i < nrow_rr_d-skip; i++) { //i=skip
        // iterate each (possible) target day
        df_rr_h_out.date = (p_rrd + i)->date;
        df_rr_h_out.rr_d = (p_rrd + i)->p_rr; // is this valid?; address transfer
        df_rr_h_out.rr_h = calloc(p_gp->N_STATION, sizeof(double) * 24);  // allocate memory (stack);
        Toggle_wd = 0;  // initialize with 0 (non-rainy)
        n_cans_c = 0;
        for (j=0; j < p_gp->N_STATION; j++) {
            if (*((p_rrd + i)->p_rr + j) > 0.0) {
                // any gauge with rr > 0.0
                Toggle_wd = 1;
                break;
            }
        }
        // printf("Targetday: %d-%d-%d: %d\n", 
        //     (p_rrd + i)->date.y, (p_rrd + i)->date.m, (p_rrd + i)->date.d, Toggle_wd
        // );
        if (Toggle_wd == 0) {
            // this is a non-rainy day; all 0.0
            n_can = -1;
            for (j=0; j<p_gp->N_STATION; j++){
                for (h=0; h<24; h++) {
                    df_rr_h_out.rr_h[j][h] = 0.0;
                }
            }
        } else {
            // Toggle_wd == 1;
            // this is a rainy day; we will disaggregate it.
            n_cans_c = Toggle_CONTINUITY(
                p_rrh,
                p_rrd + i,  // pointing to the target day struct
                p_gp,
                ndays_h,
                pool_cans,
                p_gp->WD  // 
                /****
                 * WD == 0, strict; 
                 * wet-dry status vectors match with each other 
                 *      (candidate and target day) 
                 *      at multiple rain gauges perfectly.
                 * typically, WD == 1, flexible in wet-dry status filtering 
                 */
            );
            // printf("WD-0: ncan: %d\n", n_cans_c);
            if (n_cans_c < 2 && p_gp->WD == 0) {
                /***
                 * when the number of candidate days 
                 *      after strict (WD == 0) wet-dry status filter 
                 *      is less than 2.
                 * */ 
                n_cans_c = Toggle_CONTINUITY(
                            p_rrh,
                            p_rrd + i,
                            p_gp,
                            ndays_h,
                            pool_cans,
                            1 
                        );
                // printf("WD-1: ncan: %d\n", n_cans_c);
            }
            if (n_cans_c == 0) {
                printf("the target day %d-%d-%d has no matching hourly candidate!\n", 
                (p_rrd+i)->date.y, (p_rrd+i)->date.m, (p_rrd+i)->date.d);
                printf("Programm terminated!"); exit(0);
            } else if (n_cans_c == 1) {
                /**
                 * only one candidate fits here after wet-dry status filtering 
                 *      then we just skip other filtering conditions, like
                 *      CP or seasonality.
                */
                fragment = pool_cans[0];
            } else {
                // n_cans_c > 1;
                // candidates filtering based on CP and seasonality
                n_can=0;
                for (int t = 0; t < n_cans_c; t++) {
                    toggle_cp = 0; // initialize the toggle_cp for every iteration
                    if (strcmp(p_gp->T_CP, "TRUE") == 0) {
                        // disaggregation conditioned on circulation pattern classification
                        if (
                            Toogle_CP( (p_rrh + pool_cans[t])->date, p_cp, nrow_cp ) == Toogle_CP( (p_rrd + i)->date, p_cp, nrow_cp )
                        ) {
                            // only continue when target and candidate day share the same cp class.
                            if (strcmp(p_gp->SEASON, "TRUE") == 0) {
                                // seasonality: summer and winter
                                if (
                                    ((p_rrh + pool_cans[t])->date.m >= 5 && (p_rrh + pool_cans[t])->date.m <= 10) == 
                                    ((p_rrd + i)->date.m >= 5 && (p_rrd + i)->date.m <= 10) 
                                ) {
                                    toggle_cp = 1;
                                }
                            } else {
                                // only cp, without seasonality
                                toggle_cp = 1;
                            }
                        } 
                    } else {
                        /***
                         * p_gp->T_CP != "TRUE": 
                         *      the disaggregation conditioned only on seasonality (12 months)
                        */
                        if (
                            (p_rrh + pool_cans[t])->date.m == (p_rrd + i)->date.m
                        ) {
                            toggle_cp = 1;
                        }
                    }
                    if (toggle_cp == 1) {
                        // this candidate selected into (final) pool
                        pool_cans[n_can] = pool_cans[t];  // rewrite pool_cans[] array; t is always >= n_can
                        n_can++;
                    }
                }
                /******** filtering is done: *********/
                if (n_can == 1) {
                    /***
                     * after candidates filtering, only one left.
                    */
                    fragment = pool_cans[0];
                } else {
                    // n_can == 0 or n_can > 1
                    if (n_can == 0) {
                        /***
                         * after cp and seasonality filtering, 
                         *      there is not candidates any more 
                         *      (toggle_cp always equals to 0).
                         *      pool_cans remains unchanged.
                         * Then we just discard the cp and seasonality filtering
                         *      in order to make the disaggregation possible.
                        */
                        n_can = n_cans_c;
                    }
                    /* kNN sampling, based on the qualified candidates pool */
                    fragment = kNN_sampling(p_rrd+i, p_rrh, p_gp, pool_cans, n_can);
                }
                // printf("%d-%d-%d: fragment: %d\n", (p_rrd+i)->date.y, (p_rrd+i)->date.m, (p_rrd+i)->date.d, fragment);
            }
            /*assign the sampled fragments to target day (disaggregation)*/
            Fragment_assign(p_rrh, &df_rr_h_out, p_gp, fragment);
        }
        /* write the disaggregation output */
        Write_df_rr_h(&df_rr_h_out, p_gp, p_FP_OUT);
        printf("%d-%02d-%02d: Done!\n", (p_rrd+i)->date.y, (p_rrd+i)->date.m, (p_rrd+i)->date.d);
        // printf("%d-%d-%d: %d\n", (p_rrd+i)->date.y, (p_rrd+i)->date.m, (p_rrd+i)->date.d, n_can);
    }
    fclose(p_FP_OUT);
}

int Toggle_CONTINUITY(
    struct df_rr_h *p_rrh,
    struct df_rr_d *p_rrd,
    struct Para_global *p_gp,
    int ndays_h,
    int pool_cans[],
    int WD // Wet-dry status matching: strict:0; flexible:1
){
    /*****************
     * Description:
     *      wet-dry status continuity matching check
     * Parameters:
     *      p_rrh: pointing to df_rr_hourly struct array (all the hourly rr observations);
     *      p_rrd: pointing to target day (to be disaggregated);
     *      p_gp: pointing to global parameter struct;
     *      ndays_h: number of days in hourly rr data;
     *      pool_cans: the pool of candidate index;
     *      WD: matching flexcibility, global parameter
     * Output:
     *      - the number of candidates (pool size)
     *      - bring back the pool_cans array
     * ***************/
    int i, j, k, skip, toggle_WD;
    // toggle_WD: whether wet-dry status match; 1: match; 0: not match
    int n_cans=0;  // number of candidates fullfilling the CONTINUITY criteria (output)
    skip = (p_gp->CONTINUITY - 1) / 2; // p_gp->CONTINUITY == 1: skip=0; p_gp->CONTINUITY == 3: skip=1
    
    for (k=skip; k < ndays_h-skip; k++) { 
        // iterate each day in hourly observations
        toggle_WD = 1;
        for (i=0-skip; i < 1+skip; i++) {
            // here we have to compare vectors between candidates and target; 
            //      CONTINUITY=1: skip=0; CONTINUITY=3: skip=1; 
            for (j=0; j < p_gp->N_STATION; j++){
                if (WD == 0) {
                    /*strict wet-dry matching*/
                    // the wet-dry status in each vector should be completely the same with each other.
                    if (
                        !(
                            ((p_rrd + i)->p_rr[j] > 0.0) == ((p_rrh + k + i)->rr_d[j] > 0.0)
                        )
                    ) {
                        toggle_WD = 0;break;
                    } else {
                        continue;
                    }
                } else {
                    // WD == 1
                    /* flexible wet-dry status matching*/
                    if (
                        ((p_rrd + i)->p_rr[j] > 0.0) && ((p_rrh + k + i)->rr_d[j] <= 0.0)
                    ) {
                        toggle_WD = 0;break;
                    }
                }
            }
        }
        if (toggle_WD == 1) {
            *(pool_cans + n_cans) = k;
            n_cans++;
            // printf("n_cans: %d\n", n_cans);
        }
    }
    return n_cans;
}

int Toogle_CP(
    struct Date date,
    struct df_cp *p_cp,
    int nrow_cp
){
    /*************
     * Description:
     *      derive the cp value (class) of the day based on date stamp (y-m-d)
     * Parameters:
     *      date: a Date struct, conaining y, m and d
     *      p_cp: pointing to the cp data struct array
     *      nrow_cp: total rows of cp observations
     * Output:
     *      return the derived cp value
     * **********/
    int i;
    int cp = -1;
    for (i = 0; i < nrow_cp; i++) {
        if (
            (p_cp+i)->date.y == date.y && (p_cp+i)->date.m == date.m && (p_cp+i)->date.d == date.d
        ) {
            cp = (p_cp+i)->cp;
            break; // terminate the loop directly
        }
    }
    if (cp == -1) {
        printf(
            "Program terminated: cannot find the cp class for the date %d-%02d-%02d\n",
            date.y, date.m, date.d
        );
        exit(0);
    }
    return cp;
}

int kNN_sampling(
    struct df_rr_d *p_rrd,
    struct df_rr_h *p_rrh,
    struct Para_global *p_gp,
    int pool_cans[],
    int n_can
){
    /**************
     * Description:
     *      - compute the manhattan distance of rr between target and candidate days
     *      - sort the distance in the increasing order
     *      - select the sqrt(n_can) largest distance
     *      - weights defined as inverse of distance
     *      - sample one distance 
     * Parameters:
     *      p_rrd: the target day (structure pointer)
     *      p_rrh: pointing to the hourly rr obs structure array
     *      p_gp: pointing to global parameter structure
     *      pool_cans: the index pool of candidats
     *      n_can: the number (or size) fo candidates pool
     * Output:
     *      return a sampled index (fragments source)
     * ***********/
    int i, j;
    int temp_c;
    double temp_d;
    double distance[MAXrow];
    int size_pool; // the k in kNN
    int index_out;
    for (i=0; i<n_can; i++){
        *(distance+i) = 0.0;
        for (j=0; j<p_gp->N_STATION; j++){
            *(distance+i) += fabs(p_rrd->p_rr[j] - (p_rrh + pool_cans[i])->rr_d[j]);
        }
    }
    // sort the distance in the increasing order
    for (i=0; i<n_can-1; i++) {
        for (j=i+1; j<n_can; j++) {
            if (distance[i] > distance[j]) {
                temp_c = pool_cans[i]; pool_cans[i] = pool_cans[j]; pool_cans[j] = temp_c;
                temp_d = distance[i]; distance[i] = distance[j]; distance[j] = temp_d;
            }
        }
    }
    if (distance[0] <= 0.0) {
        // the closest candidate with the distance of 0.0, then we skip the weighted sampling.
        index_out = pool_cans[0];
    } else {
        size_pool = (int)sqrt(n_can) + 1; // final candidates pool size
        /*compute the weights for kNN sampling*/
        double *weights;
        weights = malloc(size_pool * sizeof(double));
        double w_sum=0.0; 
        for (i=0; i<size_pool; i++){
            *(weights+i) = 1/distance[i];
            w_sum += 1/distance[i];
        }
        for (i=0; i<size_pool; i++){
            *(weights+i) /= w_sum; // reassignment
        }
        /* compute the empirical cdf for weights (vector) */
        double *weights_cdf;
        weights_cdf = malloc(size_pool * sizeof(double));
        *(weights_cdf + 0) = weights[0];  // initialization
        for (i=1; i<size_pool; i++){
            *(weights_cdf + i) = *(weights_cdf + i-1) + weights[i];
        }
        /* generate a random number, then an fragments index*/
        
        srand(time(NULL)); // randomize seed
        double rd = 0;
        rd = get_random(); // call the function to get a different value of n every time

        if (rd <= weights_cdf[0])
        {
            index_out = pool_cans[0];
        }
        else
        {
            for (i = 1; i < size_pool; i++)
            {
                if (rd<=weights_cdf[i] && rd > weights_cdf[i-1]) {index_out = pool_cans[i];break;}
            }
        }
    }
    
    return index_out;
}

double get_random() { return ((double)rand() / (double)RAND_MAX); }

void Fragment_assign(
    struct df_rr_h *p_rrh,
    struct df_rr_h *p_out,
    struct Para_global *p_gp,
    int fragment
){
    /**********
     * Description:
     *      disaggregate the target day rainfall into hourly scale based on the selected fragments
     * Parameters: 
     *      p_rrh: pointing to the hourly obs rr structure array
     *      p_out: pointing to the disaggregated hourly rr results struct (to output) 
     *      p_gp: global parameters struct
     *      fragment: the index of p_rrh struct after filtering and resampling
     * Output:
     *      p_out
     * *******/
    int j,h,toggle;
    toggle = 1;
    for (j=0; j<p_gp->N_STATION;j++) {
        if (p_out->rr_d[j] > 0.0 && (p_rrh + fragment)->rr_d[j] <= 0.0 ) {
            toggle = 0;
            break;
        }
    }
    if (toggle == 1) {
        for (j=0; j<p_gp->N_STATION; j++) {
            if (p_out->rr_d[j] > 0.0) {
                for (h=0; h<24; h++) {
                    p_out->rr_h[j][h] = p_out->rr_d[j] * (p_rrh + fragment)->rr_h[j][h] / (p_rrh + fragment)->rr_d[j];
                }
            } else {
                // no rain at station j
                for (h=0; h<24; h++) {p_out->rr_h[j][h] = 0.0;}
            }
        }
    } else {
        printf("The target day %d-%02d-%02d disaggregation failed, please check or retry!\n---Programm terminated!", 
        p_out->date.y, p_out->date.m, p_out->date.d);
        exit(0);
    }
    
    // printf("hourly example: %f,%f,%f,%f\n", p_out->rr_h[0][0],p_out->rr_h[1][0],p_out->rr_h[2][0],p_out->rr_h[3][0]);
}
void Write_df_rr_h(
    struct df_rr_h *p_out,
    struct Para_global *p_gp,
    FILE *p_FP_OUT
){
    /**************
     * Description:
     *      write the disaggregated results into output file (.csv)
     * Parameters:
     *      p_gp: 
     *      p_FP_OUT: a FILE pointer, pointing to the output file
     * 
     * ************/
    int j,h;
    for (h=0;h<24;h++) {
        fprintf(
            p_FP_OUT,
            "%d,%d,%d,%d,%.3f", 
            p_out->date.y, p_out->date.m, p_out->date.d, h, 
            p_out->rr_h[0][h]
        ); // print the date and time (y, m, d, h), together with the value from first rr gauge (0)
        
        for (j=1;j<p_gp->N_STATION;j++){
            fprintf(
                p_FP_OUT,
                ",%.3f", p_out->rr_h[j][h]
            );
        }
        fprintf(p_FP_OUT, "\n"); // print "\n" (newline) after one row
    }
    // printf("%d-%d-%d: Done\n", p_out->date.y, p_out->date.m, p_out->date.d); // print to screen (command line)
}
