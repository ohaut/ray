#!/bin/sh
#requisites: npm install -g inliner

# cleanup the output data directory
rm -rf data
mkdir data

# go to the source directory
cd data_raw

# make an inlined version of index.html to avoid multiple requests
# at load
inliner -m index.html >../data/index.html
gzip -9 ../data/index.html

# link version and remove usless data in the directory
cd ../data
rm -rf .DS*
ln -s ../ray/version.h VERSION_H
