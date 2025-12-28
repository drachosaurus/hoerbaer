#!/bin/bash

rm ../WeckerFirmware/data/webinterface/app.*
cp build/static/js/main.*.js ../WeckerFirmware/data/webinterface/app.js
cp build/static/css/main.*.css ../WeckerFirmware/data/webinterface/app.css