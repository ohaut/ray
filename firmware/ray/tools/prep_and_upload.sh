#!/bin/sh
echo preparing app.html.gz
./tools/prep_data.sh
echo uploading app.html.gz
curl -v -F "image=@data/app.html.gz" $1/update-app/

echo ""
echo ""
