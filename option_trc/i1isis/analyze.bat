echo off
rem Analyze the independant patches in rand536
rem using the two created profiles
echo i1Patches -C rand_536_M2.txt x6_M2A.icm opt957_M2A.icm
i1Patches -C rand536_M2.txt x6_M2A.icm opt957_M2A.icm >>log.txt
echo on
