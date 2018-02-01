#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/SimpleFileSystem.h>
#include <Guid/FileInfo.h>
#include <FileSystemLib/FileSystemLib.h>
#include <MiniShellLib/MiniShellLib.h>

EFI_STATUS
EFIAPI
LSRecursive(
	EFI_FILE_PROTOCOL *RootDir,
	CHAR16			  *Buffer,
	CHAR16			  *FileName,
	CHAR16  		  *Path,
	UINTN 			  *Flag
)
{
	EFI_STATUS Status;
	EFI_FILE_PROTOCOL *File = NULL;
	EFI_FILE_INFO *FileInfo = NULL;
	UINTN BufferSize = SIZE_OF_EFI_FILE_INFO + 512 * sizeof(CHAR16);
	FileInfo = AllocateZeroPool(BufferSize);
	CHAR16 *Dir = AllocateZeroPool(512 * sizeof(CHAR16));

	Status = RootDir->Open(
			RootDir,
			&File,
			Buffer,
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
			StrCpy(Path, L"");
			return Status;
		}
		if (FileInfo->Attribute == EFI_FILE_DIRECTORY) {
			if (StrCmp(FileInfo->FileName, L".") != 0 && StrCmp(FileInfo->FileName, L"..") != 0) {
				StrCpy(Dir, Path);
				StrCat(Path, FileInfo->FileName);
				StrCat(Path, L"/");
				LSRecursive(File, FileInfo->FileName, FileName, Path, Flag);
				StrCpy(Path, Dir);
			}
		}
		if (StrCmp(FileInfo->FileName, FileName) == 0) {
			Print(L"%s%s\n", Path, FileInfo->FileName);
			*Flag = 1;
		}
	}

	if (FileInfo != NULL) {
		FreePool(FileInfo);
	}

	if (Dir != NULL) {
		FreePool(Dir);
	}

close_file:
	CloseFile(File);

close_root:
	CloseFile(RootDir);

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
    UINTN Argc = 0;
    CHAR16 **Argv = NULL;
    CHAR16 *FileName = NULL;
    CHAR16 *Path = NULL;
    UINTN Flag = 0;

    Status = OpenRootDir(ImageHandle, &RootDir);
    if (EFI_ERROR(Status)) {
    	return Status;
    }

    Status = GetShellInput(&Argc, &Argv);
    if (EFI_ERROR(Status)) {
    	return Status;
    }

    FileName = AllocateZeroPool(512 * sizeof(CHAR16));
    Path = AllocateZeroPool(512 * sizeof(CHAR16));

    if (Argc == 1) {
    	StrCpy(FileName, L"Hello.txt");
    }
    else {
    	StrCpy(FileName, Argv[1]);
    }

    Print(L"Searching for %s.\n", FileName);
    LSRecursive(RootDir, L".", FileName, Path, &Flag);
    if (Flag != 1) {
    	Print(L"File not found.\n");
    }

    if (Path != NULL) {
    	FreePool(Path);
    }

    if (FileName != NULL) {
    	FreePool(FileName);
    }

    CloseProtocol();

	return Status;
}
