#!/bin/sh
set -x

#requisites: npm install -g inliner

APP_VERSION=$(head -n 1 webapp/version)

# cleanup the output data directory
rm -rf data
mkdir data

rm -rf data_raw
mkdir data_raw
echo rendering app, version $APP_VERSION
ohaut-render-html webapp/ray.html data_raw \
        --set app_version=$APP_VERSION \
        --project_name Ray
#        --template webapp_template \

cp -rfp webapp_template/* data_raw

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
