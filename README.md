# pda-midi-player

The app is written in C++, and comprised of the following main parts:
A software synthesizer for generating PCM wave data from MIDI.  The source code of the synthesizer is placed under the "src\common" folder.
IO layer, including File IO,  Wave output and RS232 under the "src\win32" folder.
Wavpack, a third party library for decompressing wave data in sound bank file.
Libmad, a third party library for decompressing MP3 data.
Registration system, which uses Small compiler, a third party embedded script language library, to hide the algorithm of registration key. The algorithm of registration key is in the source file "calckey.sma" which is written in Small Language and interpreted by "amx.c".
MIDI encrypt/decrypt system, which uses UCL, a third party library, to compress the MIDI file, then uses bit operation and arithmatic operation to obfuscate the compressed data. The encrypt/decrypt code can be found in .cpp file under the  "DecapPlayer\encmid" and "DecapPlayer\decmid" folder.
UI code, which uses Windows CE USER32&GDI API to show the user interface.
The app reads MIDI data from file, sends it to software synthesizer and RS232 COM port. The software synthesizer generates WAV data from MIDI data, and send the WAV data to audio output.

3rd party library used:
Wavpack, Libmad, Small compiler (an old version of the PAWN language), UCL

No 3rd party framework is used.

Use  MS eMbedded Visual C++ 3.0 with latest service pack to build/debug the app.

Most code including the software synthesizer, MIDI encrypt/decrypt, Wavpack, Libmad, UCL are platform independent, which means they should be able to get compiled for Android without much effort.
But the IO code and UI code are for Windows only, they can't be reused in Android and have to be written.

To build PDA player:
1. Install MS eMbedded Visual C++ 3.0 with latest service pack.
2. Open PDA\DecapPlayer\MidiPlayer.vcw in MS eMbedded Visual C++ 3.0.
3. In menu "Build->Set Active Configuration", choose "MidiPlayer - Win32 [WCE ARM] Released".
4. Press F7 to build MidiPlayer.exe.
5. Open file folder PDA\DecapPlayer\install, run update.bat, wait till finished.
6. Run build.bat to generate midiplayer.ppc30_arm.CAB. 


