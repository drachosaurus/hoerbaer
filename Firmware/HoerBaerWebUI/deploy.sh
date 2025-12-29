#!/bin/bash

rm -rf ../HoerBaerFirmware/data/webui/*
cp build/static/index-*.js ../HoerBaerFirmware/data/webui
cp build/static/index-*.css ../HoerBaerFirmware/data/webui
cp build/index.html ../HoerBaerFirmware/data/webui
