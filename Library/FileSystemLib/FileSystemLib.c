#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/SimpleFileSystem.h>

static EFI_HANDLE FsHandle = NULL;
static EFI_HANDLE AgentHandle = NULL;

EFI_STATUS
EFIAPI
OpenRootDir (
	IN  EFI_HANDLE 	      ImageHandle,
	OUT EFI_FILE_PROTOCOL **RootDir
)
{
	EFI_STATUS Status = EFI_SUCCESS;
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem = NULL;
	UINTN BufferSize = sizeof(EFI_HANDLE);

	AgentHandle = ImageHandle;

	Status = gBS->LocateHandle(
		ByProtocol,
		&gEfiSimpleFileSystemProtocolGuid,
		NULL,
		&BufferSize,
		&FsHandle);
	if (Status == EFI_BUFFER_TOO_SMALL) {
		Status = gBS->LocateHandle(
			ByProtocol,
			&gEfiSimpleFileSystemProtocolGuid,
			NULL,
			&BufferSize,
			&FsHandle);
		if (EFI_ERROR(Status)) {
			return Status;
		}
	}
	else if (EFI_ERROR(Status)) {
		return Status;
	}

	Status = gBS->OpenProtocol(
		FsHandle,
		&gEfiSimpleFileSystemProtocolGuid,
		(VOID**)&FileSystem,
		AgentHandle,
		NULL,
		EFI_OPEN_PROTOCOL_GET_PROTOCOL);
	if (EFI_ERROR(Status)) {
		return Status;
	}

	Status = FileSystem->OpenVolume(FileSystem, RootDir);

	return Status;
}

EFI_STATUS
EFIAPI
CloseProtocol ()
{
    EFI_STATUS Status = EFI_SUCCESS;

	if (FsHandle != NULL) {
		Status = gBS->CloseProtocol(
				FsHandle,
				&gEfiSimpleFileSystemProtocolGuid,
				AgentHandle,
				NULL);
	}

	return Status;
}

EFI_STATUS
EFIAPI
CloseFileProtocol (
	IN EFI_FILE_PROTOCOL *FileProtocol
)
{
    EFI_STATUS Status = EFI_SUCCESS;

	if (FileProtocol != NULL) {
		Status = FileProtocol->Close(FileProtocol);
	}

	return Status;
}
