#!/bin/bash

rm -rf ../HoerBaerFirmware/data/webui/*.js
rm -rf ../HoerBaerFirmware/data/webui/*.css
cp build/static/index-*.js ../HoerBaerFirmware/data/webui/app.js
cp build/static/index-*.css ../HoerBaerFirmware/data/webui/app.css