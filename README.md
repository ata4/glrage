This is an OpenGL wrapper for the ATI 3D C Interface (ATI3DCIF) and DirectDraw 2. It allows to play vintage Windows 95/98 games designed for the ATI 3D Rage series on modern GPUs and Windows versions in FHD, UHD and beyond.

It is based on a heavily modified version of [GLCIF](http://glcif.sourceforge.net/) that was originally tailored specifically for Tomb Raider, which has been ported to modern OpenGL and C++ with the goal to support more games.

It also supports non-destructive memory patching, which allows .exe files to be untouched while applying patches on demand.

# Tested Games

## Tomb Raider (including Unfinished Business/Gold)

The original patch for the ATI Rage Pro was released in a pretty unfinished state and contains a mixture of old DOS version bugs, Windows port bugs and new bugs introduced by ATI.
Therefore, a lot of patches are required to fully restore the original Tomb Raider features and aesthetics.

### Patches

#### Forced patches
* Fixed a startup crash on modern Windows versions.
* Fixed an explosion effect crash when killing the Tihocan Centaurs or the Giant Atlantean.
* Fixed Tomb Raider CD not being detected in the drives 'A' or 'B'.
* Fixed an integer overflow that caused long voice samples to be missing or cut off.
* Fixed bugged demo mode and credit screens.
* Fixed missing looping sounds (rivers, waterfalls, lava, etc.).
* Reduced stereo panning issues when sounds are played very closely to Lara's position.

#### Optional patches
* Replaced 800x600 resolution option with the current desktop resolution for widescreen support.
* Increased vertex color brightness to match DOS and PSX versions. ([Screenshot 1](http://i.imgur.com/S7GP9hH.jpg) / [Screenshot 2](http://i.imgur.com/xqJflAi.jpg))
* Changed water color from grey-blue to turquoise, similar to the PSX version. ([Screenshot](http://i.imgur.com/NpYRg9j.jpg))
* Added No-CD mode. Requires `DATA` and `FMV` folders to be copied to the Tomb Raider directory and a WinMM wrapper for CD audio support.
* Added soundtrack patch, which unlocks the full soundtrack, combining PC ambient tracks with PSX music tracks. Requires a custom CD or WinMM wrapper with the additional tracks.

### Known problems

* Occasionally, the movement keys may get stuck in a permanently pressed state until the game has been restarted.
* Music volume control has no effect.
* The menu is fully opaque when opened in-game.
* Changing the key bindings in the options crashes the game.
* Clipped vertices sometimes have incorrect positions on the far left and right side of the screen when using a widescreen resolution.
* Savegame format is incompatible to DOS format.
* Ambient tracks played from the CD won't loop properly in Windows Vista and later ([caused by a MCI emulation bug in Windows](https://social.msdn.microsoft.com/Forums/windowsdesktop/en-US/dfa22274-8122-4b92-a0bc-653f5749b1bd/audio-cd-playing-with-mci-mcinotifysuccessful-never-arrives?forum=windowspro-audiodevelopment)).

## Assault Rigs

Apart from a broken memory check, Assault Rigs runs flawlessly even on recent Windows versions.

### Patches

#### Forced patches
* Fixed "Insufficient memory" error on systems with more than 4 GB RAM.

#### Optional patches
* Replaced 640x480 resolution option with the current desktop resolution for widescreen support.