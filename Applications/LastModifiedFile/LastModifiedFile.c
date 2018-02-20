#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Protocol/SimpleFileSystem.h>
#include <Guid/FileInfo.h>
#include <FileSystemLib/FileSystemLib.h>
#include <time.h>
#include <sys/time.h>

EFI_FILE_INFO*
EFIAPI
CompareTwoTimes(
	EFI_FILE_INFO *FileInfo,
	EFI_FILE_INFO *FileInfoAux
)
{
	INT32 TimeFileInfo = Efi2Time(&FileInfo->ModificationTime);
	INT32 TimeFileInfoAux = Efi2Time(&FileInfoAux->ModificationTime);

	if (TimeFileInfo >= TimeFileInfoAux) {
		return FileInfo;
	}
	else {
		return FileInfoAux;
	}
}

EFI_STATUS
EFIAPI
LSRecursive (
	EFI_FILE_PROTOCOL *RootDir,
	CHAR16 			  *Path,
	EFI_FILE_INFO	  *FileInfoAux,
	CHAR16			  *FileName
)
{
	EFI_STATUS Status;
	EFI_FILE_PROTOCOL *File = NULL;
	UINTN BufferSize = SIZE_OF_EFI_FILE_INFO + 512 * sizeof(CHAR16);
	EFI_FILE_INFO *FileInfo = AllocateZeroPool(BufferSize);
	EFI_FILE_INFO *Result = NULL;

	Status = RootDir->Open(
			RootDir,
			&File,
			Path,
			EFI_FILE_MODE_READ,
			0);
	if (EFI_ERROR(Status)) {
		Print(L"Could not open file: %r\n", Status);
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
			return Status;
		}

		if (FileInfo->Attribute == EFI_FILE_DIRECTORY) {
			if (StrCmp(FileInfo->FileName, L".") != 0 && StrCmp(FileInfo->FileName, L"..") != 0) {
				LSRecursive(File, FileInfo->FileName, FileInfoAux, FileName);
			}
		}
		else {
			Result = CompareTwoTimes(FileInfo, FileInfoAux);
			CopyMem(FileInfoAux, Result, SIZE_OF_EFI_FILE_INFO + (StrLen(Result->FileName) * 2) + 1);
			StrCpy(FileName, FileInfoAux->FileName);
		}
	}

FREE_RESOURCES:

	if (FileInfo != NULL) {
		FreePool(FileInfo);
	}

	Status = CloseFileProtocol(File);

	Status = CloseFileProtocol(RootDir);

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
    UINTN BufferSize;
    EFI_FILE_PROTOCOL *RootDir = NULL;
    EFI_FILE_INFO *FileInfo = NULL;
    CHAR16 *FileName = NULL;

    Status = OpenRootDir(ImageHandle, &RootDir);
    if (EFI_ERROR(Status)) {
    	Print(L"Could not open root dir: %r\n", Status);
    	return Status;
    }

    BufferSize = SIZE_OF_EFI_FILE_INFO + 512 * sizeof(CHAR16);
    FileInfo = AllocateZeroPool(BufferSize);
    FileInfo->ModificationTime.Year = 0;
    FileInfo->ModificationTime.Month = 0;
    FileInfo->ModificationTime.Day = 0;
    FileInfo->ModificationTime.Hour = 0;
    FileInfo->ModificationTime.Minute = 0;
    FileInfo->ModificationTime.Second = 0;

    FileName = AllocateZeroPool(512 * sizeof(CHAR16));
    FileName[0] = '\0';
    LSRecursive(RootDir, L".", FileInfo, FileName);

    Print(L"Last modified file: %s\n", FileName);
    Print(L"Date: %d/%d/%d \n", FileInfo->ModificationTime.Day,
    							FileInfo->ModificationTime.Month, FileInfo->ModificationTime.Year);
    Print(L"Time: %d:%d:%d \n", FileInfo->ModificationTime.Hour,
    							FileInfo->ModificationTime.Minute, FileInfo->ModificationTime.Second);

    if (FileInfo != NULL) {
    	FreePool(FileInfo);
    }

    if (FileName != NULL) {
    	FreePool(FileName);
    }

    Status = CloseFileProtocol(RootDir);

    Status = CloseProtocol();

    return Status;
}

