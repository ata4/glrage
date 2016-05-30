GLRage is an OpenGL wrapper for the ATI 3D C Interface (ATI3DCIF) and DirectDraw 2. It allows to play vintage Windows 95/98 games designed for the ATI 3D Rage series on modern GPUs and Windows versions in FHD, UHD and beyond.

It is based on a heavily modified version of GLCIF that was originally tailored specifically for Tomb Raider, which has been ported to modern OpenGL and C++ with the goal to support more games.

It also supports non-destructive memory patching, which allows bug fixes and enhancements while leaving the original .exe files untouched.

# Requirements

* Windows 7 or higher (may work on Vista, but I haven't tested it).
* An OpenGL 3.3 compatible graphics card. Anything that was made post 2010 should be fine.
* [Visual C++ Redistributable for Visual Studio 2015.](https://www.microsoft.com/en-us/download/details.aspx?id=48145)

# Tested Games

## Tomb Raider (including Unfinished Business/Gold)

The original patch for the ATI Rage Pro was released in a pretty unfinished state and contains a mixture of old DOS version bugs, Windows port bugs and new bugs introduced by ATI.
Therefore, a lot of patches are required to fully restore the original Tomb Raider features and aesthetics.

### Installation instructions

For the absolute minimum installation, the original Tomb Raider CD as well as the correct ATI patch besides this wrapper is required:

* [ATI patch for Tomb Raider](http://web.archive.org/web/20081208051118/http://ftp.eidos-france.fr/pub/fr/tomb_raider/patches/tombatiragepro.zip)
* [ATI patch for Tomb Raider: Unfinished Business](http://web.archive.org/web/20081208051118/http://ftp.eidos-france.fr/pub/fr/tomb_raider/patches/tombati.zip) (executable only, requires ESCAPE DLLs included in the patch above)

On default, Tomb Raider is patched for a standalone installation, so you'll need to copy the `DATA` and `FMV` folders from the CD to the TR directory. If you want to use the CD instead, you have to set `patch_nocd` to `false` in `tombati.ini`/`tombub.ini`.

To have working music in this mode, you'll need a WinMM wrapper and audio track rips as well. To unlock the full soundtrack, you also need to set `patch_soundtrack` to `true` and have all tracks set up in the right order.
[Here's an overview of all TR1 CD audio tracks,](https://docs.google.com/spreadsheets/d/1xQsqGF-T0oqv_TxdgAMJr31XDiJrnch5tuas6QpXzsI/) GLRage uses "Raw" if `patch_soundtrack` is enabled.

Also, you'll need to separate folders for TR and TR:UB to avoid clashing files.

If you have the GOG.com or Steam release, you'll find the two folders in GAME.GOG/GAME.DAT, which is actually a BIN/CUE image of the Sold-Out Software release of TR. You can temporary rename GAME.DAT to GAME.cue and mount it
with [WinCDEmu](http://wincdemu.sysprogs.org/) to access the image.

### Hotkeys

With the wrapper installed, following key combinations are available:

* Print screen: creates a screenshot in the Targa format inside the current game directory.
* Alt + enter: switch between windowed/fullscreen mode.

### Patches

#### Forced patches
* Fixed a startup crash on modern Windows versions.
* Fixed an explosion effect crash when killing the Tihocan Centaurs or the Giant Atlantean.
* Fixed Tomb Raider CD not being detected in the drives 'A' or 'B'.
* Fixed an integer overflow that caused long voice samples to be missing or cut off.
* Fixed broken demo mode.
* Fixed broken credit screens.
* Fixed movement keys sometimes getting stuck in a permanently pressed state until the game has been restarted.
* Fixed missing looping sounds (rivers, waterfalls, lava, etc.).
* Fixed music volume control having no effect (changes are applied when the next track is played).
* Fixed movie interlacing and low brightness.
* Fixed Eidos title screen appearing for just one frame.
* Fixed sound issues when multiple active sound effects share the same sound buffer.
* Reduced stereo panning issues when sounds are played very closely to Lara's position.

#### Optional patches
* The 800x600 resolution can be customized. ([7680x4320 Screenshot](http://i.imgur.com/RpXgWDD.jpg))
* Increased vertex color brightness to match DOS and PSX versions. ([Screenshot 1](http://i.imgur.com/S7GP9hH.jpg) / [Screenshot 2](http://i.imgur.com/xqJflAi.jpg))
* Changed water color from grey-blue to turquoise, similar to the PSX version. ([Screenshot](http://i.imgur.com/NpYRg9j.jpg))
* Increased rendering distance. ([Screenshot](http://i.imgur.com/CUnIoIY.jpg), disabled on default)
* No-CD mode. (enabled on default)
* Customizable field of view.
* Full soundtrack mode, combining PC ambient tracks with PSX music tracks. (disabled on default)

### Known problems
* The savegame format is incompatible to DOS format. To prevent accidental loading or overwriting of existing DOS savegames, the file name format has been changed as well.
* Custom key bindings have been partially disabled in the ring menu to prevent various crashes.
* Some objects disappear when viewed from certain angles or are just partially visible.
* Ambient tracks played from a physical CD won't loop properly in Windows Vista and later ([caused by a MCI emulation bug in Windows](https://social.msdn.microsoft.com/Forums/windowsdesktop/en-US/dfa22274-8122-4b92-a0bc-653f5749b1bd/audio-cd-playing-with-mci-mcinotifysuccessful-never-arrives?forum=windowspro-audiodevelopment)).
* If the soundtrack patch is activated, music cues triggered by levels are played again when loading a savegame where they have been played already.

## Assault Rigs

Apart from a broken memory check, Assault Rigs runs almost flawlessly out of the box, even on recent Windows versions.

### Installation instructions

You'll need the 3D Rage Edition of Assault Rigs that was bundled with ATI 3D Xpression cards. Patching the DOS version is not possible, since both game files and CD audio track order are different.

Unlike the DOS version, the ATI edition has no CD check, so it'll run without if you don't need the music. For a complete standalone installation with music, you'll need a WinMM wrapper and ripped audio tracks.

### Patches

#### Forced patches
* Fixed "Insufficient memory" error on systems with more than 4 GB RAM.

#### Optional patches
* Replaced 640x480 resolution option with the current desktop resolution for widescreen support.

### Known problems
* Slightly low subpixel vertex accuracy, which causes wobbly polygons in higher resolutions.

## Wipeout

You'll need the 3D Rage Edition of Wipeout that was bundled with Sony Vaio PCV laptops. A patch for the DOS version may be possible but is not yet available.

Wipeout runs mostly stable, but there are some graphical glitches right now.

#### Forced patches
* Preventing the game from messing with the key repeat rate and screensaver settings.

#### Optional patches
* Added option to skip the title screen and intro video.

### Known problems
* Low subpixel vertex accuracy, which causes wobbly polygons in higher resolutions.

# Compiling

You'll need Visual Studio 2015 to compile the source files. Additionally, you will also need the [ATi 3D Rage Pro SDK 4.1](http://www.vogonsdrivers.com/getfile.php?fileid=497) and extract it to the repo's main folder.
You can also just extract ATI3DCIF.H header file to `ragesdk/include`, since the rest isn't required for compiling.

# License
This software is provided under GNU Lesser General Public License version 3.
See LGPL-3 or http://www.gnu.org/licenses/lgpl.html for the full text of this license.