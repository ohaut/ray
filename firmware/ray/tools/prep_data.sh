#!/bin/sh

cd data_raw
rm -rf ../data
mkdir ../data
cp -rfpv * ../data
cd ../data
rm -rf .DS*
rm VERSION_H
ln -s ../ray/version.h VERSION_H
cd css
./all_css.sh
cp all.css /tmp
rm *
cp /tmp/all.css .
cd ..
echo compressing:
for file in $(find ./ -type f ! -type l  -name  "*"); do
    echo  - $file
    gzip $file
done

