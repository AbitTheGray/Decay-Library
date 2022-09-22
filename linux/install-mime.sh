#!/bin/bash

# Definitions
xdg-mime install --novendor 'mime/goldsrc-bsp30.xml'
xdg-mime install --novendor 'mime/goldsrc-fgd.xml'
xdg-mime install --novendor 'mime/goldsrc-map.xml'
xdg-mime install --novendor 'mime/goldsrc-wad2.xml'
xdg-mime install --novendor 'mime/goldsrc-wad3.xml'

# Icons
xdg-icon-resource install --context mimetypes --size 64 'mime/icon/goldsrc-bsp30_64x.png' 'application-goldsrc-bsp30'
#xdg-icon-resource install --context mimetypes --size 64 'mime/icon/goldsrc-fgd_64x.png' 'application-goldsrc-fgd'
#xdg-icon-resource install --context mimetypes --size 64 'mime/icon/goldsrc-map_64x.png' 'application-goldsrc-map'
xdg-icon-resource install --context mimetypes --size 64 'mime/icon/goldsrc-wad2_64x.png' 'application-goldsrc-wad2'
xdg-icon-resource install --context mimetypes --size 64 'mime/icon/goldsrc-wad3_64x.png' 'application-goldsrc-wad3'
