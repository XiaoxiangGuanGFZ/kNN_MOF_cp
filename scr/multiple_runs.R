library(tidyverse)
ws <- "D:/kNN_MOF_cp/data/"
df <- read.table(
  paste0(ws, "rr_obs_hourly.csv"),
  header = F, sep = ','
)

out = df %>% group_by(V1, V2, V3) %>%
  summarise(V5 = sum(V8))

df <- read.table(
  "D:/ran.txt",
  header = F, sep = ','
) %>% t() %>% as.data.frame()
hist(
  df$V1
)

dates <- seq(
  as.Date("2006-01-01"),
  as.Date("2020-12-31"),
  1
)
dates[225+1]

which(
  dates == as.Date("2007-05-17")
)
501

