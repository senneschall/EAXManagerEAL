# EAXManager EAL

The aim of this repository is to collect enough of the information out there to eventually allow the use of the *.eal* files coming with *Creative's EAX-Patches* in later patches of *Unreal Tournament*.

There are two main goals:

1. understand Creative's *EaxMan.dll* so it can be used in UnrealTournament v469+

2. create a platform-independent alternative implementation to *EaxMan.dll*

## 

## The story so far

In 2001, *Creative* released a patch that added level-specific EAX presets. The website for that patch is gone now, but it was archived here: [EAX enhancements for Unreal Tournament](https://web.archive.org/web/20050320092918/http://eax.creative.com/eagle/unreal/)

Here is a demonstration of the patches: [Unreal Tournament's Special EAX Version: Deck16 Demo - YouTube](https://www.youtube.com/watch?v=oTwO91NBOn0)

Those advanced sound effects needed sound hardware supporting EAX ([Environmental Audio Extensions - Wikipedia](https://en.wikipedia.org/wiki/Environmental_Audio_Extensions)). So the sound effects were applied in hardware and needed the *DirectSound3D* API, which was part of *DirectX* from version 3 up to version 7.

Starting with Windows Vista, Microsoft deprecated *DirectSound3D*, which meant a loss of hardware acceleration, and therefore the EAX effects are no longer accessible.

To restore the hardware acceleration, *Creative* released [ALChemy](https://support.creative.com/Downloads/searchdownloads.aspx?strstring=almy&showdetails=1). This translates the calls to *DirectSound3D* into hardware-accelerated *OpenAL* calls. The downside is you must have specific sound hardware. The list of supported hardware is limited to some *Creative* SoundBlaster cards.

##### Make EAX effects work in modern times

When *OpenAL* was developed, it added the Effects Extension (**EFX**), which enables *EAX* calls to be processed in *OpenAL*. Eventually, *Creative* ceased the *OpenAL* development, but a software implementation of *OpenAL* is still actively developed: [OpenAL Soft is a software implementation of the OpenAL 3D audio API.](https://github.com/kcat/openal-soft/) *OpenAL Soft* implements the *EFX* extension in software, which makes it independent of specific audio hardware. Therefore, the effects can be used with any sound card.

To make use of *EFX*, the game needs to use *OpenAL*, which the Windows version of Unreal Tournament v436 does not. The *Creative EAX patches* only run on this version. They include a modified *Galaxy.dll*, which does not use *OpenAL*. But it can use the *DirectSound3D* API. As we've learned, it does not work under Windows Vista and newer versions of Windows. But the maintainer of *OpenAL Soft* also created a [DSOAL](https://github.com/kcat/dsoal), a translation layer from *DirectSound* to *OpenAL*. With that, the *DirectSound3D* calls from the game can be processed by *OpenAL Soft*. So the *EAX* effects can be used on a modern Windows.

##### What is this repository about?

The *EAX* presets from *Creative's EAX patches* can be used on modern computers, so why not stop there? The answer is the limitations.

The disadvantage of the solution presented above is its limited scope. The two main limitations are:

1. the *EAX patches* can only be used in v436. The patched *Galaxy.dll* does not work with the modern [OldUnreal/UnrealTournamentPatches](https://github.com/OldUnreal/UnrealTournamentPatches/)  

2. the *EAX patches* only work on Windows, so there's no way to enable those *EAX presets* on other OS

## EAX patches explained

The *EAX patches* for Unreal Tournament are still available today. Several mirrors exist, for example [ut-files](https://www.ut-files.com/index.php?dir=Patches/) or [OldUnreal](https://www.oldunreal.com/downloads/unrealtournament/other-patches/) or [medor](https://medor.no-ip.org/index.php?dir=Patches/EAX/&sort=size&sort_mode=a) or through the [Archive](https://web.archive.org/web/20050320092918/http://eax.creative.com/eagle/unreal/) mentioned above.

The patches are 3 separate files. The content of each file when installed:

**eaxutsys.exe**

> UnrealTournament\System\EaxMan.dll
> UnrealTournament\System\Galaxy.dll

**uteaxdm.exe**

> UnrealTournament\Eal\DM-Barricade.eal
> UnrealTournament\Eal\DM-Codex.eal
> UnrealTournament\Eal\DM-Conveyor.eal
> UnrealTournament\Eal\DM-Curse][.eal
> UnrealTournament\Eal\DM-Deck16][.eal
> UnrealTournament\Eal\DM-Fetid.eal
> UnrealTournament\Eal\DM-Fractal.eal
> UnrealTournament\Eal\DM-Gothic.eal
> UnrealTournament\Eal\DM-Grinder.eal
> UnrealTournament\Eal\DM-Hyperblast.eal
> UnrealTournament\Eal\DM-Kgalleon.eal
> UnrealTournament\Eal\DM-Liandri.eal
> UnrealTournament\Eal\DM-Morbias][.eal
> UnrealTournament\Eal\DM-Morpheus.eal
> UnrealTournament\Eal\DM-Oblivion.eal
> UnrealTournament\Eal\DM-Peak.eal
> UnrealTournament\Eal\DM-Phobos.eal
> UnrealTournament\Eal\DM-Pressure.eal
> UnrealTournament\Eal\DM-Pyramid.eal
> UnrealTournament\Eal\DM-Stalwart.eal
> UnrealTournament\Eal\DM-StalwartXL.eal
> UnrealTournament\Eal\DM-Tempest.eal
> UnrealTournament\Eal\DM-Turbine.eal
> UnrealTournament\Eal\DM-Zeto.eal

**uteaxctf.exe**

> UnrealTournament\Eal\CTF-Command.eal
> UnrealTournament\Eal\CTF-Coret.eal
> UnrealTournament\Eal\CTF-dreary.eal
> UnrealTournament\Eal\CTF-EternalCave.eal
> UnrealTournament\Eal\CTF-Face.eal
> UnrealTournament\Eal\CTF-Gauntlet.eal
> UnrealTournament\Eal\CTF-LavaGiant.eal
> UnrealTournament\Eal\CTF-Niven.eal
> UnrealTournament\Eal\CTF-November.eal

The `*.eal` files are **Environmental Audio Library** files. Those are installed to the newly created folder `UnrealTournament\Eal`. Each file contains the EAX presets for the corresponding map.

The `Galaxy.dll` installed to `UnrealTournament\System` replaces the existing file. This is the patched *Galaxy Audio* system that must be used for the EAX presets to work.

The `EaxMan.dll` is the **EAXManager**. It is used to parse the *.eal* files. 

## EAGLE™

The *.eal* files are created and edited in the *Environmental Audio Graphical Librarian Editor* (EAGLE™) by *Creative*. There is no longer an official download, but it has been [archived](https://web.archive.org/web/20060114235206/http://websound.ru/index_e.pl?libs_e).

To run EAGLE™, [DSOAL](https://github.com/kcat/dsoal) is needed on modern computers.

EAGLE™ can be used to confirm and understand the information read from *.eal* files. But its greatest potential lies in the **Import Plugin-In SDK**, which might be a way to understand the geometry data stored inside *.eal* files.

## EAL file

There is an official specification for Environmental Audio Library files. But it can be decoded: the container format of the *.eal* files is little endian RIFF - [Resource Interchange File Format](https://en.wikipedia.org/wiki/Resource_Interchange_File_Format).

*.eal* files consist of the main `RIFF chunk` and 4 `LIST Subchunks`.

###### Main chunk

The ID of this chunk is `eal ` (with a trailing space). Global data is stored here. The data fields are:

| ID     | data                                                                 | shown in EAGLE                                     |
| ------ | -------------------------------------------------------------------- | -------------------------------------------------- |
| 'majv' | 4 [*fixed*]                                                          |                                                    |
| 'minv' | 4 [*fixed*]                                                          |                                                    |
| 'exep' | <string> [*260 bytes*]                                               | Executable (full path) in *Execute Project* dialog |
| 'cmds' | <string> [*260 bytes*]                                               | Command (parameters) in *Execute Project dialog*   |
| 'gdfm' | <int32> MaxAttenuation; <float> LFRatio; <int32> AngleMaxAttenuation | Global Diffraction Model                           |
| 'lisa' | <struct> LISTENERATTRIBUTES [defined in *EaxMan.h*]                  | Listener Attributes                                |
| 'denv' | <struct> EAXLISTENERPROPERTIES [defined in *eax.h*]                  | Default Environment                                |
| 'dsrc' | <struct> SOURCEATTRIBUTES [defined in *EaxMan.h*]                    | Default Source                                     |
| 'dfil' | <string> [*260 bytes*]                                               | Wave file (full path) in Default Source Properties |
| 'dmat' | <struct> MATERIALATTRIBUTES [defined in *EaxMan.h*]                  | Default Obstacle                                   |

###### Environment Models Chunk

The ID of this chunk is `envp`. The data fields are:

| ID                            | data                                                 | shown in EAGLE                                                            |
| ----------------------------- | ---------------------------------------------------- | ------------------------------------------------------------------------- |
| 'num ' (incl. trailing space) | <int32>                                              | total number of *Environment Models*                                      |
| 'nams'                        | array of <string>; length of each string: *32 bytes* | entries are shown under *Environment Models*                              |
| 'lisp'                        | array of <struct> EAXLISTENERPROPERTIES              | dialog **Environment Controls** shows data for selected environment model |

###### Source Models Chunk

The ID of this chunk is `srcp`.

| ID                            | data                                                | shown in EAGLE                                                  |
| ----------------------------- | --------------------------------------------------- | --------------------------------------------------------------- |
| 'num ' (incl. trailing space) | <int32>                                             | total number of *Source Models*                                 |
| 'nams'                        | array of <string>; length of each string: *32 bytes | entries are shown under *Source Models*                         |
| 'fils'                        | array of <string> [*260 bytes*]                     | Wave file (full path) in selected source model properties       |
| 'srca'                        | array of <struct> SOURCEATTRIBUTES                  | dialog **Source Controls** shows data for selected source model |

###### Obstacle Models Chunk

The ID of this chunk is `matp`.

| ID                            | data                                                | shown in EAGLE                                                      |
| ----------------------------- | --------------------------------------------------- | ------------------------------------------------------------------- |
| 'num ' (incl. trailing space) | <int32>                                             | total number of *Obstacle Models*                                   |
| 'nams'                        | array of <string>; length of each string: *32 bytes | entries are shown under *Obstacle Models*                           |
| 'mata'                        | array of <struct> MATERIALATTRIBUTES                | dialog **Obstacle Controls** shows data for selected obstacle model |

###### Geometry Sets Chunk

The ID of this chunk is `gemp`.

| ID                            | data                                                | shown in EAGLE                                                 |
| ----------------------------- | --------------------------------------------------- | -------------------------------------------------------------- |
| 'num ' (incl. trailing space) | <int32>                                             | total number of *Geometry Sets*                                |
| 'nams'                        | array of <string>; length of each string: *32 bytes | entries are shown under *Geometry Sets*                        |
| 'fils'                        | <string> [*260 bytes*]                              | Mesh file (full path) in selected geometry set                 |
| 'gema'                        | **TODO** - not decoded yet                          | **fails** because *.eam* file needed to show geometry in EAGLE |

## 

# Goal 1: Understand EAXManager

###### EAXManager reference documentation

The **EAXManager SDK** provides a [reference documentation](/docs/eaxman.pdf).

###### EAXManager dependencies

The EAXManager header file from the **EAXManager SDK** is included in this repository: `inc/EaxMan.h`.

EaxMan.h depends on EAX 2.0. All necessary files are in the **EAX 2.0 SDK**.  The needed header is included in this repository: `inc/eax.h`.

### 

## Sample program ReadEalFiles

Included in this repository in `src/ReadEalFiles` is a little test program that uses all *EAXManager* API calls exposed by the header and documented in the reference documentation.

The source code is written in C++20 using Visual Studio C++ 2026. The main use of the code is to learn the behavior of `EaxMan.dll` so a proper implementation is possible.  The tool does not have any purpose other than that, so quality standards like proper error handling are only done in a very primitive and incomplete way.

There are example tools coming with the *EaxManager SDK* that are more complete and useful for a proper implementation.

Using both methods (`EaxMan.dll`  and EAGLE™) alternately to read the *.eal* files leads to a good understanding. It is also a test if the use of `EaxMan.dll` is properly implemented.

#### Usage

The tool has two modes:

###### Static analysis

`ReadEalFiles.exe <filename>.eal`

Analyze the *.eal* file with both `EaxMan.dll` and by a manual analysis and compare both results. See Goal 2.

All static data that can be read out with EAXManager API calls are retrieved. The corresponding API calls are:

`GetListenerAttributes`

`GetSourceAttributes`

`GetMaterialAttributes`

`GetEnvironmentAttributes`

The most relevant call is the last one, as it provides all parameters for setting up the Listener properties.

An example output looks like this:

```
> ReadEalFiles.exe CTF-Gauntlet.eal
analyzing file manually: CTF-Gauntlet.eal ... success
analyzing file with EaxMan.dll: CTF-Gauntlet.eal ... success
comparing data obtained from EaxMan.dll with values retrieved manuall from eal file ... success
datasets are identical
Statistics: Geometry Sets: 1 [ "Geometry1" ]
            Material Sets: 1 [ "Walls" ]
            Environment Sets: 9 [ "Tunnel" "Tunnel mouth" "Large chamber" "Small chamber" "Big room" "Underground" "Small room" "Tiny room" "In the lava aargh!" ]
            Sources Sets: 1 [ "stone04.wav" ]
            Sources placed: 0
```

###### Dynamic analysis

`ReadEalFiles.exe -d <filename>.eal`

Reads the *.eal* file with `EaxMan.dll` and then asks for user input. The coordinates `X`, `Y` and `Z` need to be entered by the user as floating point numbers in the range `-32767.0 ... +32767.0`.

The dynamic analysis uses the API call `GetListenerDynamicAttributes` which takes the entered coordinate and gives back the ID of the environment preset that applies to a Listener standing at that coordinate.

According to the documentation, *EAXManager* uses the same coordinate system as *DirectSound*, with positive values of X, Y and Z pointing forward, right, and up. It is unknown to the author if Unreal Tournament uses the same orientation, although the v469 patch programmers can easily check the source code.

`GetListenerDynamicAttributes` seems to be a major piece of what EAXManager does. It calculates internally which zone the coordinates lie in and then returns the ID of the environment that the audio renderer should use.

An example output looks like this:

```
> ReadEalFiles.exe -d CTF-Gauntlet.eal
analyzing file manually: CTF-Gauntlet.eal ... success
analyzing file with EaxMan.dll: CTF-Gauntlet.eal ... success
Enter coordinate x: -1100.35
Enter coordinate y: 0
Enter coordinate z: 0
query result: Environment ID: 0 - EAX Environment: EAX_ENVIRONMENT_HALLWAY
Enter coordinate x: 0.6
Enter coordinate y: 0.333
Enter coordinate z: -1.2
query result: Environment ID: 2 - EAX Environment: EAX_ENVIRONMENT_AUDITORIUM
Enter coordinate x: 1400
Enter coordinate y: -80
Enter coordinate z: 160
query result: Environment ID: 3 - EAX Environment: EAX_ENVIRONMENT_AUDITORIUM
```

## Conclusion

What `EaxMan.dll` actually does is read all EAX parameters from the *.eal* file and return them. It does not set up *DirectSound3D* / *EAX* calls but only provides the parameters that the program then needs to use.

This means that an implementation in Windows should be doable without major problems. `EaxMan.dll` can be used to gather all parameters needed. *OpenAL Soft* can wrap the *EAX* parameters into *EFX* calls. So the sound effects can be enjoyed on newer versions of *Unreal Tournament* as well.

## 

# Goal 2: Create a platform-independent replacement

`EaxMan.dll` is a library that can be run on Windows. But Unreal Tournament can be run under other operating systems. So there is a desire to make the EAX sound effects available under different OSs as well.

*OpenAL Soft* does work on all OS that Unreal Tournament v469 does run on. So wrapping the EAX parameters in EFX calls is possible on every OS, and the sound effects can be enjoyed on any system.

The *ReadEalFiles* tool provided in this repository does implement a way to read all the data from the *.eal* files. And it compares it to the results obtained with *EAXManager* to verify the results. The output of that tool when analyzing *CTF-Gauntlet.eal* looks like this:

```
> ReadEalFiles.exe CTF-Gauntlet.eal
analyzing file manually: CTF-Gauntlet.eal ... success
analyzing file with EaxMan.dll: CTF-Gauntlet.eal ... success
comparing data obtained from EaxMan.dll with values retrieved manuall from eal file ... success
datasets are identical
```

The data retrieved with both methods (using *EAXManager* API calls and the custom RIFF analysis done in *ReadEalFiles*) is compared. For all but one *.eal* file, the data retrieved with both methods are identical. The only exception is *DM-Grinder.eal*

###### DM-Grinder.eal

*ReadEalFiles* shows a mismatch on *DM-Grinder.eal*. This *.eal* file has 8 environments defined. The environment with ID 3 is named **Small verb**. The environment with ID 5 is named **Small Verb**. The only difference, therefore, is the lowercase/uppercase **V**. Both environments have different values as parameters, so they are treated as two separate presets when analyzed manually. Using *EaxMan.dll*, the API call *GetEnvironmentID(name)* is used, which gives the same ID 3 with both names. So it does not distinguish between upper- and lowercase letters and gives back the first ID it finds.

The correct behavior cannot be determined until the geometry data is decoded and we can see what exactly is stored there. The fact that the dynamic API call *GetSourceDynamicAttributes* does return an ID and not the environment name is a strong hint that the manual analysis is correct.

Therefore, this reported mismatch must be considered a bug in the *EaxMan.dll* API call *GetEnvironmentID(name)*.



#### Challenge still to be overcome

Still, there is one problem left: `EaxMan.dll` does do the 3D position calculations internally as described in the *ReadEalFiles dynamic analysis* section. The source code for EAXManager was not disclosed, and the calculations are only described in a rudimentary way. So there is yet decoding work to do.

#### Brute force approach

 `GetListenerDynamicAttributes` only gives back the environment ID and *ReadEalFiles* shows that there is a way to get the corresponding parameters without using *EaxMan.dll*. A straightforward approach is to read out all data and save it outside the *.eal* files. The most basic approach would be to loop over all coordinates and store the results of `GetListenerDynamicAttributes`.

The downside to this approach is the resulting file size. If we want to have a resolution of 1 cube unit, we would iterate all three coordinates in the range `-32767` to `32767`. So a total of `65536³ = (2^16)³ = 2^48` values needed to be stored. If the ID is stored in 1 Byte, that would result in a file size of `256 TB`.

Even if we only consider the playable space, this does not improve too much. For example, the player reachable space in *CTF-Face* is `X:-6238 to +10712`, `Y:-8539 to 8361`, `Z:-7358 to 10642`. This still results in a file size of `4.7 TB`.

There is still room for improvement. Compressing the data, storing more IDs per byte, and further reducing the volume are some ideas. Still, the brute force method does not seem promising except as a last resort.

So a refined approach is needed. An idea is to not store the ID for each coordinate but look at the coordinates where the ID changes and calculate back the shape of each environment zone from there.

This is a task for someone with experience with 3D coordinate calculations.

A better approach would, of course, be:

#### Decoding the geometry data

The geometry data is stored inside the *.eal* file and the calculation is done by the `EaxMan.dll`. So maybe there is a way to reverse engineer the actual calculation and then use that.

## Conclusion

Replacing `EaxMan.dll` with custom code for OS other than Windows is possible, but a feasible approach to obtain the 3D geometry data has still to be identified.



#### Sample program PlayEalFiles

*PlayEalFiles* is a simple proof of concept for that conclusion. Just like *ReadEalFiles*, *PlayEalFiles* is a quick-and-dirty tool with the sole purpose of testing the *EFX* calls on OS other than Windows.

It uses the algorithm from *ReadEalFiles* to read the environment presets from a *.eal* file. It then creates an *EFX* call with the parameters from the preset and uses *OpenAL Soft* to play the sounds with the effects.

The tool shows that those effects can be read and applied on amd64 Linux without any need for *EaxMan.dll* or the modifiex *Galaxy.dll*.

###### How to use

`./PlayEalFiles <sounds>.wav <level>.eal`

The *wave* file is played once without any effects applied as a reference.

It then loops over all environment effects described in the *.eal* file and plays the *wave* file once with each effect applied.

###### How to build

Compile with any C++20 compatible compiler.

Dependencies: libsndfile and libopenal



## Credits

The source code for *PlayEalFiles* contains pieces from [openal-soft](https://github.com/kcat/openal-soft), which is licensed under the LGPL. Those pieces remain under the LGPL.

Other pieces are heavily derived from the RIFF decoder and information [RIFF File Structure](https://johnloomis.org/cpe102/asgn/asgn1/riff.html)

Another helpful resource in making *PlayEalFiles* work was the [OpenAL short example](https://ffainelli.github.io/openal-example/)

And a final shoutout and thanks to *Creative* for those *EAX patches* and the folks behind *websound.ru* who saved those old SDKs. And to the contributors of [OldUnreal/UnrealTournamentPatches](https://github.com/OldUnreal/UnrealTournamentPatches) who keep this game alive.
