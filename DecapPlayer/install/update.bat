md ppc30_arm
md ppc30_mips
md ppc30_sh3
copy ..\ARMRel\midiplayer.exe ppc30_arm
copy ..\MIPSRel\midiplayer.exe ppc30_mips
copy ..\SH3Rel\midiplayer.exe ppc30_sh3

copy ..\data\*.msk common
copy ..\data\*.msk.l common

attrib -r common\*.*
attrib -r ppc30_arm\*.*
attrib -r ppc30_mips\*.*
attrib -r ppc30_sh3\*.*
pause