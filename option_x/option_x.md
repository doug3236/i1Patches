### Option X: Simulate Measurements, Creates "measurement" files from an RGB file and ICC profile

    i1Patches -X noise0 noise1 cgats_aggregate_rgb_file.txt iccProfile.icm
        Generates pseudo forward/reverse measurement files using reference profile.
        Useful for testingand provides patch quaility info that tracks printed
        charts without printing.Provides initial guidance for selecting patch sets
        to printand test.noise0and noise1 are the standard deveation of gaussian noise added
        to synthesized Lab values from the RGB file.
        noise0 should approximate the printer's patch to patch variation. noise1 should
        approximate spectro instrument variation when reading the same patches.
        I use noise0 = .2 and noise1 = .05\n";


This option is useful for debugging and workflow checking without actually making measurements.
Additionally, if you aready have a high quality ICC printer profile you can use this to
explore the expected quality of small patch sets by applying it to the aggregate RGB file from
the -T option.

Example: Synthesize RGBLAB measurement file from the i1profiler 400 patch RGB CGATs file.

    i1patches -X .2 .05 i400.txt o957_medium_16bit.icm

This creates the two files: i400_M2.txt and i400r_M2.txt, the
synthesized forward and reverse "measurement" files. These can be used to create
profiles. Aside from debugging, useful when comparing sets of small patches to see which
is better providing a high quality, large patch set for the printer/paper already
exists.