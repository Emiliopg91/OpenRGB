podman build -t openrgb-build .
podman run --rm --privileged --cpus=$(nproc) -v $(pwd):/input openrgb-build
/var/mnt/Datos/Desarrollo/Workspace/VSCode/RogControlCenter/resources/OpenRGB.AppImage --appimage-extract
cp ./build/openrgb squashfs-root/usr/bin/OpenRGB
/var/mnt/Datos/Desarrollo/Workspace/VSCode/AppImage-Creator-2/resources/appimagetool squashfs-root/ ./OpenRGB-Exp.AppImage
