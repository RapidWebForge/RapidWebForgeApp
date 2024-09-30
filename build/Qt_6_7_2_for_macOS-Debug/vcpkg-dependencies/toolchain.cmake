
        set(CMAKE_C_COMPILER "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang")
        set(CMAKE_CXX_COMPILER "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang++")
        set(CMAKE_OSX_SYSROOT /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX15.0.sdk)

        set(VCPKG_TARGET_TRIPLET x64-osx)
        include("/Users/lecav/Programs/Tesis/RapidWebForgeApp/vcpkg/scripts/buildsystems/vcpkg.cmake")
      