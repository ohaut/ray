#!/bin/sh
rm -f all.css 2>/dev/null
cat *.css >>all.css
gzip -k all.css
