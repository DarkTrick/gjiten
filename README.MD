# GJiten

GJiten is a dictionary

Gjiten is a Japanese dictionary program for `edict` files.
It also contains a kanji dictionary.


## Installation

### Ubuntu
    % # Add `universe` repository
    % sudo add-apt-repository "deb http://archive.ubuntu.com/ubuntu $(lsb_release -sc) universe"
    % sudo apt update
    % # install all dependencies
    % sudo apt install build-essential libglib2.0-dev intltool-debian libtool libtool-bin intltool libgtk-3-dev gnome-common

    % ./autogen.sh
    % make
    % sudo make install

## How to build

On Ubuntu:

    % # Add `universe` repository
    % sudo add-apt-repository "deb http://archive.ubuntu.com/ubuntu $(lsb_release -sc) universe"
    % sudo apt update
    % # install all dependencies
    % sudo apt install build-essential libglib2.0-dev intltool-debian libtool libtool-bin intltool libgtk-3-dev gnome-common


    % ./autogen.sh
    % make

## How to Develop
Use the `--enable-debug` while developing

    % # Add `universe` repository
    % sudo add-apt-repository "deb http://archive.ubuntu.com/ubuntu $(lsb_release -sc) universe"
    % sudo apt update
    % # install all dependencies
    % sudo apt install build-essential libglib2.0-dev intltool-debian libtool libtool-bin intltool libgtk-3-dev gnome-common

    % # build it with
    % ./autogen.sh --enable-debug
    % make

    % # run it
    % cd ./src
    % ./gjiten

With `--enable-debug` you have to start gjiten from within `src`. Otherwise static paths will be invalid and you'll encounter various problems



# Reporting Bugs

Report bugs directly here on Github.