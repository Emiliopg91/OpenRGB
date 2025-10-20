podman build -t openrgb-build .
rm -R ./build
mkdir build
podman run --rm --privileged --cpus=$(nproc) -v $(pwd):/input openrgb-build
if [ -f ./build/openrgb ]; then
    ./OpenRGB.AppImage --appimage-extract
    cp ./build/openrgb squashfs-root/usr/bin/OpenRGB
    ./appimagetool squashfs-root/ ./OpenRGB.AppImage
fi