#!/bin/bash

#add check if directory exists

if [ ! -d "build_arm" ]; then 
    echo "Creating build_arm directory..."
    mkdir build_arm    
fi

echo "Entering build_arm directory..."
cd build_arm
../configure \
--prefix=/usr \
--target-list=arm-softmmu \
--disable-libnfs \
--disable-libvduse \
--disable-fuse \
--disable-fuse-lseek \
--enable-trace-backends=simple

echo "Building filesystem..."
make -j14
cd ..
