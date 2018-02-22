#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/SimpleFileSystem.h>
#include <Guid/FileInfo.h>
#include <FileSystemLib/FileSystemLib.h>

EFI_STATUS
EFIAPI
UefiMain (
    IN EFI_HANDLE        ImageHandle,
    IN EFI_SYSTEM_TABLE  *SystemTable
)
{
    EFI_STATUS Status = EFI_SUCCESS;
    EFI_HANDLE *Handle = NULL;
    UINTN NoHandles;
    UINTN Index;
    UINTN BufferSize;
    UINTN Count = 0;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem = NULL;
    EFI_FILE_PROTOCOL *RootDir = NULL;
    EFI_FILE_PROTOCOL *File = NULL;
    EFI_FILE_PROTOCOL *FileAux = NULL;
    EFI_FILE_INFO *FileInfo = NULL;
    CHAR8 MiniBuffer[2];
	CHAR16 *Buffer = NULL;
	CHAR16 *BufferAux = NULL;

    Status = gBS->LocateHandleBuffer(
        ByProtocol,
        &gEfiSimpleFileSystemProtocolGuid,
        NULL,
        &NoHandles,
        &Handle);
    if (EFI_ERROR(Status)) {
        Print(L"Could not find any handles: %r\n", Status);
        return Status;
    }
    Print(L"%d handle(s) found.\n", NoHandles);

    for (Index = 0; Index < NoHandles; Index++) {
		Status = gBS->OpenProtocol(
			Handle[Index],
			&gEfiSimpleFileSystemProtocolGuid,
			(VOID**) &FileSystem,
			ImageHandle,
			NULL,
			EFI_OPEN_PROTOCOL_GET_PROTOCOL);
		if (EFI_ERROR(Status)) {
			Print(L"Could not open system protocol: %r\n", Status);
			return Status;
		}

		Status = FileSystem->OpenVolume(FileSystem, &RootDir);
		if (EFI_ERROR(Status)) {
			Print(L"Could not open volume: %r\n", Status);
			goto close_fs;
		}

		Status = RootDir->Open(
				RootDir,
				&File,
				L".",
				EFI_FILE_MODE_READ,
				0);
		if (EFI_ERROR(Status)) {
			Print(L"Could not open root dir: %r\n", Status);
			goto FREE_RESOURCES;
		}

		Status = RootDir->Open(
				RootDir,
				&RootDir,
				L"root_deletion",
				EFI_FILE_MODE_READ,
				0);
		if (EFI_ERROR(Status)) {
			Print(L"Could not open root_deletion: %r\n", Status);
			continue;
		}

		Status = RootDir->Open(
				RootDir,
				&File,
				L"file_deletion.conf",
				EFI_FILE_MODE_READ,
				0);
		if (EFI_ERROR(Status)) {
			Print(L"Could not open file_deletion.conf: %r\n", Status);
			continue;
		}

		Index = NoHandles;

		BufferSize = SIZE_OF_EFI_FILE_INFO + 512 * sizeof(CHAR16);
		FileInfo = AllocateZeroPool(BufferSize);
		Status = File->GetInfo(
				File,
				&gEfiFileInfoGuid,
				&BufferSize,
				FileInfo);
		if (EFI_ERROR(Status)) {
			Print(L"Could not get info: %r\n", Status);
			goto FREE_RESOURCES;
		}

		Buffer = AllocateZeroPool(512 * sizeof(CHAR16));

		while (Count <= FileInfo->FileSize - 1) {
			BufferSize = 1;

			Status = File->Read(File, &BufferSize, MiniBuffer);
			if (EFI_ERROR(Status)) {
				Print(L"Could not read file: %r\n", Status);
				goto FREE_RESOURCES;
			}

			MiniBuffer[1] = '\0';
			BufferAux = AllocateZeroPool((AsciiStrLen(MiniBuffer) + 1) * sizeof(CHAR16));
			AsciiStrToUnicodeStr(MiniBuffer, BufferAux);
			StrCat(Buffer, BufferAux);

			if(MiniBuffer[0] == '\n') {
				Buffer[StrLen(Buffer) - 1] = '\0';
				Status = RootDir->Open(
						RootDir,
						&FileAux,
						Buffer,
						EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE,
						0);
				if (EFI_ERROR(Status)) {
					Print(L"Could not open file: %r\n", Status);
					goto FREE_RESOURCES;
				}

				Status = FileAux->Delete(FileAux);
				if (EFI_ERROR(Status)) {
					Print(L"Could not delete file: %r\n", Status);
					goto FREE_RESOURCES;
				}

				Print(L"%s deleted successfully.\n", Buffer);
				Buffer[0] = '\0';
			}
			Count++;
		}
    }

FREE_RESOURCES:

	if (FileInfo != NULL) {
		FreePool(FileInfo);
	}

	if (Buffer != NULL) {
		FreePool(Buffer);
	}

	if (BufferAux != NULL) {
		FreePool(BufferAux);
	}

	Status = CloseFileProtocol(File);

    Status = CloseFileProtocol(RootDir);

close_fs:
    Status = gBS->CloseProtocol(
        Handle,
        &gEfiSimpleFileSystemProtocolGuid,
        ImageHandle,
        NULL);

    return Status;
}
