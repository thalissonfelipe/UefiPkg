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
LSRecursive(
	EFI_FILE_PROTOCOL *RootDir,
	CHAR16			  *Path,
	CHAR16			  *Arquivo,
	BOOLEAN 		  *CheckFile
)
{
	EFI_STATUS Status;
	EFI_FILE_PROTOCOL *File = NULL;
	EFI_FILE_INFO *FileInfo = NULL;
	UINTN BufferSize = SIZE_OF_EFI_FILE_INFO + 512 * sizeof(CHAR16);
	FileInfo = AllocateZeroPool(BufferSize);

	Status = RootDir->Open(
			RootDir,
			&File,
			Path,
			EFI_FILE_MODE_READ,
			0);
	if (EFI_ERROR(Status)) {
		Print(L"Could not open file/dir: %r\n", Status);
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
		if (StrCmp(Arquivo, FileInfo->FileName) == 0) {
			*CheckFile = TRUE;
		}
		if (FileInfo->Attribute == EFI_FILE_DIRECTORY) {
			if (StrCmp(FileInfo->FileName, L".") != 0 && StrCmp(FileInfo->FileName, L"..") != 0) {
				if (*CheckFile == TRUE) {
					return Status;
				}
				LSRecursive(File, FileInfo->FileName, Arquivo, CheckFile);
			}
		}
	}

FREE_RESOURCES:

	if (FileInfo != NULL){
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
    EFI_HANDLE *NtfsHandle = NULL;
    UINTN NoHandles;
    UINTN Index;
    UINTN Argc;
    CHAR16 **Argv = NULL;
    CHAR16 *FileName = NULL;
    BOOLEAN CheckFile = FALSE;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem = NULL;
    EFI_FILE_PROTOCOL *RootDir = NULL;

    Status = gBS->LocateHandleBuffer(
        ByProtocol,
        &gEfiSimpleFileSystemProtocolGuid,
        NULL,
        &NoHandles,
        &NtfsHandle);
    if (EFI_ERROR(Status)) {
        Print(L"Could not find any handles: %r\n", Status);
        goto FREE_RESOURCES;
    }

    GetShellInput(&Argc, &Argv);

    if (Argc == 1 || Argc > 2) {
    	FileName = L"Hello.txt";
    }
    else if (Argc == 2) {
    	FileName = AllocateZeroPool(StrSize(Argv[1]) * sizeof(CHAR16));
    	StrCpy(FileName, Argv[1]);
    }

    Print(L"%d handle(s) found.\n", NoHandles);
    Print(L"Looking for: %s\n", FileName);

    for (Index = 0; Index < NoHandles; Index++) {
		Status = gBS->OpenProtocol(
			NtfsHandle[Index],
			&gEfiSimpleFileSystemProtocolGuid,
			(VOID**) &FileSystem,
			ImageHandle,
			NULL,
			EFI_OPEN_PROTOCOL_GET_PROTOCOL);
		if (EFI_ERROR(Status)) {
			Print(L"Could not open system protocol: %r\n", Status);
			goto FREE_RESOURCES;
		}

		Status = FileSystem->OpenVolume(
			FileSystem,
			&RootDir);
		if (EFI_ERROR(Status)) {
			Print(L"Could not open root dir: %r\n", Status);
			goto FREE_RESOURCES;
		}

		LSRecursive(RootDir, L".", FileName, &CheckFile);
    }

    if (CheckFile) {
    	Print(L"The file exists.\n");
    }
    else {
    	Print(L"File not found.\n");
    }

FREE_RESOURCES:

	if (FileName != NULL && Argc == 2) {
		FreePool(FileName);
	}

	Status = CloseFileProtocol(RootDir);

    Status = gBS->CloseProtocol(
        NtfsHandle,
        &gEfiSimpleFileSystemProtocolGuid,
        ImageHandle,
        NULL);

    return Status;
}
