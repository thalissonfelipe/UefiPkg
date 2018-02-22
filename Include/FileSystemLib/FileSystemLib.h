#ifndef _FILE_SYSTEM_LIB_
#define _FILE_SYSTEM_LIB_

#include <Uefi.h>
#include <Protocol/SimpleFileSystem.h>

EFI_STATUS
EFIAPI
OpenRootDir (
	IN  EFI_HANDLE 	  	  ImageHandle,
	OUT EFI_FILE_PROTOCOL **RootDir
);

EFI_STATUS
EFIAPI
CloseFileProtocol (
	IN EFI_FILE_PROTOCOL *FileProtocol
);

EFI_STATUS EFIAPI CloseProtocol ();

#endif
