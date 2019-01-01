# What's this?

This is a project of VST.rex (Virtual Studio T.rex) plugin.

<blockquote class="twitter-tweet" data-lang="ja"><p lang="ja" dir="ltr">最近WordやVimにティラノサウルスが実装されたので、僕もDAW上にティラノサウルス呼べるようにした！！！！！ <a href="https://t.co/wKvhp4FS5y">pic.twitter.com/wKvhp4FS5y</a></p>&mdash; ほっと (@hotwatermorning) <a href="https://twitter.com/hotwatermorning/status/1079729217686786048?ref_src=twsrc%5Etfw">2018年12月31日</a></blockquote>

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
