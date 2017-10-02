#!/bin/bash

EDK_PATH=~/edk2
BUILD_PATH=$EDK_PATH/Build/
MODE=(DEBUG RELEASE)
TOOL=GCC5
PLATFORM=X64
BOOT_HEALTH_PATH=ThalissonPkg/QEMU/harddisk
OUTPUT_DIR=$EDK_PATH/$BOOT_HEALTH_PATH


echo "Copying images:"
for mode in ${MODE[@]}
do  
    mkdir -p $OUTPUT_DIR/$mode

    FILES=$(find $BUILD_PATH/*/$mode\_$TOOL/$PLATFORM/ -maxdepth 1 -name "*.efi")

    FILES_ARRAY=$(echo $FILES | tr " " "\n")

    for file in $FILES_ARRAY;
    do
        cp --verbose $file $OUTPUT_DIR/$mode/ 
    done
done
