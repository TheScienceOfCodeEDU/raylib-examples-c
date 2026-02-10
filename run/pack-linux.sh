#!/bin/bash

# Clear
read -p "[SCRIPT]: Do you want to clear the release folder? (y/n)" choice
if [[ "$choice" == "y" || "$choice" == "Y" ]]; then
    rm -r -p bin/rel-linux/
fi

# Compile
echo "[SCRIPT]: Compiling release binaries"
mkdir -p root/
mkdir -p bin/rel-linux/AppDir/lib
mkdir -p tmp/rel-linux/
gcc src/main.c -O3 -Wall -lraylib -lm -o bin/rel-linux/AppDir/run -std=c99 -pedantic

# Copy missing raylib shared libraries
cp /usr/local/lib64/libraylib.so.5.5.0 bin/rel-linux/AppDir/lib/
mv bin/rel-linux/AppDir/lib/libraylib.so.5.5.0 bin/rel-linux/AppDir/lib/libraylib.so.550
cp /lib64/libc.so.6 bin/rel-linux/AppDir/lib/
mv bin/rel-linux/AppDir/lib/libc.so.6 bin/rel-linux/AppDir/lib/libc.so

# AppImage recipe
if [ ! -f "tmp/rel-linux/AppImageBuilder.yml" ]; then
    cd bin/rel-linux
    read -p "[SCRIPT]: AppImage recipe not found. Do you want to download appimage-builder? (y/n)" choice

    if [[ "$choice" == "y" || "$choice" == "Y" ]]; then
        wget -O appimage-builder-x86_64.AppImage https://github.com/AppImageCrafters/appimage-builder/releases/download/v1.1.0/appimage-builder-1.1.0-x86_64.AppImage
        chmod +x appimage-builder-x86_64.AppImage
    fi

    echo "[SCRIPT]: Running appimage-builder. For the executable path be sure to write the following filename -> run"
    appimage-builder --generate

    cp AppImageBuilder.yml ../../tmp/rel-linux/
    cd ../..
fi
sh patch-appimage.sh
cp tmp/rel-linux/AppImageBuilder.yml bin/rel-linux/AppImageBuilder.yml

# Root folder
echo "[SCRIPT]: Copying root folder"
cp -r root/. bin/rel-linux/AppDir/

# Build AppImage
export LD_LIBRARY_PATH=/lib64:/usr/lib64:$LD_LIBRARY_PATH
read -p "[SCRIPT]: Press enter to Build the AppImage"
echo "[SCRIPT]: Building AppImage"
cd bin/rel-linux
appimage-builder --recipe AppImageBuilder.yml
cd ../..
