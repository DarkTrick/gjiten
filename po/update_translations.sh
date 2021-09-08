#!/bin/sh

# Use this file, if you changed strings in code and 
#   you need to update the po-files
# This script must be run from within the po directory

#intltool needs potfile to compare stuff
intltool-update --pot

# update all po files you can find
for f in $(find *.po); do
    filename_noExt="${f%.*}"
    echo "${filename_noExt}: "
    intltool-update -d ${filename_noExt}
done

# We could remove the potfile here
rm ./gjiten.pot