## Build on Ubuntu

### Dependecies

```bash
sudo apt-get install -y cmake ninja-build pkg-config dpkg-dev dh-autoreconf libltdl-dev liblua5.2-dev libglib2.0-dev libpango1.0-dev librsvg2-dev libsoup2.4-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libfontconfig1-dev libgtk-3-dev gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav libgssdp-1.2-dev libjsoncpp-dev nclua -qq
```

### Build

Build with `cmake` (main way):

```bash
mkdir _build && cd _build
cmake ..
ninja
cmake --install . --prefix /usr
```

### Build installer

```bash
  cd ginga _build
  cmake .. -G -DCMAKE_BUILD_TYPE=Release
  ninja 
  cpack
```

To deploy installer on github, run:

```bash
  hub release edit v1.0 -m "" -a ginga-itv_1.0_amd64.deb
```

### decrypted autotools

The Build with autotools is decrypted in favor cmake. You can try it with:

```bash
./bootstrap
./configure --prefix=/usr/
make
```
