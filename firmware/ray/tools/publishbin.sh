#!/bin/sh
set -e
VERSION=$(python -c "print open('ray/version.h', 'rb').read().split('\"')[1]")
VERSION_JS=$(python -c "print open('data_raw/js/version.js', 'rb').read().split('\"')[1]")
COMMIT_ID=$(git rev-parse --short HEAD)


if ! git diff --quiet; then
    echo "You have unstagged changes."
    exit 1
fi

git push

platformio run
./tools/prep_data.sh

git fetch origin
git checkout origin/gh-pages -B gh-pages

LAST_VERSION=$(tail -n 1 ../master/firmware.csv | cut -f1 -d,)
LAST_VERSION_JS=$(tail -n 1 ../master/firmware.csv | cut -f3 -d,)

echo "FW version.h (CSV)  : $LAST_VERSION"
echo "FW version.h        : $VERSION"
echo "APP version.js (CSV): $LAST_VERSION_JS"
echo "APP version.js      : $VERSION_JS"

if [[ $LAST_VERSION=>$VERSION ]] && [[ $LAST_VERSION_JS=>$VERSION_JS ]];  then
    echo "This version already exist in firmware.csv, please update " \
         "your version.h and/or version.js"
    git checkout master
    exit 1
fi

git checkout origin/gh-pages -B gh-pages

if [ $LAST_VERSION == $VERSION ] && \
    ! cmp .pioenvs/esp12e/firmware.bin ../master/firmware-$VERSION.bin; then
   echo "You should have bumped your version.h because your firmware changed"
   git checkout master
   exit 1
fi

gzip -d -k data/app.html.gz > /tmp/app.html.1
gzip -d -k ../master/app.html-$VERSION_JS.gz > /tmp/app.html.2

if [ $LAST_VERSION_JS == $VERSION_JS ] && \
    ! cmp /tmp/app.html.1 /tmp/app.html.2 ; then
   echo "You should have bumped your version.js because your app.html.gz changed"
   git checkout master
   exit 1
fi

rm /tmp/app.html.[12]



cp -vf .pioenvs/esp12e/firmware.bin ../master/firmware-$VERSION.bin
cp -vf data/app.html.gz ../master/app.html-$VERSION_JS.gz
rm -f ../master/firmware.bin
rm -f ../master/app.html.gz
ln -s ../master/firmware-$VERSION.bin ../master/firmware.bin
ln -s ../master/app.html-$VERSION_JS.gz ../master/app.html.gz
echo $VERSION,firmware-$VERSION.bin,$VERSION_JS,app.html-$VERSION_JS.gz,$COMMIT_ID \
    >> ../master/firmware.csv

git add ../master/firmware*
git add ../master/app*

git commit -a -m "Updated to FW/APP $VERSION/$VERSION_JS, from source commit $COMMIT_ID"
git push
git checkout master

