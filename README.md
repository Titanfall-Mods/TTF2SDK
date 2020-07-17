# TTF2SDK

TTF2SDK is a DLL, injected by Icepick into Titanfall 2, which modifies the behaviour of the game and allows mods to be loaded.

## Requirements

* Visual Studio 2019
* cmake

## Build Instructions

### Cloning the repository

TTF2SDK uses several submodules, so you'll need to pull them when you clone the repository using:

```
git clone --recurse-submodules git@github.com:Titanfall-Mods/TTF2SDK.git
```

If you forgot to use `--recurse-submodule` when you cloned the repository, you can pull the submodules using:

```
git submodule update --init --recursive
```

### Creating protobuf build files

The solution expects that protobuf project files are generated and placed in `thirdparty/protobuf_build`. To create them, run these commands from the root of the project:

```
mkdir thirdparty\protobuf_build
cd thirdparty\protobuf_build
cmake -G "Visual Studio 16 2019" -Dprotobuf_BUILD_TESTS=OFF ..\protobuf\cmake
```

### Building TTF2SDK

Open `TTF2SDK.sln` and build the project in either `Debug` or `Release` mode. `Debug` mode enables the `trace` log level which generates a lot of output and slows down the game, so try to use `Release` unless otherwise necessary.

Once built, replace `TTF2SDK.dll` in your Icepick folder with the one from `x64/Debug/TTF2SDK.dll` or `x64/Release/TTF2SDK.dll` and launch the game through the launcher.

## Third-party Libraries

TTF2SDK makes use of the following third-party libraries:

| Package Name | URL                                                                  |
|--------------|----------------------------------------------------------------------|
| breakpad     | https://chromium.googlesource.com/breakpad/breakpad/+/master/LICENSE |
| imgui        | https://github.com/ocornut/imgui/blob/master/LICENSE.txt             |
| MinHook      | https://github.com/TsudaKageyu/minhook/blob/master/LICENSE.txt       |
| protobuf     | https://github.com/google/protobuf/blob/master/LICENSE               |
| rapidjson    | https://github.com/Tencent/rapidjson/blob/master/license.txt         |
| spdlog       | https://github.com/gabime/spdlog/blob/v1.x/LICENSE                   |
