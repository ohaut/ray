#!/bin/sh
set -x

#requisites: npm install -g inliner

APP_VERSION=$(python -c "print open('webapp/version', 'rb').readlines()[0]")

# cleanup the output data directory
rm -rf data
mkdir data

rm -rf data_raw
mkdir data_raw
echo rendering app, version $APP_VERSION
ohaut-render-html webapp/ray.html data_raw \
        --set app_version=$APP_VERSION \
        --project_name Ray

cp -rf webapp/js/ray.js data_raw/js/

# go to the source directory
cd data_raw

# make an inlined version of index.html to avoid multiple requests
# at load
inliner -m index.html >../data/app.html
gzip -9 ../data/app.html

# link version and remove usless data in the directory
cd ../data
rm -rf .DS*
