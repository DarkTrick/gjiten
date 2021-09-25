# GJiten

GJiten is a dictionary

Gjiten is a Japanese dictionary program for `edict` files.
It also contains a kanji dictionary.

<img src="./screenshots/word_dictionary.png" width="70%">
<img src="./screenshots/kanji_dictionary.png" width="60%">



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

If you file bugs please try to include the following information:
- gjiten version ( `gjiten --version` )
- distribution and other info about your OS
  (under Ubuntu: `lsb_release --all`)
- gtk release ( `pkg-config --modversion gtk+-3.0` )
- output of `gdb gjiten` and/or `strace gjiten`
- and any other info you might think is important.


# User Settings
User settings are stored in `<system config path>/gjiten/gjiten.config` .
Under Ubuntu this amounts to `~/.config/gjiten/gjiten.conf`.