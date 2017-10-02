#include  <Uefi.h>
#include  <Library/UefiLib.h>
#include  <Library/UefiApplicationEntryPoint.h>
#include  <Library/UefiBootServicesTableLib.h>
#include  <Library/MemoryAllocationLib.h>
#include  <Protocol/SimpleFileSystem.h>
#include  <Guid/FileInfo.h>

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
    UINTN BufferSize = SIZE_OF_EFI_FILE_INFO + 512 * sizeof(CHAR16);
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem = NULL;
    EFI_FILE_INFO *FileInfo = NULL;
    FileInfo = AllocateZeroPool(BufferSize);
    EFI_FILE_PROTOCOL *RootDir = NULL;
    EFI_FILE_PROTOCOL *File = NULL;
    EFI_FILE_PROTOCOL *FileAux = NULL;

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

		Status = RootDir->Open(
				RootDir,
				&File,
				L".",
				EFI_FILE_MODE_READ,
				0);
		if (EFI_ERROR(Status)) {
			Print(L"Could not open file: %r\n", Status);
			goto close_root;
		}

		UINTN Count = 0;
		while (TRUE) {
			BufferSize = SIZE_OF_EFI_FILE_INFO + 512 * sizeof(CHAR16);
			Status = File->Read(File, &BufferSize, FileInfo);
			if (EFI_ERROR(Status)) {
				Print(L"Could not read file: %r\n", Status);
				goto close_file;
			}
			if (BufferSize == 0) {
				goto end_for;
			}
			if (StrCmp(FileInfo->FileName, L"root_deletion") == 0)
				Count++;
			if (StrCmp(FileInfo->FileName, L"file_deletion.conf") == 0)
				Count++;
			if (Count == 2) {
				Print(L"Tudo certo!\n");
				i = NoHandles;
				break;
			}
		}

		Status = RootDir->Open(
				RootDir,
				&File,
				L"file_deletion.conf",
				EFI_FILE_MODE_READ,
				0);
		if (EFI_ERROR(Status)) {
			Print(L"Could not open file_deletion.conf: %r\n", Status);
			goto close_root;
		}

		Status = RootDir->Open(
				RootDir,
				&RootDir,
				L"root_deletion",
				EFI_FILE_MODE_READ,
				0);
		if (EFI_ERROR(Status)) {
			Print(L"Could not open root_deletion: %r\n", Status);
			goto close_root;
		}

		BufferSize = SIZE_OF_EFI_FILE_INFO + 512 * sizeof(CHAR16);
		Status = File->GetInfo(
				File,
				&gEfiFileInfoGuid,
				&BufferSize,
				FileInfo);
		if (EFI_ERROR(Status)) {
			Print(L"Could not get info: %r\n", Status);
			goto close_file;
		}
		Print(L"FileSize: %d\n", FileInfo->FileSize);

		CHAR8 MiniBuffer[2];
		UINTN StringSize = 0;
		Count = 0;
		CHAR16 *Buffer = AllocateZeroPool(512 * sizeof(CHAR16));
		CHAR16 *BufferAux;

		while (Count <= FileInfo->FileSize - 1) {
			BufferSize = 1;
			Status = File->Read(File, &BufferSize, MiniBuffer);
			if (EFI_ERROR(Status)) {
				Print(L"Could not read file: %r\n", Status);
				goto close_file;
			}

			MiniBuffer[1] = '\0';
			BufferAux = AllocateZeroPool((AsciiStrLen(MiniBuffer) + 1) * sizeof(CHAR16));
			AsciiStrToUnicodeStr(MiniBuffer, BufferAux);
			StrCat(Buffer, BufferAux);

			if(MiniBuffer[0] == '\n') {
				StringSize = StrLen(Buffer) - 1;
				Buffer[StringSize] = '\0';
				Print(L"%Buffer: %s\n", Buffer);
				Status = RootDir->Open(
						RootDir,
						&FileAux,
						Buffer,
						EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE,
						0);
				if (EFI_ERROR(Status)) {
					Print(L"Could not open file: %r\n", Status);
					goto close_root;
				}
				Status = FileAux->Delete(FileAux);
				if (EFI_ERROR(Status)) {
					Print(L"Could not delete file: %r\n", Status);
					goto close_file;
				}
				Print(L"Arquivo (%s) deletado com sucesso!\n", Buffer);
				Buffer = AllocateZeroPool(512 * sizeof(CHAR16));
			}
			Count++;
		}

		end_for:
			continue;

    }

close_file:
    File->Close(File);

close_root:
    RootDir->Close(RootDir);

close_fs:
    gBS->CloseProtocol(
        NtfsHandle,
        &gEfiSimpleFileSystemProtocolGuid,
        ImageHandle,
        NULL);

bottom:
    return Status;
}
