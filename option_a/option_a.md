### Option A

Create new CGATs file by copying all or a part of one or more other CGATs files of the same type.
The result is optionally randomized. This is primarily useful to construct RGB CGATS files from
all or portions of RGB CGATs files.

    i1Patches -A cgats_combined.txt cgats1.txt[start stop] cgats2.txt[start stop] ....
        Combines and one or more CGATs files. Files can be RGB
        only or measurement files but must all have the same layout.
        Optionally, start and stop indexes can be specified for each cgats.For instance:
        "cgats1.txt 10 55" will retrieve the 10'th through 55'th entries.
        After combining the patches are randmized and the CGATs file is saved.

    i1Patches -Ad cgats_combined.txt cgats1.txt cgats2.txt ....
        Same as above but patches are de-randomized.

    i1Patches -a cgats_combined.txt cgats1.txt cgats2.txt ....
        Same as above but patches are not randomized.


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

scrambled.txt:

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
