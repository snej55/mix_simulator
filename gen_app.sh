#!/usr/bin/fish
rm -rf build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/ -j$(nproc)
strip build/main

set APPDIR "release/DuckBowling.AppDir"
rm -rf $APPDIR
mkdir -p $APPDIR/usr/bin
# copy assets
cp build/main $APPDIR/usr/bin/
cp -r build/data $APPDIR/usr/bin/
cp -r build/shaders $APPDIR/usr/bin/
cp release/icon.png $APPDIR/
cp release/DuckBowling.desktop $APPDIR/
echo '#!/bin/bash
HERE="$(dirname "$(readlink -f "${0}")")"
export LD_LIBRARY_PATH="$HERE/usr/lib:$LD_LIBRARY_PATH"
exec "$HERE/usr/bin/main" "$@"' > $APPDIR/AppRun
chmod +x $APPDIR/AppRun

if not test -f release/appimagetool
    curl -L "https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-x86_64.AppImage" > "release/appimagetool"
    chmod +x ./release/appimagetool
end

set -x ARCH x86_64
cd release; ./appimagetool DuckBowling.AppDir