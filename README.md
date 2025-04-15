# FAAC Library

Convert audio PCM into AAC.

## Prerequisite
    sudo apt update

    sudo apt install -y autoconf automake libtool pkg-config

## How to build ?
    mkdir installl

    ./bootstrap

	./configure --host=$(YOUR_TARGET_HOST) --prefix=$(pwd)/install CC=$(YOUR_CC_TOOLCHAIN) CXX=$(YOUR_CPP_TOOLCHAIN)

    make
	
    make install

<b>NOTE</b>: 
- All libraries will be put in <i>install</i> directory.
- If no need to cross compile, just run <i>./configure --prefix=$(pwd)/install</i>

## How to use ?
    cd examples

    make

    ./pcm2aac --help