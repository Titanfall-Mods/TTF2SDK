# TTF2SDK

TTF2SDK is a DLL, injected by Icepick into Titanfall 2, which modifies the behaviour of the game and allows mods to be loaded.

## Requirements

* Visual Studio 2017 - latest version
* cmake

## Build Instructions

### Cloning the Repository

TTF2SDK uses several submodules, so you'll need to pull them when you clone the repository using:

```
git clone --recurse-submodules git@github.com:Titanfall-Mods/TTF2SDK.git
```

If you forgot to use `--recurse-submodule` when you cloned the repository, you can pull the submodules using:

```
git submodule update --init --recursive
```

### Creating protobuf Build Files

The solution expects that protobuf project files are generated and placed in `thirdparty/protobuf_build`. To create them, run these commands from the root of the project:

```
mkdir thirdparty\protobuf_build
cd thirdparty\protobuf_build
cmake -G "Visual Studio 15 2017 Win64" -Dprotobuf_BUILD_TESTS=OFF ../protobuf/cmake
```

### Building TTF2SDK

Open `TTF2SDK.sln` and build the project in either `Debug` or `Release` mode. `Debug` mode enables the `trace` log level which generates a lot of output and slows down the game, so try to use `Release` unless otherwise necessary.

Once built, replace `TTF2SDK.dll` in your Icepick folder with the one from `x64/Debug/TTF2SDK.dll` or `x64/Release/TTF2SDK.dll` and launch the game through the launcher.
