### Option A

Create new CGATs file by copying all or a part of one or more other CGATs files of the same type.
The result is optionally randomized. This is primarily useful to construct RGB CGATS files from
all or portions of RGB CGATs files. Argyll ".ti1" RGB files with multiple sections can be
included and RGB values will be rescaled from 0:100 to 0:255 for CGATS compatibility with i1Profiler.

Argyll .ti1 files can be read which allows for comparing profile performance of
Argyll patch sets. When .ti1 files, which can have multiple sections, are aggregated,
details for each section are printed out.

    argyll_file.ti1:
    .ti1 section 1 starts at 1 and ends at 957
    .ti1 section 2 starts at 958 and ends at 965
    .ti1 section 3 starts at 966 and ends at 974

Command option details:

    i1Patches -A cgats_combined.txt cgats1.txt [start stop] cgats2.txt [start stop] ....
        Combines and one or more CGATs files. Files can be RGB
        only and/or measurement files but must all have the same type
        and layout if measurement files. If the cgats_combined.txt target file
        does not end in _Mn.txt (n=0,1,2) then only RGB values are aggregated.
        Optionally, start and stop indexes can be specified for each cgats file.
        For instance: "cgats1.txt 10 55" will retrieve the 10'th through 55'th entries.
        Multiple ranges can be given. "cgats1.txt 1 5 7 10" will retrieve
        the first 9 values skipping the 6th.
        After combining the patches are randmized and the CGATs file is saved.

    i1Patches -Ad cgats_combined.txt cgats1.txt cgats2.txt ....
        Same as above but patches are de-randomized.

    i1Patches -a cgats_combined.txt cgats1.txt cgats2.txt ....
        Same as above but patches are not randomized or de-randomized.


#### Example
This execute.bat command file creates a CGATs scrambled.txt file from i2.txt
and the first two entries of i3.txt. It then descrambles scambled.txt to
create descrambled.txt.


    i1patches -A scrambled.txt i2.txt i3.txt 1 2
    i1patches -Ad descrambled.txt scrambled.txt

i2.txt:

    BEGIN_DATA
       1    0.00   0.00   0.00
       2    1.00   1.00   1.00
    END_DATA

i3.txt:

    BEGIN_DATA
       1    2.00   2.00   2.00
       2    3.00   3.00   3.00
       3    4.00   4.00   4.00
    END_DATA

scrambled txt:

    BEGIN_DATA
    1	  3.00	  3.00	  3.00
    2	  1.00	  1.00	  1.00
    3	  2.00	  2.00	  2.00
    4	  0.00	  0.00	  0.00
    END_DATA

descrambled txt:

    BEGIN_DATA
    1	  0.00	  0.00	  0.00
    2	  1.00	  1.00	  1.00
    3	  2.00	  2.00	  2.00
    4	  3.00	  3.00	  3.00
    END_DATA
