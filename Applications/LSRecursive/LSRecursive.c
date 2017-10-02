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
	CHAR16			  *Path
)
{
	EFI_STATUS Status;
	EFI_FILE_PROTOCOL *File = NULL;
	EFI_FILE_INFO *FileInfo = NULL;
	UINTN BufferSize = SIZE_OF_EFI_FILE_INFO + 512 * sizeof(CHAR16);
	FileInfo = (EFI_FILE_INFO*)AllocateZeroPool(BufferSize);
	//CHAR16 *Buffer = (CHAR16*)AllocateZeroPool(512 * sizeof(CHAR16));
	CHAR16 *BufferAux = (CHAR16*)AllocateZeroPool(512 * sizeof(CHAR16));

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
		//Buffer = (CHAR16*)AllocateZeroPool(512 * sizeof(CHAR16));
		Status = File->Read(File, &BufferSize, FileInfo);
		if (EFI_ERROR(Status)) {
			Print(L"Could not read file: %r\n", Status);
			goto close_file;
		}
		if (BufferSize == 0) {
			return Status;
		}
		if (FileInfo->Attribute == EFI_FILE_DIRECTORY) {
			if (StrCmp(FileInfo->FileName, L".") != 0 && StrCmp(FileInfo->FileName, L"..") != 0) {
				StrCat(BufferAux, FileInfo->FileName);
				Print(L"%s\n", BufferAux);
				//StrCat(Buffer, FileInfo->FileName);
				//StrCat(BufferAux, FileInfo->FileName);
				StrCat(BufferAux, L"/");
				LSRecursive(File, FileInfo->FileName);
			}
		}
		else {
			Print(L"%s\n", FileInfo->FileName);
		}
	}

close_file:
	File->Close(File);

close_root:
	RootDir->Close(RootDir);

//bottom:
	Print(L"Status: %r\n", Status);
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
    EFI_HANDLE NtfsHandle = NULL;
    UINTN BufferSize = SIZE_OF_EFI_FILE_INFO + 512 * sizeof(CHAR16);
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem = NULL;
    //EFI_FILE_INFO *FileInfo = NULL;
    //FileInfo = AllocateZeroPool(BufferSize);
    EFI_FILE_PROTOCOL *RootDir = NULL;
    //EFI_FILE_PROTOCOL *File = NULL;

    Status = gBS->LocateHandle(
        ByProtocol,
        &gEfiSimpleFileSystemProtocolGuid,
        NULL,
        &BufferSize,
        &NtfsHandle);
    if (Status == EFI_BUFFER_TOO_SMALL) {
        Print(L"Required handle buffer of size: %d\n", BufferSize);
    } else if (EFI_ERROR(Status)) {
        Print(L"Could not find any NTFS volume: %r\n", Status);
        goto bottom;
    }
    Print(L"Found a NTFS file system\n");

    Status = gBS->OpenProtocol(
        NtfsHandle,
        &gEfiSimpleFileSystemProtocolGuid,
        (VOID**) &FileSystem,
        ImageHandle,
        NULL,
        EFI_OPEN_PROTOCOL_GET_PROTOCOL);
    if (EFI_ERROR(Status)) {
        Print(L"Could not open NTFS sile system protocol: %r\n", Status);
        goto bottom;
    }

    Status = FileSystem->OpenVolume(
        FileSystem,
        &RootDir);
    if (EFI_ERROR(Status)) {
        Print(L"Could not open root dir: %r\n", Status);
        goto close_fs;
    }

    LSRecursive(RootDir, L".");


/*close_file:
    File->Close(File);

close_root:
    RootDir->Close(RootDir);*/

close_fs:
    gBS->CloseProtocol(
        NtfsHandle,
        &gEfiSimpleFileSystemProtocolGuid,
        ImageHandle,
        NULL);

bottom:
    return Status;
}
