# ------ test run kNN_MOF_cp in powershell/ R ------
ws <- "D:/kNN_MOF_cp/"
fp_gp = paste0(ws, "data/global_para.txt")
shellc = paste0(
  ws, 'scr/main.exe ', fp_gp
)
status = system(shellc) 
status


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
  

# ----- debug the candidates sampling -----
library(tidyverse)
ws <- "D:/kNN_MOF_cp/"
n = 300

df_rwg <- read.table(
	paste0(ws, "data/rr_sim_daily_long.csv"),
	header = F, sep = ","
) %>% `colnames<-`(c('y', 'm', 'd', paste0("s", 1:n)))

df_obs_h <- read.table(
	paste0(ws, "data/rr_obs_hourly_long.csv"),
	header = F, sep = ','
) %>% `colnames<-`(c('y', 'm', 'd', 'h', paste0("s", 1:n)))

for (i in 1:n) {
	df_aggre = df_obs_h[, c(1:4, i + 4)] %>% 
	`colnames<-`(c('y', 'm', 'd', 'h', 'rr')) %>% 
	group_by(y,m,d) %>%
	summarise(rr = sum(rr))
	if (i == 1) {
		df_obs_d = df_aggre
	} else {
		df_obs_d = cbind(df_obs_d, df_aggre[, 4])
	}
}
df_obs_d <- df_obs_d %>% `colnames<-`(c('y', 'm', 'd', paste0("s", 1:n)))
write.table(
	df_obs_d,
	paste0(ws, "data/rr_obs_daily_long.csv"),
	col.names = F, row.names = F, append = F, quote = F, sep = ","
)

df_cp <- read.table(
	paste0(ws, "data/cp_series_long.csv"),
	header = F, sep = ','
) %>% `colnames<-`(c('y', 'm', 'd', 'cp'))

df_index <- read.table(
	paste0(ws, "data/candidates.txt"),
	header = T
)

# --- check the CP class
cp_obs = 2

for (i in 1:dim(df_index)[1]) {
	df_can <- df_obs_d[df_index$index[i]+1, ] %>% as.data.frame()
	cp_can = df_cp[df_cp[,1] == df_can[1,1] & df_cp[,2] == df_can[1,2] & df_cp[,3] == df_can[1,3], 4]
	print(cp_can)
}

# --- check the wet-dry status

for (i in 1:dim(df_index)[1]) {
	df_can <- df_obs_d[df_index$index[i]+1, ] %>% as.data.frame()
	for (j in 1:n) {
		if (df_rwg[1, 3 + j] > 0.0 & df_can[1, 3 + j] <= 0.0) {
			print("mismatch")
		}
	}
}

df_cans <- df_obs_d[df_index$index+1, ] %>% as.data.frame()
write.table(
	df_cans,
	paste0(ws, "data/rr_obs_daily_long_candadites.csv"),
	col.names = F, row.names = F, append = F, quote = F, sep = ","
)

df_random <- read.table(
	paste0(ws, "data/random_output.txt"),
	header = F, sep = ','
) %>% `colnames<-`(c("rd", "fragment"))

for (i in 1:dim(df_random)[1]) {
	if (!(df_random$fragment[i] %in% df_index[,1])) {
		print0(df_random$fragment[i])
		
	}
}
