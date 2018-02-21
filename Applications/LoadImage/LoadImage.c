#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/LoadedImage.h>
#include <Library/DevicePathLib.h>
#include <Guid/FileInfo.h>
#include <FileSystemLib/FileSystemLib.h>

EFI_STATUS
EFIAPI
LSRecursive(
	EFI_FILE_PROTOCOL *RootDir,
	CHAR16			  *Buffer,
	CHAR16			  *ImageName,
	CHAR16  		  *ImagePath,
	BOOLEAN 		  *Flag
)
{
	EFI_STATUS Status = EFI_SUCCESS;
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
			StrCpy(ImagePath, L"");
			return Status;
		}
		if (FileInfo->Attribute == EFI_FILE_DIRECTORY) {
			if (StrCmp(FileInfo->FileName, L".") != 0 && StrCmp(FileInfo->FileName, L"..") != 0) {
				StrCpy(Dir, ImagePath);
				StrCat(ImagePath, FileInfo->FileName);
				StrCat(ImagePath, L"\\");
				LSRecursive(File, FileInfo->FileName, ImageName, ImagePath, Flag);
				if (*Flag) {
					return Status;
				}
				StrCpy(ImagePath, Dir);
			}
		}
		if (StrCmp(FileInfo->FileName, ImageName) == 0) {
			StrCat(ImagePath, FileInfo->FileName);
			*Flag = TRUE;
			break;
		}
	}

FREE_RESOURCES:

	if (FileInfo != NULL) {
		FreePool(FileInfo);
	}

	if (Dir != NULL) {
		FreePool(Dir);
	}

	Status = CloseFileProtocol(File);

	Status = CloseFileProtocol(RootDir);

	return Status;
}

EFI_STATUS
EFIAPI
ReadInput (
	IN OUT CHAR16 *ImageName
)
{
	EFI_STATUS Status;
	UINTN EventIndex;
	UINTN Index = 0;
	EFI_INPUT_KEY Key;
	CHAR16 *Buffer = AllocateZeroPool(512 * sizeof(CHAR16));

	Print(L"Press the ENTER key to end the entry.\n");

	while (Key.UnicodeChar != SCAN_F3) {
		Status = gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &EventIndex);
		if (EFI_ERROR(Status)) {
			Print(L"Error: %r\n", Status);
			goto bottom;
		}

		Status = gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);
		if (EFI_ERROR(Status)) {
			Print(L"Error: %r\n", Status);
			goto bottom;
		}

		if (Key.UnicodeChar == CHAR_BACKSPACE) {
			if (Index == 0) continue;
			Print(L"%c", Key.UnicodeChar);
			Index--;
			continue;
		}

		Buffer[Index] = Key.UnicodeChar;
		Print(L"%c", Key.UnicodeChar);
		Index++;
	}

	Buffer[Index-1] = '\0';
	StrCpy(ImageName, Buffer);

bottom:

	if (Buffer != NULL) {
		FreePool(Buffer);
	}

	return Status;
}

EFI_STATUS
EFIAPI
UefiMain(
	IN EFI_HANDLE		ImageHandle,
	IN EFI_SYSTEM_TABLE *SystemTable
)
{
	EFI_STATUS Status;
	BOOLEAN Flag = FALSE;
	EFI_HANDLE Handle;
	EFI_HANDLE ImageApplication = NULL;
	EFI_DEVICE_PATH_PROTOCOL *DevicePath = NULL;
	CHAR16 *ImageName = NULL;
	CHAR16 *ImagePath = NULL;
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem = NULL;
	EFI_FILE_PROTOCOL *RootDir = NULL;
	UINTN BufferSize = 0;

	Status = gBS->LocateHandle(
		ByProtocol,
		&gEfiSimpleFileSystemProtocolGuid,
		NULL,
		&BufferSize,
		&Handle);
	if (Status == EFI_BUFFER_TOO_SMALL) {
		Status = gBS->LocateHandle(
			ByProtocol,
			&gEfiSimpleFileSystemProtocolGuid,
			NULL,
			&BufferSize,
			&Handle);
		if (EFI_ERROR(Status)) {
			Print(L"Could not locate handle: %r\n", Status);
			return Status;
		}
	}
	else if (EFI_ERROR(Status)) {
		Print(L"Could not locate handle: %r\n", Status);
		return Status;
	}

	Status = gBS->OpenProtocol(
		Handle,
		&gEfiSimpleFileSystemProtocolGuid,
		(VOID**)&FileSystem,
		ImageHandle,
		NULL,
		EFI_OPEN_PROTOCOL_GET_PROTOCOL);
	if (EFI_ERROR(Status)) {
		Print(L"Could not open protocol: %r\n", Status);
		return Status;
	}

	Status = FileSystem->OpenVolume(FileSystem, &RootDir);
	if (EFI_ERROR(Status)) {
		Print(L"Could not open root dir: %r\n", Status);
		goto bottom;
	}

	ImageName = AllocateZeroPool(512 * sizeof(CHAR16));
	ImageName[0] = '\0';
	ReadInput(ImageName);
	Print(L"Load image: %s\n", ImageName);

	ImagePath = AllocateZeroPool(512 * sizeof(CHAR16));
	ImagePath[0] = '\0';
	LSRecursive(RootDir, L".", ImageName, ImagePath, &Flag);
	Print(L"Image path: %s\n", ImagePath);

	DevicePath = FileDevicePath(Handle, ImagePath);

	Status = gBS->LoadImage(
			FALSE,
			ImageHandle,
			DevicePath,
			(VOID*)NULL,
			0,
			&ImageApplication);
	if (EFI_ERROR(Status)) {
		Print(L"Could not load image: %r\n", Status);
		goto FREE_RESOURCES;
	}
	Print(L"Image loaded.\n");

	Status = gBS->StartImage(ImageApplication, NULL, NULL);
	if (EFI_ERROR(Status)) {
		Print(L"Could not start image: %r\n", Status);
		goto FREE_RESOURCES;
	}
	Print(L"Image started.\n");


FREE_RESOURCES:

	if (ImageName != NULL) {
		FreePool(ImageName);
	}

	if (ImagePath != NULL) {
		FreePool(ImagePath);
	}

	if (DevicePath != NULL) {
		FreePool(DevicePath);
	}

	Status = CloseFileProtocol(RootDir);

bottom:

	Status = gBS->CloseProtocol(
			Handle,
			&gEfiSimpleFileSystemProtocolGuid,
			Handle,
			NULL);

	return Status;
}
