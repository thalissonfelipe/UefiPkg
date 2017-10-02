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
    EFI_HANDLE NtfsHandle = NULL;
    UINTN BufferSize = SIZE_OF_EFI_FILE_INFO + 512 * sizeof(CHAR16);
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem = NULL;
    EFI_FILE_INFO *FileInfo = NULL;
    FileInfo = AllocateZeroPool(BufferSize);
    EFI_FILE_PROTOCOL *RootDir = NULL;
    EFI_FILE_PROTOCOL *File = NULL;
    EFI_FILE_PROTOCOL *FileAux = NULL;

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

    Status = RootDir->Open(
            RootDir,
            &File,
            L"files.conf",
            EFI_FILE_MODE_READ,
            0);
    if (EFI_ERROR(Status)) {
        Print(L"Could not open file: %r\n", Status);
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

    CHAR8 BarraN = '\n';
    CHAR8 MiniBuffer[2];
    UINTN Size = 0;
    UINTN Count = 0;
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

		if(BarraN == MiniBuffer[0]) {
			Size = StrLen(Buffer) - 1;
			Buffer[Size] = '\0';
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
