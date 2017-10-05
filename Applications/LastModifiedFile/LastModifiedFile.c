#include  <Uefi.h>
#include  <Library/UefiLib.h>
#include  <Library/UefiApplicationEntryPoint.h>
#include  <Library/UefiBootServicesTableLib.h>
#include  <Library/MemoryAllocationLib.h>
#include  <Protocol/SimpleFileSystem.h>
#include  <Guid/FileInfo.h>
#include  <Library/BaseMemoryLib.h>

EFI_FILE_INFO*
CompareTwoTimes(
	EFI_FILE_INFO *FileInfo1,
	EFI_FILE_INFO *FileInfo2
)
{
	EFI_TIME Time1 = FileInfo1->ModificationTime;
	EFI_TIME Time2 = FileInfo2->ModificationTime;

	if (Time1.Year > Time2.Year)
		return FileInfo1;
	else if (Time1.Year < Time2.Year)
		return FileInfo2;
	else {
		if (Time1.Month > Time2.Month)
			return FileInfo1;
	   	else if (Time1.Month < Time2.Month)
	   		return FileInfo2;
	   	else {
	   		if (Time1.Day > Time2.Day)
	   			return FileInfo1;
	   		else if (Time1.Day < Time2.Day)
	   			return FileInfo2;
	   		else {
	   			if (Time1.Hour > Time2.Hour)
	   				return FileInfo1;
	   			else if (Time1.Hour < Time2.Hour)
	   				return FileInfo2;
	   			else {
	   				if (Time1.Minute > Time2.Minute)
	   					return FileInfo1;
	   				else if (Time1.Minute < Time2.Minute)
	   					return FileInfo2;
	   				else {
	   					if (Time1.Second > Time2.Second)
	   						return FileInfo1;
	   					else
	   						return FileInfo2;
	   				}
	   			}
	   		}
	   	}
	}
}

EFI_STATUS
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
	EFI_FILE_INFO *FileInfo = (EFI_FILE_INFO*)AllocateZeroPool(BufferSize);
	Status = RootDir->Open(
			RootDir,
			&File,
			Path,
			EFI_FILE_MODE_READ,
			0);
	if (EFI_ERROR(Status)) {
		Print(L"Could not open file: %r\n", Status);
		goto close_root;
	}

	while (TRUE) {
		BufferSize = SIZE_OF_EFI_FILE_INFO + 512 * sizeof(CHAR16);
		Status = File->Read(File, &BufferSize, FileInfo);
		if (EFI_ERROR(Status)) {
			Print(L"Could not read file: %r\n", Status);
			goto close_file;
		}
		if (BufferSize == 0)
			return Status;
		if (FileInfo->Attribute == EFI_FILE_DIRECTORY) {
			if (StrCmp(FileInfo->FileName, L".") != 0 && StrCmp(FileInfo->FileName, L"..") != 0) {
				LSRecursive(File, FileInfo->FileName, FileInfoAux, FileName);
			}
		}
		else {
			EFI_FILE_INFO *Result = CompareTwoTimes(FileInfo, FileInfoAux);
			CopyMem(FileInfoAux, Result, SIZE_OF_EFI_FILE_INFO + (StrLen(Result->FileName) * 2) + 1);
			StrCpy(FileName, FileInfoAux->FileName);
		}
	}

	FreePool(FileInfo);

close_file:
	File->Close(File);

close_root:
	RootDir->Close(RootDir);

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
    EFI_HANDLE NtfsHandle = NULL;
    UINTN BufferSize = SIZE_OF_EFI_FILE_INFO + 512 * sizeof(CHAR16);
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem = NULL;
    EFI_FILE_PROTOCOL *RootDir = NULL;
    EFI_FILE_INFO *FileInfo = (EFI_FILE_INFO*)AllocateZeroPool(BufferSize);

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

    Status = gBS->OpenProtocol(
        NtfsHandle,
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


    FileInfo->ModificationTime.Year = 0;
    FileInfo->ModificationTime.Month = 0;
    FileInfo->ModificationTime.Day = 0;
    FileInfo->ModificationTime.Hour = 0;
    FileInfo->ModificationTime.Minute = 0;
    FileInfo->ModificationTime.Second = 0;

    CHAR16* FileName = (CHAR16*)AllocateZeroPool(512 * sizeof(CHAR16));
    FileName[0] = '\0';
    LSRecursive(RootDir, L".", FileInfo, FileName);
    Print(L"Último arquivo modificado: %s\n", FileName);
    Print(L"Data: %d/%d/%d \n", FileInfo->ModificationTime.Day, FileInfo->ModificationTime.Month, FileInfo->ModificationTime.Year);
    Print(L"Hora: %d:%d:%d \n", FileInfo->ModificationTime.Hour, FileInfo->ModificationTime.Minute, FileInfo->ModificationTime.Second);

    FreePool(FileInfo);
    FreePool(FileName);


close_fs:
    gBS->CloseProtocol(
        NtfsHandle,
        &gEfiSimpleFileSystemProtocolGuid,
        ImageHandle,
        NULL);

bottom:
    return Status;
}

