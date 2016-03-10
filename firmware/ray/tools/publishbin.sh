#!/bin/sh
set -e
VERSION=$(python -c "print open('data_raw/VERSION_H', 'rb').read().split('\"')[1]")
COMMIT_ID=$(git rev-parse --short HEAD)


if ! git diff --quiet; then
    echo "You have unstagged changes."
    exit 1
fi

git push

git fetch origin
git checkout origin/gh-pages -B gh-pages

if grep $VERSION ../master/firmware.csv; then
    echo "This version already exist in firmware.csv, please update " \
         "your version.h"
    git checkout master
    exit 1
fi

git checkout master

platformio run
mv .pioenvs/esp12e/firmware.bin .pioenvs/esp12e/firmware-$VERSION.bin
./tools/prep_data.sh
~/.platformio/packages/tool-mkspiffs/mkspiffs -c data -p 256 -b 8192 \
                        -s 1028096 .pioenvs/esp12e/spiffs-$VERSION.bin

git checkout origin/gh-pages -B gh-pages


cp -v .pioenvs/esp12e/firmware-$VERSION.bin ../master
cp -v .pioenvs/esp12e/spiffs-$VERSION.bin ../master
rm -f ../master/firmware.bin
rm -f ../master/spiffs.bin
ln -s ../master/firmware-$VERSION.bin ../master/firmware.bin
ln -s ../master/spiffs-$VERSION.bin ../master/spiffs.bin
echo $VERSION,firmware-$VERSION.bin,spiffs-$VERSION.bin,$COMMIT_ID \
    >> ../master/firmware.csv

git add ../master/firmware*
git add ../master/spiffs*

git commit -a -m "Updated to version $VERSION, from source commit $COMMIT_ID"
git push
git checkout master

