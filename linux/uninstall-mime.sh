#!/bin/bash

# Remove old
xdg-mime uninstall 'mime/goldsrc-bsp30.xml'
xdg-mime uninstall 'mime/goldsrc-fgd.xml'
xdg-mime uninstall 'mime/goldsrc-map.xml'
xdg-mime uninstall 'mime/goldsrc-rmf.xml'
xdg-mime uninstall 'mime/goldsrc-wad2.xml'
xdg-mime uninstall 'mime/goldsrc-wad3.xml'

echo "Don't forget to run this as booth your user and root"
