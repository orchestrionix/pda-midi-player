<html>
<body>
<pre>
<h1>Build Log</h1>
<h3>
--------------------Configuration: MidiPlayer - Win32 (WCE ARM) Release--------------------
</h3>
<h3>Command Lines</h3>
Creating temporary file "C:\DOCUME~1\athena\LOCALS~1\Temp\RSPB.tmp" with contents
[
/nologo /W3 /Ob2 /I "..\src" /I "..\src\win32" /I "." /D "ARM" /D "_ARM_" /D "NDEBUG" /D _WIN32_WCE=300 /D "WIN32_PLATFORM_PSPC" /D UNDER_CE=300 /D "UNICODE" /D "_UNICODE" /D "DECAP_PLAYER" /D "FPM_DEFAULT" /D inline=__inline /D "OPT_SPEED" /Fp"ARMRel/MidiPlayer.pch" /YX /Fo"ARMRel/" /Oxs /MC /c 
"C:\Projects\PowerSynth\DecapPlayer\Player.cpp"
]
Creating command line "clarm.exe @C:\DOCUME~1\athena\LOCALS~1\Temp\RSPB.tmp" 
Creating temporary file "C:\DOCUME~1\athena\LOCALS~1\Temp\RSPC.tmp" with contents
[
commctrl.lib coredll.lib aygshell.lib /nologo /base:"0x00010000" /stack:0x10000,0x1000 /entry:"WinMainCRTStartup" /incremental:no /pdb:"ARMRel/MidiPlayer.pdb" /map:"ARMRel/MidiPlayer.map" /nodefaultlib:"libc.lib /nodefaultlib:libcd.lib /nodefaultlib:libcmt.lib /nodefaultlib:libcmtd.lib /nodefaultlib:msvcrt.lib /nodefaultlib:msvcrtd.lib /nodefaultlib:oldnames.lib" /out:"ARMRel/MidiPlayer.exe" /subsystem:windowsce,3.00 /align:"4096" /MACHINE:ARM 
.\ARMRel\blit16to16.obj
.\ARMRel\blit8to16.obj
.\ARMRel\blita16to16.obj
.\ARMRel\blita8to16.obj
.\ARMRel\DIBSurface.obj
.\ARMRel\Surface.obj
.\ARMRel\resmgr.obj
.\ARMRel\ucl_dcmp.obj
.\ARMRel\bits.obj
.\ARMRel\unpack.obj
.\ARMRel\words4.obj
.\ARMRel\PsAudioSystem.obj
.\ARMRel\PsChorus.obj
.\ARMRel\PsEG.obj
.\ARMRel\PsFileReader.obj
.\ARMRel\PsFilter.obj
.\ARMRel\PsInstrument.obj
.\ARMRel\PsLFO.obj
.\ARMRel\PsMath.obj
.\ARMRel\PsMidiIO.obj
.\ARMRel\PsMixer.obj
.\ARMRel\PsOscillator.obj
.\ARMRel\PsPatch.obj
.\ARMRel\PsPatchWave.obj
.\ARMRel\PsPortRS232.obj
.\ARMRel\PsPortSoftSynth.obj
.\ARMRel\PsReverb.obj
.\ARMRel\PsReverbNoAllPass.obj
.\ARMRel\PsReverbSchroeder.obj
.\ARMRel\PsSequence.obj
.\ARMRel\PsSequencer.obj
.\ARMRel\PsSoundBank.obj
.\ARMRel\PsSynthesizer.obj
.\ARMRel\PsSys.obj
.\ARMRel\PsTrack.obj
.\ARMRel\PsVolEG.obj
.\ARMRel\PsWinDriver.obj
.\ARMRel\bit.obj
.\ARMRel\fixed.obj
.\ARMRel\frame.obj
.\ARMRel\huffman.obj
.\ARMRel\layer12.obj
.\ARMRel\layer3.obj
.\ARMRel\stream.obj
.\ARMRel\synth.obj
.\ARMRel\timer.obj
.\ARMRel\xing.obj
.\ARMRel\amx.obj
.\ARMRel\DlgBase.obj
.\ARMRel\DlgChannel.obj
.\ARMRel\DlgDecapSetting.obj
.\ARMRel\DlgDirTree.obj
.\ARMRel\DlgOption.obj
.\ARMRel\DlgPlEdit.obj
.\ARMRel\MidiPlayer.obj
.\ARMRel\MP3Channel.obj
.\ARMRel\Player.obj
.\ARMRel\PlayList.obj
.\ARMRel\Register.obj
.\ARMRel\RegisterCalcKey.obj
.\ARMRel\RegisterCore.obj
.\ARMRel\Skin.obj
.\ARMRel\StdAfx.obj
.\ARMRel\Utils.obj
.\ARMRel\MidiPlayer.res
]
Creating command line "link.exe @C:\DOCUME~1\athena\LOCALS~1\Temp\RSPC.tmp"
<h3>Output Window</h3>
Compiling...
Player.cpp
C:\Projects\PowerSynth\DecapPlayer\Player.cpp(143) : warning C4244: '=' : conversion from '__int64' to 'int', possible loss of data
Linking...



<h3>Results</h3>
MidiPlayer.exe - 0 error(s), 1 warning(s)
</pre>
</body>
</html>
