### Option T: Create aggregated, structured charts to combine and analyze one or more RGB patch sets


    i1Patches -[T|T2] n_rows[-e size] patch_set1.txt, patch_set2.txt, ...
        Create RGB CGATs/txf files and tif image files. "n_rows" specifies the number of
        rows on each page. It must be between 21 and 33 for i1isis, the "-T" option,
        and exactly 21 for the i1Pro2, the "-T2" option.
        There are always 29 columns. Recommend 33 rows for full i1isis profile page size.
        The optional "-e size" can be used to create a set of "size" random, RGB patches to compare
        profile performance against. 500 or more will provide reasonable statistical
        comparisons of accuracy between profiles. Zero or more "-e" options are allowed.
        if no size argument is provided, a default of 250 is used.
        if (size >= 250)      // add 52 neutral axis RGBs, split remaining among low saturation and full gamut.
        else if (size > 100)  // add 18 neutral axis RGBs, split remaining among low saturation and full gamut.
        else if (size <= 100) // Just add full gamut randomly spread RGB patches.

        i1Patches -[T|T2] n_rows[-e size dup_count] patch_set1.txt, patch_set2.txt, ...
        Same as above but all the "-e" patches generated are duplicated. This provides a means
        of determining how much variation occurs when the same RGB patches printed
        in different locations.

        i1Patches -[T|T2] n_rows [-e size dup_count random_seed] patch_set1.txt, patch_set2.txt, ...
        Same as above but seed can be specified with specific values used to create
        the random patch set.This can be useful to check the variation that occurs
        by creating multiple patch sets with or without the same RGB values.

        i1Patches -[T|T2] -n_rows patch_set.txt
        Note : n_rows is negative!.No coded structure or randomization applied.
        This generates tif files and forward/reverse CGATs RGB files _x appended to
        the base name. Useful for making tif profile targets that can be read both forward and
        reverse with an i1isis or i1Pro 2 spectro. RGB CGATs file will be expanded
        with white patches to be a multiple of 29*n_rows.
        It also produces charts the same as would appear in i1Profiler.

        i1Patches -t ...
        Same as "-T" above but only create RGB CGATs/txf files and
        doesn't create tif images files. Use this when using
        i1Profiler to print target images from the txf or RGB CGATs files.
        In i1Profiler, use the default patches. 6mm for i1isis(Profile)
        and 8mm(Landscape) for i1Pro2 on US letter size paper.

        i1Patches ACPU ......Rest of command line as above....... 
        This increases size by 2% to adjust for shrinkage when using Adobe's ACPU
        and should not be used with other utilities that print w/o color management.


Create i1isis or i1Pro 2 charts to compare performance of one or more patch sets.
Combines one or more RGB patch sets with optional, independantly generated random RGB patches.

As an example we use the custom 957 patch set and add random patches for accuracy verification.
Initial RGB file: **o957.txt**, a CGATs custom RGB set.

Running execute.bat creates tif charts, CGATs RGBs, and "txf" chart files compatible with i1Profiler.
Aggregate files with base names ending in "r" are for use to read charts in reverse order.
Last page first, and bottom to top. The i1isis charts have an additional registration bar on the
bottom to facilitate this. When printing, it's important to adjust the top/bottom margin so
that there is similar space from the registration bar to the paper's top/bottom if you intend to read
both the forward and reverse spectra.

Executing execute.bat

    Command: i1patches  -T2 21 -e 800 o957.txt
    Randomizing and adding edge check colors
    3 pages of 21 rows with 29 columns
    21 check colors are added to the right edge of each page
    Randomized patch count=1757
    Total patch count=1827
    3 patches can be added before additional pages are created
    Creating single, grouped CGATs and Tif image files: rand800__o957

    Command: i1patches  -T 33 -e 900 o957.txt
    Randomizing and adding edge check colors
    2 pages of 33 rows with 29 columns
    25 check colors are added to the right edge of each page
    Randomized patch count=1857
    Total patch count=1914
    10 patches can be added before additional pages are created
    Creating single, grouped CGATs and Tif image files: rand900__o957

****Note for users of Adobe ACPU print utility****<br>
*When using Adobe's ACPU utility to print tif files without color management add "ACPU"
as the first argument preceding the "-T" like this **i1Patches ACPU -T ...rest of command...**. This expands the
printed image by 2% to mostly correct for ACPU's tendency to shrink prints slightly.*
