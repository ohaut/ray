#!/bin/sh
./tools/prep_data.sh
curl --progress-bar -F "image=@data/app.html.gz" $1/update-app/ 
