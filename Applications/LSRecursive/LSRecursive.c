#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/SimpleFileSystem.h>
#include <FileSystemLib/FileSystemLib.h>
#include <Guid/FileInfo.h>

EFI_STATUS
LSRecursive(
	EFI_FILE_PROTOCOL *RootDir,
	CHAR16			  *Buffer,
	CHAR16			  *Path
)
{
	EFI_STATUS Status;
	EFI_FILE_PROTOCOL *File = NULL;
	EFI_FILE_INFO *FileInfo = NULL;
	UINTN BufferSize = SIZE_OF_EFI_FILE_INFO + 512 * sizeof(CHAR16);
	FileInfo = AllocateZeroPool(BufferSize);
	CHAR16 *Dir = AllocateZeroPool(512 * sizeof(CHAR16));
	Dir[0] = '\0';

	Status = RootDir->Open(
			RootDir,
			&File,
			Buffer,
			EFI_FILE_MODE_READ,
			0);
	if (EFI_ERROR(Status)) {
		Print(L"Could not open dir: %r\n", Status);
		goto FREE_RESOURCES;
	}

	while (TRUE) {
		BufferSize = SIZE_OF_EFI_FILE_INFO + 512 * sizeof(CHAR16);
		Status = File->Read(File, &BufferSize, FileInfo);
		if (EFI_ERROR(Status)) {
			Print(L"Could not read file: %r\n", Status);
			goto FREE_RESOURCES;
		}
		if (BufferSize == 0) {
			StrCpy(Path, L"");
			return Status;
		}
		if (FileInfo->Attribute == EFI_FILE_DIRECTORY) {
			if (StrCmp(FileInfo->FileName, L".") != 0 && StrCmp(FileInfo->FileName, L"..") != 0) {
				StrCpy(Dir, Path);
				StrCat(Path, FileInfo->FileName);
				Print(L"%s\n", Path);
				StrCat(Path, L"/");
				LSRecursive(File, FileInfo->FileName, Path);
				StrCpy(Path, Dir);
			}
		}
		else {
			Print(L"%s%s\n", Path, FileInfo->FileName);
		}

	}

FREE_RESOURCES:

	if (FileInfo != NULL) {
		FreePool(FileInfo);
	}

	if (Dir != NULL) {
		FreePool(Dir);
	}

	CloseFileProtocol(File);

	CloseFileProtocol(RootDir);

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
    EFI_FILE_PROTOCOL *RootDir = NULL;
    CHAR16 *Path = NULL;

    Status = OpenRootDir(ImageHandle, &RootDir);
    if (EFI_ERROR(Status)) {
    	return Status;
    }

    Path = AllocateZeroPool(512 * sizeof(CHAR16));
    LSRecursive(RootDir, L".", Path);

    if (Path != NULL) {
    	FreePool(Path);
    }

    Status = CloseFileProtocol(RootDir);

    Status = CloseProtocol();

    return Status;
}
