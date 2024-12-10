podman build -t openrgb-build .
mkdir out
podman run --privileged --cpus=$(nproc) -v $(pwd)/out:/output openrgb-build
cp out/openrgb squashfs-root/usr/bin/OpenRGB
/var/mnt/Datos/Desarrollo/Workspace/VSCode/RogControlCenter/resources/OpenRGB.AppImage --appimage-extract
/var/mnt/Datos/Desarrollo/Workspace/VSCode/AppImage-Creator-2/resources/appimagetool squashfs-root/ OpenRGB-Exp.AppImage
rm -R out
