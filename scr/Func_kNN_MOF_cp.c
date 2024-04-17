#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include "def_struct.h"
#include "Func_kNN_MOF_cp.h"
#include "Func_dataIO.h"

void kNN_MOF(
    struct df_rr_h *p_rrh,
    struct df_rr_d *p_rrd,
    struct df_cp *p_cp,
    struct Para_global *p_gp,
    int nrow_rr_d,
    int ndays_h,
    int nrow_cp
) 
{
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
    int class_t, class_c;
    int skip = 0;
    struct df_rr_h df_rr_h_out; // this is a struct variable, not a struct array;
    
    // p_gp->CONTINUITY: 1, skip = 0;
    // p_gp->CONTINUITY: 3, skip = 1;
    skip = (int)((p_gp->CONTINUITY - 1) / 2);
    int pool_cans[MAXrow]; // the index of the candidates (a pool) after continuity filtering; the size is sufficient
    int *pool_cans_cp;
    pool_cans_cp = (int *)malloc(sizeof(int) * MAXrow);
    int n_cans_c = 0;      // the number of candidates after continuity filtering
    int n_can;             // the number of candidates after all conditioning (cp and seasonality)
    int fragment;          // the index of df_rr_h structure with the final chosed fragments

    FILE *p_FP_OUT;
    if ((p_FP_OUT=fopen(p_gp->FP_OUT, "w")) == NULL) {
        printf("Program terminated: cannot create or open output file\n");
        exit(1);
    }
    for (i = 0; i < nrow_rr_d; i++)
    { 
        // iterate each (possible) target day
        df_rr_h_out.date = (p_rrd + i)->date;
        df_rr_h_out.rr_d = (p_rrd + i)->p_rr; // is this valid?; address transfer
        df_rr_h_out.rr_h = calloc(p_gp->N_STATION, sizeof(double) * 24);  // allocate memory (stack);
        Toggle_wd = 0;  // initialize with 0 (non-rainy)
        Toggle_wd = Toggle_WD(p_gp->N_STATION, (p_rrd + i)->p_rr);
        n_cans_c = 0;

        if (Toggle_wd == 0) {
            // this is a non-rainy day; all 0.0
            n_can = -1;
            for (j = 0; j < p_gp->N_STATION; j++)
            {
                for (h = 0; h < 24; h++)
                {
                    df_rr_h_out.rr_h[j][h] = 0.0;
                }
            }
            for (size_t r = 0; r < p_gp->RUN; r++)
            {
                Write_df_rr_h(&df_rr_h_out, p_gp, p_FP_OUT, r+1);
            }
            
        } else {
            // Toggle_wd == 1; this is a rainy day; we will disaggregate it.
            int *index_fragments;
            index_fragments = (int *)malloc(sizeof(int) * p_gp->RUN);
            /*************************
             * condition: 
             *  multi-site wet-dry status vector between target and candidates
             *  
             */
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
            if (n_cans_c < 2 && p_gp->WD == 0) {
                /**************
                 * when the number of candidate days 
                 *      after strict (WD == 0) wet-dry status filter 
                 *      is less than 2, 
                 * relax the wet-dry status restrict: WD == 1
                 * */ 
                n_cans_c = Toggle_CONTINUITY(
                            p_rrh,
                            p_rrd + i,
                            p_gp,
                            ndays_h,
                            pool_cans,
                            1 
                        );
            }
            if (n_cans_c == 0) {
                printf("the target day %d-%02d-%02d has no matching hourly candidate!\n",
                       (p_rrd + i)->date.y, (p_rrd + i)->date.m, (p_rrd + i)->date.d);
                printf("Programm terminated!");
                exit(2);
            } else if (n_cans_c == 1) {
                /**
                 * only one candidate fits here after wet-dry status filtering 
                 *      then we just skip other filtering conditions, like
                 *      CP or seasonality.
                */
               for (size_t r = 0; r < p_gp->RUN; r++)
               {
                    *(index_fragments + r) = pool_cans[0];
               }
               // fragment = pool_cans[0];
            } else {
                // n_cans_c > 1;

                /***************************
                 * condition:
                 *          circulation pattern classification and(or) seasonality
                 * *******/
                n_can = 0;
                class_t = (p_rrd + i)->class;
                for (j = 0; j < n_cans_c; j++)
                {
                    class_c = (p_rrh + pool_cans[j])->class; // the class of the candidate day
                    if (class_c == class_t)
                    {
                        pool_cans_cp[n_can] = pool_cans[j];
                        n_can++;
                    }
                }
                /******** filtering is done: *********/
                if (n_can == 1) {
                    /***
                     * after candidates filtering, only one left.
                    */
                    fragment = pool_cans_cp[0];
                    for (size_t r = 0; r < p_gp->RUN; r++)
                    {
                        *(index_fragments + r) = pool_cans_cp[0];
                    }
                } else {
                    // n_can == 0 or n_can > 1
                    if (n_can == 0) {
                        /***
                         * after cp and seasonality filtering, 
                         *      there is not candidates any more.
                         * Then we just discard the cp and seasonality filtering
                         *      in order to make the disaggregation possible.
                        */
                        n_can = n_cans_c; pool_cans_cp = pool_cans;
                    }
                    if (i >= skip && i < nrow_rr_d - skip)
                    {
                        kNN_sampling(p_rrd, p_rrh, p_gp, i, pool_cans_cp, n_can, skip, index_fragments);
                    }
                    else
                    {
                        kNN_sampling(p_rrd, p_rrh, p_gp, i, pool_cans_cp, n_can, 0, index_fragments);
                    }
                }
                
            }
            /*assign the sampled fragments to target day (disaggregation)*/
            for (size_t r = 0; r < p_gp->RUN; r++)
            {
                Fragment_assign(p_rrh, &df_rr_h_out, p_gp, index_fragments[r]);
                /* write the disaggregation output */
                Write_df_rr_h(&df_rr_h_out, p_gp, p_FP_OUT, r + 1);
            }
        }
        
        printf("%d-%02d-%02d: Done!\n", (p_rrd+i)->date.y, (p_rrd+i)->date.m, (p_rrd+i)->date.d);
        free(df_rr_h_out.rr_h);  // free the memory allocated for disaggregated hourly output
    }
    fclose(p_FP_OUT);
}


int Toggle_WD(
    int N_STATION,
    double *p_rr_d
)
{
    /***********
     * rainy day (wet, WD == 1) or non rainy day (dry, WD == 0)
    */
    int WD = 0;
    for (size_t i = 0; i < N_STATION; i++)
    {
        if (*(p_rr_d + i) > 0.0)
        {
            WD = 1;
            break;
        }
    }
    return WD;
}

int Toggle_CONTINUITY(
    struct df_rr_h *p_rrh,
    struct df_rr_d *p_rrd,
    struct Para_global *p_gp,
    int ndays_h,
    int pool_cans[],
    int WD // Wet-dry status matching: strict:0; flexible:1
)
{
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
    int skip, match;  // match: whether wet-dry status match; 1: match; 0: not match
    int n_cans = 0;   // number of candidates fullfilling the CONTINUITY criteria (output)
    skip = (int)((p_gp->CONTINUITY - 1) / 2); // p_gp->CONTINUITY == 1: skip=0; p_gp->CONTINUITY == 3: skip=1

    for (int k = skip; k < ndays_h - skip; k++)
    {
        // iterate each day in hourly observations
        match = 1;
        if (WD == 1)
        {
            for (int s = 0; s < p_gp->N_STATION; s++)
            {
                /* flexible wet-dry status matching*/
                if (
                    p_rrd->p_rr[s] > 0.0 && (p_rrh + k)->rr_d[s] == 0.0
                )
                {
                    match = 0; break;
                }
            }
        } else if (WD == 0)
        {
            for (int s = 0; s < p_gp->N_STATION; s++)
            {
                /* strict wet-dry matching */
                // the wet-dry status in each vector should be completely the same with each other.
                if (
                    !(p_rrd->p_rr[s] > 0.0 == (p_rrh + k)->rr_d[s] > 0.0)
                )
                {
                    match = 0; break;
                }
            }
        }
        
        if (match == 1)
        {
            *(pool_cans + n_cans) = k;
            n_cans++;
        }
    }
    return n_cans;
}

void kNN_sampling(
    struct df_rr_d *p_rrd,
    struct df_rr_h *p_rrh,
    struct Para_global *p_gp,
    int index_target,
    int pool_cans[],
    int n_can,
    int skip,
    int *index_fragments
)
{
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
    int i, j;  // iteration variable
    int temp_c;  // temporary variable during sorting 
    double temp_d;
    double rd = 0.0;  // a random decimal value between 0.0 and 1.0

    // double distance[MAXrow]; 
    double *distance;  // the distance between target day and candidate days
    distance = (double *)malloc(n_can * sizeof(double));
    int size_pool; // the k in kNN
    // int index_out; // the output of this function: the sampled fragment from candidates pool

    /* manhattan distance */
    for (i = 0; i < n_can; i++)
    {
        *(distance + i) = 0.0;
        for (int s = 0 - skip; s < 1 + skip; s++)
        {
            *(distance + i) += Distance_MHT((p_rrd + index_target + s)->p_rr, (p_rrh + pool_cans[i] + s)->rr_d, p_gp->N_STATION);
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
    /*****************
     * if 0 distance exists, add 0.1, avoid inverse of 0.0
     * ****/
    for (size_t t = 0; t < n_can; t++)
    {
        if (*(distance + t) == 0.0)
        {
            *(distance + t) = 1;
        }
    }

    /***
     * the size of candidate pool in kNN algorithm
     *      the range of size_pool:
     *      [2, n_can]
     */
    size_pool = (int)sqrt(n_can) + 1;
    /***
     * compute the weights for kNN sampling
     *      the weight is defined as distance inverse
     * dynamic memory allocation for the double array - weights
     * */
    double *weights;
    weights = (double *)malloc(size_pool * sizeof(double)); // a double array with the size of size_pool
    double w_sum = 0.0;                           // the sum of the inversed distances
    for (i = 0; i < size_pool; i++)
    {
        *(weights + i) = 1 / distance[i];
        w_sum += 1 / distance[i];
    }
    for (i = 0; i < size_pool; i++)
    {
        *(weights + i) /= w_sum; // reassignment
    }
    /* compute the empirical cdf for weights (vector) */
    double *weights_cdf;
    weights_cdf = (double *)malloc(size_pool * sizeof(double));
    *(weights_cdf + 0) = weights[0]; // initialization
    for (i = 1; i < size_pool; i++)
    {
        *(weights_cdf + i) = *(weights_cdf + i - 1) + weights[i];
    }
    /* generate a random number, then an fragments index*/
    for (size_t r = 0; r < p_gp->RUN; r++)
    {
        *(index_fragments + r) = weight_cdf_sample(size_pool, pool_cans, weights_cdf);
    }

    free(weights);
    free(weights_cdf);
    free(distance);
}

double Distance_MHT(
    double *rr_t,
    double *rr_c,
    int n
)
{
    double out=0.0;
    for (size_t i = 0; i < n; i++)
    {
        out += fabs(*(rr_t + i) - *(rr_c + i));
    }
    return out;
}
double get_random() { return ((double)rand() / (double)RAND_MAX); }
int weight_cdf_sample(
    int size_pool,
    int pool_cans[],
    double *weights_cdf    
) {
    int i;
    double rd = 0.0;  // a random decimal value between 0.0 and 1.0
    int index_out; // the output of this function: the sampled fragment from candidates pool

    // srand(time(NULL)); // randomize seed
    rd = get_random(); // call the function to get a different value of n every time
    if (rd <= weights_cdf[0])
    {
        index_out = pool_cans[0];
    }
    else
    {
        for (i = 1; i < size_pool; i++)
        {
            if (rd <= weights_cdf[i] && rd > weights_cdf[i - 1])
            {
                index_out = pool_cans[i];
                break;
            }
        }
    }
    return index_out;
}

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
    int j,h;
    for (j = 0; j < p_gp->N_STATION; j++)
    {
        if (p_out->rr_d[j] > 0.0)
        {
            for (h = 0; h < 24; h++)
            {
                p_out->rr_h[j][h] = p_out->rr_d[j] * (p_rrh + fragment)->rr_h[j][h] / (p_rrh + fragment)->rr_d[j];
            }
        }
        else
        {
            // no rain at the station j
            for (h = 0; h < 24; h++)
            {
                p_out->rr_h[j][h] = 0.0;
            }
        }
    }
}