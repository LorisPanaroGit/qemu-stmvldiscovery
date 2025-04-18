#!/bin/bash

BUILD_DIR="build_arm"

display_menu() {
    echo "====== Script Menu ====="
    echo "1) Display this menu"
    echo "2) Run configure script"
    echo "3) Build ARM models"
    echo "4) Install executables"
    echo "5) Uninstall executables"
    echo "========================"
}

run_configure_script() {
    if [ ! -d ${BUILD_DIR} ]; then 
        echo "Creating build_arm directory..."
        mkdir ${BUILD_DIR}    
    fi

    echo "Entering ${BUILD_DIR} directory..."
    cd ${BUILD_DIR}
    ../configure \
    --prefix=/usr \
    --target-list=arm-softmmu \
    --disable-libnfs \
    --disable-libvduse \
    --disable-fuse \
    --disable-fuse-lseek \
    --enable-trace-backends=simple
    cd ..
}

build_arm_code() {
    if [ ! -d ${BUILD_DIR} ] || [ -z "$(ls -A ${BUILD_DIR} 2>/dev/null)" ]; then
        echo "${BUILD_DIR} not present or empty. Run configure script first..."
    else 
        echo "Building filesystem..."
        cd ${BUILD_DIR}
        make -j14
        cd ..
    fi
}

install_executables() {
    if [ ! -d ${BUILD_DIR}] || [ -z "$(ls -A ${BUILD_DIR} 2>/dev/null)"]; then
        echo "${BUILD_DIR} not present or empty. Run configure script first..."
    else 
        echo "Installing filesystem..."
        cd ${BUILD_DIR}
        sudo make install
        cd ..
    fi   
}

uninstall_executables() {
    if [ ! -d ${BUILD_DIR}] || [ -z "$(ls -A ${BUILD_DIR} 2>/dev/null)"]; then
        echo "${BUILD_DIR} not present or empty. Run configure script first..."
    else 
        echo "Uninstalling filesystem..."
        cd ${BUILD_DIR}
        sudo make uninstall
        cd ..
    fi
}

case "$1" in
    1)
        display_menu
        ;;
    2) 
        run_configure_script
        ;;
    3) 
        build_arm_code
        ;;
    4) 
        install_executables
        ;;
    5) 
        uninstall_executables
        ;;
    *)
        echo "Invalid option"
        display_menu
        ;;
esac

