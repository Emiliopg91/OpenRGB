mkdir -p build
cd build
qmake ../OpenRGB.pro
make -j32

cd ..
chmod +x scripts/build-udev-rules.sh
./scripts/build-udev-rules.sh ${PWD}