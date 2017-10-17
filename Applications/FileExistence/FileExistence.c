#include  <Uefi.h>
#include  <Library/UefiLib.h>
#include  <Library/UefiApplicationEntryPoint.h>
#include  <Library/UefiBootServicesTableLib.h>
#include  <Library/MemoryAllocationLib.h>
#include  <Protocol/SimpleFileSystem.h>
#include  <Guid/FileInfo.h>

EFI_STATUS
LSRecursive(
	EFI_FILE_PROTOCOL *RootDir,
	CHAR16			  *Path,
	CHAR16			  *Arquivo,
	UINTN 			  *Check
)
{
	EFI_STATUS Status;
	EFI_FILE_PROTOCOL *File = NULL;
	EFI_FILE_INFO *FileInfo = NULL;
	UINTN BufferSize = SIZE_OF_EFI_FILE_INFO + 512 * sizeof(CHAR16);
	FileInfo = (EFI_FILE_INFO*)AllocateZeroPool(BufferSize);

	Status = RootDir->Open(
			RootDir,
			&File,
			Path,
			EFI_FILE_MODE_READ,
			0);
	if (EFI_ERROR(Status)) {
		Print(L"Could not open dir: %r\n", Status);
		goto close_root;
	}

	while (TRUE) {
		BufferSize = SIZE_OF_EFI_FILE_INFO + 512 * sizeof(CHAR16);
		Status = File->Read(File, &BufferSize, FileInfo);
		if (EFI_ERROR(Status)) {
			Print(L"Could not read file: %r\n", Status);
			goto close_file;
		}
		if (BufferSize == 0) {
			return Status;
		}
		if (StrCmp(Arquivo, FileInfo->FileName) == 0) {
			*Check = 1;
		}
		if (FileInfo->Attribute == EFI_FILE_DIRECTORY) {
			if (StrCmp(FileInfo->FileName, L".") != 0 && StrCmp(FileInfo->FileName, L"..") != 0) {
				if (*Check == 1) return Status;
				LSRecursive(File, FileInfo->FileName, Arquivo, Check);
			}
		}
	}

	FreePool(FileInfo);

close_file:
	File->Close(File);

close_root:
	RootDir->Close(RootDir);

	return Status;
}

EFI_STATUS
EFIAPI
UefiMain (
    IN EFI_HANDLE        ImageHandle,
    IN EFI_SYSTEM_TABLE  *SystemTable
)
{
    EFI_STATUS Status = EFI_SUCCESS;
    EFI_HANDLE *NtfsHandle = NULL;
    UINTN NoHandles;
    UINTN i;
    UINTN *Check;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem = NULL;
    EFI_FILE_PROTOCOL *RootDir = NULL;
    CHAR16 *Arquivo = L"Felipe.txt";

    Status = gBS->LocateHandleBuffer(
        ByProtocol,
        &gEfiSimpleFileSystemProtocolGuid,
        NULL,
        &NoHandles,
        &NtfsHandle);
    if (EFI_ERROR(Status)) {
        Print(L"Could not find any handles: %r\n", Status);
        goto bottom;
    }

    Print(L"%d handle(s) encontrado(s)\n", NoHandles);
    Print(L"Procurando por %s\n", Arquivo);

    for (i = 0; i < NoHandles; i++) {
		Status = gBS->OpenProtocol(
			NtfsHandle[i],
			&gEfiSimpleFileSystemProtocolGuid,
			(VOID**) &FileSystem,
			ImageHandle,
			NULL,
			EFI_OPEN_PROTOCOL_GET_PROTOCOL);
		if (EFI_ERROR(Status)) {
			Print(L"Could not open system protocol: %r\n", Status);
			goto bottom;
		}

		Status = FileSystem->OpenVolume(
			FileSystem,
			&RootDir);
		if (EFI_ERROR(Status)) {
			Print(L"Could not open root dir: %r\n", Status);
			goto close_fs;
		}

		LSRecursive(RootDir, L".", Arquivo, Check);

		if (*Check == 1) {
			Print(L"O arquivo existe.\n");
			break;
		}
    }

    if (*Check != 1)
    	Print(L"O arquivo não existe.\n");



close_fs:
    gBS->CloseProtocol(
        NtfsHandle,
        &gEfiSimpleFileSystemProtocolGuid,
        ImageHandle,
        NULL);

bottom:
    return Status;
}
