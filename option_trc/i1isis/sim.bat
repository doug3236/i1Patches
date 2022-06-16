echo off
rem aggregate and create tif files from x6.txt, a 371 rgb patch set
rem and opt957.txt, an optimized 957 RGB patch set
rem also create a 500 patch, uncorrelated random patch set with neutral ramp
echo i1patches -T2 21 -e 500 x6.txt opt957.txt
i1patches -T 33 -e 500 x6.txt opt957.txt >log.txt
echo - >>log.txt

rem create simulated measurement files by using an existing profile
rem and adding in .2 noise with .05 noise between simulated fwd/rev files
rem normal use use i1profiler together with the printed charts make
rem the measurement file(s)
echo i1patches -X .2 .05 P_rand500__x6__opt957.txt p4357_M2.icm
i1patches -X .2 .05 P_rand500__x6__opt957.txt p4357_M2.icm >>log.txt
echo - >>log.txt

rem de-randomize and extract the individual files:
rem s6_M2.txt opt957_M2.txt, rand500_M2.txt
rem the "-Ra" option also creates argyll.bat which automates
rem creating the ICC profiles.
echo i1patches -Ra P_rand500__x6__opt957_M2.txt
i1patches -Ra P_rand500__x6__opt957_M2.txt >>log.txt
echo - >>log.txt
echo on
