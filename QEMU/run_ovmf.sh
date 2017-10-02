#!/bin/bash
#
# Runs QEMU with the specified BIOS image
#

if [ $# -lt 1 ]; then
  echo "You must inform the Ovmf image name to run, usage:"
  echo "  $0 <Ovmf firmware name>"
  exit 1
fi

FIRMWARE="-drive file=$1,format=raw,if=pflash"
VIRTUAL_HDD="-drive id=nvme0,file=fat:wr:harddisk,format=raw,if=none"
VIRTUAL_HDD="$VIRTUAL_HDD -device nvme,drive=nvme0,serial=1234"
MACHINE="-m 1G -machine q35"
SERIAL_OUTPUT="-serial file:serial_output.txt"
USB_VOLUME="-usbdevice host:0781:5571"
#WIN_HDD="-drive file=win.raw,format=raw"

if [[ -f serial_output.txt ]]; then
    rm -f serial_output.txt
fi

source cp_images.sh

qemu-system-x86_64  $MACHINE $FIRMWARE $VIRTUAL_HDD  $SERIAL_OUTPUT \
    $USB_VOLUME "${@:2}"
