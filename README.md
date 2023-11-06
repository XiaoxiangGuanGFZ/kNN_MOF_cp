# kNN_MOF_cp
k-nearest resampling (kNN) and method-of-fragments (MOF) based rainfall temporal disaggregation model conditioned on circulation patterns (cp) (written in c)

## 1. Introduction
A temporal disaggregation model is usually used to derive sub-daily (like hourly) distributions of weather variables from daily scale. kNN_MOF_CP is a non-parametric dtsaggregation procedure developed for multisite daily-to-hourly rainfall and air temperature disaggregation. In the procedure, mothod of fragments (MOF) based on k-nearest neighbor resampling conditioned on circulation pattern classification is applied. 
## 2. Techniques
### 2.1 Method Of Fragements
The idea of MOF is to resample a vector of fragments that represents the relative distribution of subdaily to daily rainfall (Pui et al., 2012). The number of fragments corresponds to the subdaily temporal resolution used, i.e. if the disaggregation is conducted from daily to hourly resolution, the relative distribution of subdaily values consists of 24 relative weights that sum up to 1
### 2.2 k-Nearest Neighbor algorithm
The k-nearest neighbors algorithm, also known as KNN or k-NN, is a non-parametric, supervised learning classifier, which uses proximity to make classifications or predictions about the grouping of an individual data point.
### 2.3 Circulation pattern classification
circulation patterns are divided on the basis of daily values of the mean sea-level pressure field of the ERA5 data set for the period from January 1979 to July 2021. 
The classification is conducted using the objective classification algorithm SANDRA (Simulated ANnealing and Diversified RAndomization) (Philipp et al., 2007). It is based on k-means and minimizes the within-cluster variance of the Euclidian distance between the cluster elements and the cluster centroid. 
## 3. How to use
### 3.1 Compile and run
#### 3.1.1 single-file version
- the program is written in c, with the compiler: gcc version 6.3.0 (MinGW.org GCC-6.3.0-1)
- to compile (in the command line / PowerShell with the path changed): `gcc ./scr/main.c`
- an executable application is generated: `a.exe`
- execute in command line again to run the programm: `./scr/a.exe ./data/global_para.txt`
- attention: do not forget the file path for both a.exe and parameter file

#### 3.1.2 multi-file version
- the algorithm is the same, just spliting the `main.c` into separate c source code files for better code organization and maintainability.
- c source code files include: `main_dis.c`, `def_struct.h`, `Func_dataIO.c` and `Func_kNN_MOF_cp.c`
- to compile (in PowerShell): `gcc main_dis.c Func_dataIO.c Func_kNN_MOF_cp.c`; `a.exe` will be produced and can be used as normal. 
- or CMake method: `CMakeLists.txt` is composed first and then call cmake tool `cmake -G "MinGW Makefiles" .` to generate the necessary build files (one of them is `Makefile`), at last call `mingw32-make` to produce the executable app `kNN_MOF_cp.exe` (the name specified in `CMakeLists.txt`)
- CMake tool can be found in https://cmake.org/, the generator I used here is still provided by MinGW, should be adapted according to specific(your) environments.

### 3.2 Global parameter file
`global_para.txt` provides the key information controlling main behaviors of kNN_MOF_cp disaggregation, including file path and algorithm parameters.
Detailed comments and explanation can be found in the example file `./data/global_para.txt`.
Lines starting with the letter # are comment (invalid) lines.

### 3.3 Multiple runs
After compiling the programm, the generated executable application can be called from other portal (for instance Matlab, R or Python). Therefore, the disaggregation can be conducted for multiple times for the same or different inputs (daily rr data) by tailoring  `global_para.txt`.

An example for multiple running by R is given here, see `./scr/multiple_runs.r`

## 4. Paper related
Xiaoxiang Guan, Katrin Nissen, Viet Dung Nguyen, Bruno Merz, Benjamin Winter, Sergiy Vorogushyn. Multisite temporal rainfall disaggregation using methods of fragments conditioned on circulation patterns. Journal of Hydrology, 621, 129640. doi: https://doi.org/10.1016/j.jhydrol.2023.129640

## 5. References
Pui, A., Sharma, A., Mehrotra, R., Sivakumar, B. and Jeremiah, E.  2012.  A comparison of alternatives for daily to sub-daily rainfall disaggregation. Journal of Hydrology, 470-471, 138-157. doi: https://doi.org/10.1016/j.jhydrol.2012.08.041.

Philipp, A., Della-Marta, P.M., Jacobeit, J., Fereday, D.R., Jones, P.D., Moberg, A. and Wanner, H.  2007.  Long-Term Variability of Daily North Atlantic–European Pressure Patterns since 1850 Classified by Simulated Annealing Clustering. Journal of Climate, 20(16), 4065-4095. doi: https://doi.org/10.1175/jcli4175.1.

## Author
[Xiaoxiang Guan](https://www.gfz-potsdam.de/staff/guan.xiaoxiang/sec44)

Email: guan@gfz-potsdam.de
