#ifndef __SIMPLE_DRIVER_H__
#define __SIMPLE_DRIVER_H__

#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>

#define SIMPLE_DRIVER_VERSION 0x000026EAu

extern EFI_GUID gSimpleDriverGuid;

#endif
