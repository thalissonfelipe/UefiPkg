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
		Print(L"Required handle buffer of size: %d\n", BufferSize);
		return Status;
	}
	else if (EFI_ERROR(Status)) {
		Print(L"Could not find any NTFS/FAT volume: %r\n", Status);
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
		Print(L"Could not open NTFS/FAT system protocol: %r\n", Status);
		return Status;
	}

	Status = FileSystem->OpenVolume(FileSystem, RootDir);
	if (EFI_ERROR(Status)) {
		Print(L"Could not open root dir: %r\n", Status);
		return Status;
	}

	return Status;
}

VOID
EFIAPI
CloseProtocol ()
{
	if (AgentHandle != NULL && FsHandle != NULL) {
		gBS->CloseProtocol(
				FsHandle,
				&gEfiSimpleFileSystemProtocolGuid,
				AgentHandle,
				NULL);
	}
}

VOID
EFIAPI
CloseFile (
	IN EFI_FILE_PROTOCOL *File
)
{
	if (File != NULL) {
		File->Close(File);
	}
}
