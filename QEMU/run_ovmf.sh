

#!/bin/bash

# Initial values. These can be overwritten by user's command line args
QEMU_EXEC='qemu-system-x86_64'
QEMU_ARGS=""
FIRMWARE="OVMF.fd"
VFAT_DIR="harddisk"
SERIAL_OUTPUT="serial_output.txt"
USB_DEVICES=""
MEMORY="1G"
WINDOWS_IMAGES=""

# Get user's custom and additional options
while getopts "f:u:m:s:w:d" opt; do
    case $opt in
        f)
            FIRMWARE="$OPTARG"
            ;;
        u)
            USB_DEVICES="$USB_DEVICES -usbdevice host:$OPTARG"
            ;;
        m)
            MEMORY="$OPTARG"
            ;;
        s)
            SERIAL_OUTPUT="$OPTARG"
            ;;
        w)
            WINDOWS_IMAGES="-drive file=$OPTARG,format=raw"
            ;;
        d)
            VFAT_DIR="$OPTARG"
            ;;
    esac
done

source cp_images.sh

# firmware image file
QEMU_ARGS="$QEMU_ARGS -drive file=$FIRMWARE,format=raw,if=pflash"

# virtual fat file system
QEMU_ARGS="$QEMU_ARGS -drive id=nvme0,file=fat:wr:$VFAT_DIR,format=raw,if=none"
QEMU_ARGS="$QEMU_ARGS -device nvme,drive=nvme0,serial=1234"

# machine config
QEMU_ARGS="$QEMU_ARGS -m $MEMORY -machine q35"

# serial output file
QEMU_ARGS="$QEMU_ARGS -serial file:$SERIAL_OUTPUT"

# real usb devices passthrough
QEMU_ARGS="$QEMU_ARGS $USB_DEVICES"

# windows image files
QEMU_ARGS="$QEMU_ARGS $WINDOWS_IMAGES"

##
## At last! let's run QEMU
##
$QEMU_EXEC $QEMU_ARGS

