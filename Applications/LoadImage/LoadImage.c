#include  <Uefi.h>
#include  <Library/UefiLib.h>
#include  <Library/UefiApplicationEntryPoint.h>
#include  <Library/UefiBootServicesTableLib.h>
#include  <Library/MemoryAllocationLib.h>
#include  <Protocol/SimpleFileSystem.h>
#include  <Protocol/LoadedImage.h>
#include  <Library/DevicePathLib.h>
#include  <Guid/FileInfo.h>

EFI_STATUS
LSRecursive(
	EFI_FILE_PROTOCOL *RootDir,
	CHAR16			  *Buffer,
	CHAR16			  *FileName,
	CHAR16  		  *Path,
	EFI_STATUS 		   Flag
)
{
	EFI_STATUS Status;
	EFI_FILE_PROTOCOL *File = NULL;
	EFI_FILE_INFO *FileInfo = NULL;
	UINTN BufferSize = SIZE_OF_EFI_FILE_INFO + 512 * sizeof(CHAR16);
	FileInfo = (EFI_FILE_INFO*)AllocateZeroPool(BufferSize);
	CHAR16 *Dir = (CHAR16*)AllocateZeroPool(512 * sizeof(CHAR16));

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
				StrCat(Path, L"\\");
				LSRecursive(File, FileInfo->FileName, FileName, Path, Flag);
				if (Flag == EFI_SUCCESS) return Status;
				StrCpy(Path, Dir);
			}
		}
		if (StrCmp(FileInfo->FileName, FileName) == 0) {
			StrCat(Path, FileInfo->FileName);
			Flag = EFI_SUCCESS;
			break;
		}
	}

	FreePool(FileInfo);
	FreePool(Dir);

close_file:
	File->Close(File);

close_root:
	RootDir->Close(RootDir);

	return Status;
}

EFI_STATUS
ReadInput (
	CHAR16	*FileName
)
{
	EFI_STATUS Status;
	UINTN EventIndex;
	UINTN i = 0;
	EFI_INPUT_KEY Key;
	CHAR16 *Buffer = (CHAR16*)AllocateZeroPool(512 * sizeof(CHAR16));

	Print(L"Tecle ESC para finalizar a entrada.\n");

	while (Key.UnicodeChar != SCAN_F3) {
		Status = gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &EventIndex);
		if (EFI_ERROR(Status)) {
			Print(L"Error: %r\n", Status);
			return Status;
		}

		Status = gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);
		if (EFI_ERROR(Status)) {
			Print(L"Error: %r\n", Status);
			return Status;
		}

		Buffer[i] = Key.UnicodeChar;
		Print(L"%c", Key.UnicodeChar);
		i++;
	}

	Buffer[i-1] = '\0';
	StrCpy(FileName, Buffer);
	FreePool(Buffer);

	return Status;
}

EFI_STATUS
EFIAPI
UefiMain(
	IN EFI_HANDLE			ImageHandle,
	IN EFI_SYSTEM_TABLE		*SystemTable
)
{
	EFI_STATUS Status;
	EFI_HANDLE Handle = NULL;
	EFI_HANDLE ImageApplication = NULL;
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem = NULL;
	EFI_FILE_PROTOCOL *RootDir = NULL;
	EFI_DEVICE_PATH_PROTOCOL *DevicePath = NULL;
	CHAR16 *FileName = (CHAR16*)AllocateZeroPool(512 * sizeof(CHAR16));
	CHAR16 *FilePath = (CHAR16*)AllocateZeroPool(512 * sizeof(CHAR16));
	FilePath[0] = '\0';
	FileName[0] = '\0';
	UINTN BufferSize = sizeof(EFI_HANDLE);

	Status = gBS->LocateHandle(
			ByProtocol,
			&gEfiSimpleFileSystemProtocolGuid,
			NULL,
			&BufferSize,
			&Handle);
	if (EFI_ERROR(Status)) {
		Print(L"Could not find handles: %r\n", Status);
		return Status;
	}
	Print(L"Handle encontrado.\n");

	Status = gBS->OpenProtocol(
			Handle,
			&gEfiSimpleFileSystemProtocolGuid,
			(VOID**) &FileSystem,
			ImageHandle,
			NULL,
			EFI_OPEN_PROTOCOL_GET_PROTOCOL);
	if (EFI_ERROR(Status)) {
		Print(L"Could not open protocol: %r\n", Status);
		return Status;
	}

	Status = FileSystem->OpenVolume(
			FileSystem,
			&RootDir);
	if (EFI_ERROR(Status)) {
		Print(L"Could not open volume: %r\n", Status);
		goto close_fs;
	}

	ReadInput(FileName);
	Print(L"Carregar imagem: %s\n", FileName);

	EFI_STATUS Flag;
	LSRecursive(RootDir, L".", FileName, FilePath, Flag);
	if (StrCmp(FilePath, L"") == 0) {
		Print(L"Arquivo não encontrado.\n");
		goto close_fs;
	}
	Print(L"Path: %s\n", FilePath);

	DevicePath = FileDevicePath(Handle, FilePath);

	Status = gBS->LoadImage(
			FALSE,
			ImageHandle,
			DevicePath,
			(VOID*) NULL,
			0,
			&ImageApplication);
	if (EFI_ERROR(Status)) {
		Print(L"Could not load image: %r\n", Status);
		goto close_fs;
	}
	Print(L"Imagem carregada.\n");

	Print(L"Startando a imagem...\n");
	Status = gBS->StartImage(ImageApplication, (UINTN*) NULL, (CHAR16**) NULL);
	if (EFI_ERROR(Status)) {
		Print(L"Could not start image: %r\n", Status);
		goto close_fs;
	}

	FreePool(FileName);
	FreePool(FilePath);


close_fs:
	gBS->CloseProtocol(
			Handle,
			&gEfiSimpleFileSystemProtocolGuid,
			ImageHandle,
			NULL);

	return Status;
}
