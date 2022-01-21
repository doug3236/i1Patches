### Option Z, Printer drift measurement tool


    i1patches -Z rows file.txt
        Create printer drift pattern and neutral ramp for testing color patch stability.
        A RGB 4x4x4 plus 2:2 : 30 dev neutrals repeated to fill selected size
        rows * 29 total patches.rows must be between 21 and 33 inclusive.
        Created are an i1isis or i1Pro2 compatible file.tif and cgats/txf files, file.txt and filer.txt.

    i1patches -Z  measurementfile1.txt [measurementfile2.txt]
        Evaluate patch color consistency and L * ramp to adjust ink levels.
        For two files, evaluate statistical differences between the two measurement files.This is
        intended to test variation over time of 1 printed chart or comparing measurement
        from 2 charts that were printed at different times, possibly with different ink levels
        or after changing ink.



This option generates a tif file and associated CGATs and txf files intended to measure
printer ink color drift. It can be used when changing ink cartridges to determine inking
consistency when printing. This is useful for comparing long term color drift for
the same print as well as comparing printer changes over time when different pages
are printed after a time interval.

    i1patches -Z nrows file.txt

A single page, with 21 to 33 rows and 29 columns is produced. This consists of 19
device neutral patches and 60 color patches for a total of 79 patches. These are
duplicated then scrambled to fill the page (between 609 and 957 total).

After the patches are read with i1Profiler, the CGATs files are compared