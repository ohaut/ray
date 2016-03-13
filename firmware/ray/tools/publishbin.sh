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

git checkout origin/gh-pages -B gh-pages


cp -v .pioenvs/esp12e/firmware-$VERSION.bin ../master
cp -v data/app.html.gz ../master/app.html-$VERSION.gz
rm -f ../master/firmware.bin
rm -f ../master/app.html.gz
ln -s ../master/firmware-$VERSION.bin ../master/firmware.bin
ln -s ../master/app.html-$VERSION.gz ../master/app.html.gz
echo $VERSION,firmware-$VERSION.bin,app.html-$VERSION.gz,$COMMIT_ID \
    >> ../master/firmware.csv

git add ../master/firmware*
git add ../master/app*

git commit -a -m "Updated to version $VERSION, from source commit $COMMIT_ID"
git push
git checkout master

