# What's this?

This is a project of VST.rex (Virtual Studio T.rex) plugin.

# How to build?

(Sorry, a detailed version of build instruction is under construction.)

1. `git submodule update --init --recuesive`
1. Build JUCE library checked-out in ext/JUCE.
    * This plugin uses the forked version of JUCE to work around mouse-click transparency bug on macOS.
1. Build assimp library checked-out in ext/assimp with cmake like this:
    ```sh
    mkdir /path/to/vstrex/ext/assimp/build_debug && cd $_
    cmake -DCMAKE_OSX_DEPLOYMENT_TARGET=10.10 -DCMAKE_INSTALL_PREFIX=/path/to/vstrex/ext/assimp/build_debug/install -DBUILD_SHARED_LIBS=OFF -DASSIMP_BUILD_SAMPLES=OFF -DBUILD_DOCS=OFF -DCMAKE_BUILD_TYPE=Debug -DDEBUG_POSTFIX= ..
    make -j && make install
    ```
1. Build FreeGLUT library checked-out in ext/JUCE (windows only).
1. Open the VSTRex.jucer file into the Projucer of ext/JUCE.

# LICENSE

This project is licensed under GPLv3 with the exception for files in ./model directory.
