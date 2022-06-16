echo off
rem Analyze the independant patches in rand_250
rem using the two created profiles
echo i1Patches -C rand_250_M2.txt x6_M2A.icm opt957_M2A.icm
i1Patches -C rand250_M2.txt x6_M2A.icm opt957_M2A.icm >>log.txt
echo on
