### Option R: Extract individual CGATs files from an aggregated CGATs file.

#### Extracting embedded CGATs files 

    i1Patches -R measurement_file_M2.txt
        Extract measurement files from an aggregated, CGATs, measurement file.
        Note that the file can end in _M0.txt, _M1.txt or _M2.txt.
        The measurement file should be in i1Profiler's CGATs format that includes,
        at a minimum, RGB and Lab values. But one can also select all fields including
        spectral data. For added data integrity, i1isis tif files can be scanned
        in reverse from bottom to top and last to first page. If a CGATs measurement file
        of the same name except ending in r_M2.txt instead of _M2.txt exists the
        two files will be averaged and statistics printed on how closely the
        patches match. This is useful for detecting transient issues such as lint on the charts.

        i1Patches -r measurement_file_M2.txt
        Same as above but doesn't extract files. Just outputs statistics.

        i1Patches -Ra measurement_file_M2.txt
        Same as -R but also creates Argyll batch file to make profiles.

#### Creating an averaged measurement file from forward/reverse files

        i1Patches -Rx measurement_file_M2.txt
        Only averages and saves the forward and reverse scans and prints stats.The averaged
        file will be saved as *_ave_M2.txt. and can be any set of forward/reverse measurement files.
        No structure, as produced with "-T" option, is required.


The -R option de-randomizes and extracts the embedded files from a structured, CGATs,
measurement file originally created with the -T option.  It creates individual CGATs
files for each patch set. The first patches in the
measurement file contain encoded RGB values from the names and sizes of aggregated
files. This is used to reconstruct the associated measurement files.

    i1patches -R rand500__i400__o957_M2.txt

This de-randmizes and extracts CGATs measurement files.
Since these CGATs were randomized and distributed across the printed charts any differences
due to first printed page or paper characteristics are also distributed improving 
comparisons.

    rand500_M2.txt
    i400_M2.txt
    o957_M2.txt

In addition, since we included a reversed CGATs measurement file, it is averaged with
the forward scan and saved as **rand500__i400__o957_ave_M2.txt**. The other extracted
measurement files are also automatically averaged.


A summary of statistics associated with each printer page are output in the ***log.txt*** file.
There are repeating sequences of 10 color patches along the right edge of each measured page.
These are 4 neutrals, 3 primaries and 3 light primaries.


    Command: i1patches  -R rand500__i400__o957_M2.txt

    File: rand500__i400__o957_M2.txt
    3 black patches, ave Lab:  1.91 -0.02 -0.21
    3 white patches, ave Lab: 94.77 -1.31 -2.24

    33 rows, 2 pages

    Extracting Measurement Files: rand500_M2.txt i400_M2.txt o957_M2.txt 
    File:rand500__i400__o957_M2.txt
    Check Colors per Page: 23
                RGB         Mean L*a*b*        StDev L*a*b*        Slope L*a*b*
    NeutH :  200 200 200   76.1  -0.9   0.5    0.08  0.04  0.05    0.06 -0.02  0.06
    NeutM1:  150 150 150   60.0   0.2   2.7    0.35  0.05  0.05    0.06 -0.04  0.02
    NeutM0:  100 100 100   41.6   2.6   0.6    0.46  0.02  0.06    0.16 -0.01  0.04
    NeutL :   50  50  50   20.5   4.6  -2.3    0.28  0.05  0.17    0.16 -0.06  0.04
    LMag  :  255 180 255   81.6  24.6 -12.9    0.08  0.13  0.09    0.11 -0.06  0.11
    LYel  :  255 255 180   93.5  -6.9  27.1    0.03  0.01  0.10    0.06  0.01  0.03
    LCyn  :  180 255 255   84.9 -16.3 -16.7    0.08  0.13  0.14    0.07  0.00  0.07
    Mag   :  255   0 255   51.4  80.5  -6.7    0.12  0.05  0.16    0.17  0.06 -0.26
    Yel   :  255 255   0   89.3  -5.6 109.2    0.07  0.18  0.08    0.08 -0.28  0.01
    Cyn   :    0 255 255   57.8 -43.9 -54.4    0.08  0.09  0.10    0.12 -0.12  0.16

                     Lab pg: 1           Lab pg: 2          
    NeutH       76.0  -0.8   0.4    76.1  -0.9   0.5
    NeutM1      60.0   0.2   2.7    60.0   0.2   2.7
    NeutM0      41.6   2.6   0.6    41.7   2.6   0.6
    NeutL       20.4   4.6  -2.4    20.6   4.6  -2.3
    LMag        81.5  24.6 -13.0    81.6  24.5 -12.9
    LYel        93.5  -6.9  27.1    93.5  -6.9  27.2
    LCyn        84.8 -16.3 -16.8    84.9 -16.3 -16.7
    Mag         51.3  80.4  -6.6    51.5  80.5  -6.8
    Yel         89.2  -5.5 109.2    89.3  -5.7 109.2
    Cyn         57.8 -43.8 -54.5    57.9 -44.0 -54.4

               Difference of each page from global average
    NeutH       -0.0   0.0  -0.0     0.0  -0.0   0.0
    NeutM1      -0.0   0.0  -0.0     0.0  -0.0   0.0
    NeutM0      -0.1   0.0  -0.0     0.1  -0.0   0.0
    NeutL       -0.1   0.0  -0.0     0.1  -0.0   0.0
    LMag        -0.1   0.0  -0.1     0.1  -0.0   0.1
    LYel        -0.0  -0.0  -0.0     0.0   0.0   0.0
    LCyn        -0.0  -0.0  -0.0     0.0   0.0   0.0
    Mag         -0.1  -0.0   0.1     0.1   0.0  -0.1
    Yel         -0.0   0.1  -0.0     0.0  -0.1   0.0
    Cyn         -0.1   0.1  -0.1     0.1  -0.1   0.1

    Mean Std Dev per page:  0.1153 0.0923


    Forward/Reverse stats, 5 worst dEs
    Patch#  dE76        RGB              Lab Forward               Lab Reverse         page  row  column
    1487    0.56      0  73   0     17.59  -27.39   11.25      17.49  -27.73   11.69    2      2    17
      44    0.50    255   0   0     49.76   77.44   74.15      49.68   77.27   74.61    1     11     2
     791    0.48     42  36   0     15.54    3.98   14.76      15.57    3.82   15.21    1     32    24
     995    0.42    255   0   0     49.70   77.34   73.77      49.75   77.37   74.19    2      5     2
     158    0.42      0 255 168     47.35  -63.49  -29.86      47.21  -63.12  -30.01    1     26     5

                 dE76 distribution (less than)
       10%   20%   30%   40%   50%   60%   70%   80%   90%
      0.06  0.08  0.10  0.11  0.13  0.15  0.16  0.19  0.23
