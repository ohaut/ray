#!/bin/sh
set -e
set -x
VERSION=$(python -c "print open('ray/version.h', 'rb').read().split('\"')[1]")
APP_VERSION=$(python -c "print open('webapp/version', 'rb').readlines()[0]")
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
LAST_APP_VERSION=$(tail -n 1 ../master/firmware.csv | cut -f3 -d,)

echo "FW version.h (CSV)  : $LAST_VERSION"
echo "FW version.h        : $VERSION"
echo "WEBAPP version.js (CSV): $LAST_APP_VERSION"
echo "WEBAPP version.js      : $APP_VERSION"

if [[ $LAST_VERSION=>$VERSION ]] && [[ $LAST_APP_VERSION=>$APP_VERSION ]];  then
    echo "This version already exist in firmware.csv, please update " \
         "your version.h and/or version.js"
    git checkout master
    exit 1
fi

git checkout origin/gh-pages -B gh-pages

if [ $LAST_VERSION == $VERSION ] && \
    ! cmp .pioenvs/esp12e/firmware.bin ../master/firmware-$LAST_VERSION.bin; then
   echo "You should have bumped your version.h because your firmware changed"
   git checkout master
   exit 1
fi

gzip -d -c -k data/app.html.gz > /tmp/app.html.1
gzip -d -c -k ../master/app.html-$LAST_APP_VERSION.gz > /tmp/app.html.2

if [ $LAST_APP_VERSION == $APP_VERSION ] && \
    ! cmp /tmp/app.html.1 /tmp/app.html.2 ; then
   echo "You should have bumped your version.js because your app.html.gz changed"
   git checkout master
   exit 1
fi

rm /tmp/app.html.[12]



cp -f .pioenvs/esp12e/firmware.bin ../master/firmware-$VERSION.bin
cp -f data/app.html.gz ../master/app.html-$APP_VERSION.gz
rm -f ../master/firmware.bin
rm -f ../master/app.html.gz
ln -s ../master/firmware-$VERSION.bin ../master/firmware.bin
ln -s ../master/app.html-$APP_VERSION.gz ../master/app.html.gz
echo $VERSION,firmware-$VERSION.bin,$APP_VERSION,app.html-$APP_VERSION.gz,$COMMIT_ID \
    >> ../master/firmware.csv

git add ../master/firmware*
git add ../master/app*

git commit -a -m "Updated to FW/APP $VERSION/$APP_VERSION, from source commit $COMMIT_ID"
git push
git checkout master

