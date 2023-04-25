# -------- introduction -------
# - evaluate the performance of kNN_MOF_cp preliminarily
# - code instance for multiple running of kNN_MOF_cp
# 
# Last updated: 2023-04-25
# Author: Xiaoxiang Guan (guan@gfz-potsdam.de)


# ------ performance evaluation ------
# for detailed method evaluation 
# see paper: 
library(tidyverse)
ws <- "D:/kNN_MOF_cp/"
N_STATION = 134

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

# -- scatter plot and relative error

stationnames = paste0('s', 1:N_STATION)

df_sim_d <- df_sim_h %>% group_by(y, m, d) %>%
  summarise(rr = sum(s111))
plot(
  df_sim_d$rr,
  df_obs_d$s111
)
sum(
  abs((df_sim_d$rr - df_obs_d$s111))
)

# --- 
dates <- seq(
  as.Date("2006-01-01"),
  as.Date("2020-12-31"),
  1
)

# ------ multiple runs ------
ws <- "D:/kNN_MOF_cp/"

df_gp <- data.frame(
  key = c("FP_DAILY", "FP_HOURLY", "FP_CP", "FP_OUT", "FP_LOG", 
          "N_STATION", "T_CP", "SEASON", "CONTINUITY", "WD"),
  value = c(
    "D:/kNN_MOF_cp/data/rr_obs_daily.csv",
    "D:/kNN_MOF_cp/data/rr_obs_hourly.csv",
    "D:/kNN_MOF_cp/data/cp_series.csv",
    "D:/kNN_MOF_cp/output/rr_sim_hourly.csv",
    "D:/kNN_MOF_cp/my.log",
    "134",
    "TRUE",
    "TRUE",
    "1",
    "1"
  )
)

runs = 10
for (run in 1:runs) {
  df_gp$value[df_gp$key == "FP_OUT"] = paste0(
    ws, "output/rr_sim_hourly_", run, ".csv"
  )
  fp_gp = paste0(
    ws, "data/global_para_", run, ".txt"
  )
  write.table(
    df_gp,
    file = fp_gp,
    col.names = F, row.names = F, append = F, 
    sep = ',', quote = F
  )
  
  # invoke application from command line directly
  # there should be other fancy approach to be explored and tested.
  shellc = paste0(
    ws, 'scr/a.exe ', fp_gp
  )
  system(shellc) 
  
}

# ---- Done


