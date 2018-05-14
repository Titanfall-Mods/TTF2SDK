# TTF2SDK

Make sure you checkout submodules

## Creating protobuf build files

```
mkdir thirdparty\protobuf_build
cd thirdparty\protobuf_build
cmake -G "Visual Studio 15 2017 Win64" -Dprotobuf_BUILD_TESTS=OFF ../protobuf/cmake
```
