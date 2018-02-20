#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/SimpleFileSystem.h>
#include <FileSystemLib/FileSystemLib.h>
#include <Guid/FileInfo.h>

EFI_STATUS
EFIAPI
UefiMain (
    IN EFI_HANDLE        ImageHandle,
    IN EFI_SYSTEM_TABLE  *SystemTable
)
{
    EFI_STATUS Status = EFI_SUCCESS;
    UINTN BufferSize = SIZE_OF_EFI_FILE_INFO + 512 * sizeof(CHAR16);
    EFI_FILE_INFO *FileInfo = NULL;
    EFI_FILE_PROTOCOL *RootDir = NULL;
    EFI_FILE_PROTOCOL *File = NULL;

    Status = OpenRootDir(ImageHandle, &RootDir);
    if (EFI_ERROR(Status)) {
    	Print(L"Could not open root dir: %r\n", Status);
    	return Status;
    }

    Status = RootDir->Open(
            RootDir,
            &File,
            L".",
            EFI_FILE_MODE_READ,
            0);
    if (EFI_ERROR(Status)) {
        Print(L"Could not open file: %r\n", Status);
        goto FREE_RESOURCES;
    }

    FileInfo = AllocateZeroPool(BufferSize);
    while (TRUE) {
		Status = File->Read(File, &BufferSize, FileInfo);
		if (EFI_ERROR(Status)) {
			Print(L"Could not read file: %r\n", Status);
			goto FREE_RESOURCES;
		}
		if (BufferSize == 0) {
			goto FREE_RESOURCES;
		}
		Print(L"%s\n", FileInfo->FileName);
		BufferSize = SIZE_OF_EFI_FILE_INFO + 512 * sizeof(CHAR16);
    }

FREE_RESOURCES:

	if (FileInfo != NULL) {
    	FreePool(FileInfo);
    }

    Status = CloseFileProtocol(File);

    Status = CloseFileProtocol(RootDir);

    Status = CloseProtocol();

    return Status;
}
