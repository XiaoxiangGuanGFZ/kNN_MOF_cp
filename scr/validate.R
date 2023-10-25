
# ------ performance evaluation ------
# in case some error in disaggregation

library(tidyverse)
ws <- "D:/kNN_MOF_cp/"

N_STATION = 134
stationnames = paste0('s', 1:N_STATION)

df_obs_d <- read.table(
  paste0(ws, "data/rr_obs_daily.csv"),
  header = F, sep = ','
) %>% `colnames<-`(
  c('y', 'm', 'd', paste0('s', 1:N_STATION))
)

df_sim_h <- read.table(
  paste0(ws, "output/rr_sim_hourly.csv"),
  header = F, sep = ','
) %>% `colnames<-`(
  c('y', 'm', 'd', 'h', paste0('s', 1:N_STATION))
)

for (i in 1:N_STATION){
  df_sim_d_s <- df_sim_h[, c(1:4, i + 4)] %>% 
    `colnames<-`(c('y', 'm', 'd', 'h', 'rr')) %>% 
    group_by(y, m, d) %>% 
    summarise(
      rr_d = sum(rr)
    )
  abs_error = abs(sum(df_sim_d_s$rr_d - df_obs_d[, i+3]))
  plot(df_sim_d_s$rr_d, df_obs_d[, i+3], 
       main = paste0("S", i, " error:", round(abs_error, 2)),
       xlab = "daily rr aggregated from simulated hourly rr [mm]",
       ylab = "observed daily rr [mm]")
  #if (abs_error > 10) {
    print(
      paste0("Site", i, " error: ", round(abs_error, 2))
    )
  #}
}
  

