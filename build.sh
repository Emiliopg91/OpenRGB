podman build -t openrgb-build .
podman run --rm --privileged --cpus=$(nproc) -v $(pwd):/input -v $(pwd)/out:/out openrgb-build
./scripts/build-udev-rules.sh .
/var/mnt/Datos/Desarrollo/Workspace/VSCode/RogControlCenter/assets/OpenRGB.AppImage --appimage-extract
cp ./build/openrgb squashfs-root/usr/bin/OpenRGB
cp ./60-openrgb.rules squashfs-root/usr/lib/udev/rules.d/60-openrgb.rules
/var/mnt/Datos/Desarrollo/Workspace/VSCode/AppImage-Creator-2/resources/appimagetool squashfs-root/ ./OpenRGB-Exp.AppImage
