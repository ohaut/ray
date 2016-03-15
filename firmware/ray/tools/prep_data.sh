#!/bin/sh
#requisites: npm install -g inliner

# cleanup the output data directory
rm -rf data
mkdir data

# go to the source directory
cd data_raw

# make an inlined version of index.html to avoid multiple requests
# at load
inliner -m index.html >../data/app.html
gzip -9 ../data/app.html

# link version and remove usless data in the directory
cd ../data
rm -rf .DS*
