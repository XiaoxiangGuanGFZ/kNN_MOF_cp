#ifndef FUNC_kNN_MOF_cp
#define FUNC_kNN_MOF_cp

int Toggle_WD(
    int N_STATION,
    double *p_rr_d
);

int Toggle_CONTINUITY(
        struct df_rr_h *p_rrh,
        struct df_rr_d *p_rrd,
        struct Para_global *p_gp,
        int ndays_h,
        int pool_cans[],
        int WD
    );

void kNN_sampling(
    struct df_rr_d *p_rrd,
    struct df_rr_h *p_rrh,
    struct Para_global *p_gp,
    int index_target,
    int pool_cans[],
    int n_can,
    int skip,
    int *index_fragments
);

double Distance_MHT(
    double *rr_t,
    double *rr_c,
    int n
);

void Fragment_assign(
    struct df_rr_h *p_rrh,
    struct df_rr_h *p_out,
    struct Para_global *p_gp,
    int fragment);

double get_random(); // declare

int weight_cdf_sample(
    int size_pool,
    int pool_cans[],
    double *weights_cdf    
);

void kNN_MOF(
        struct df_rr_h *p_rrh,
        struct df_rr_d *p_rrd,
        struct df_cp *p_cp,
        struct Para_global *p_gp,
        int nrow_rr_d,
        int ndays_h,
        int nrow_cp
    );


#endif