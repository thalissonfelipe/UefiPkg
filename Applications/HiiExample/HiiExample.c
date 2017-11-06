#include "HiiExample.h"
#include "HiiExampleData.h"

EFI_GUID mFormSetGuid = HII_EXAMPLE_GUID;

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS                      Status;
  EFI_HII_HANDLE                  HiiHandle;
  EFI_SCREEN_DESCRIPTOR           Screen;
  EFI_FORM_BROWSER2_PROTOCOL      *FormBrowser2 = NULL;

  SystemTable->ConOut->ClearScreen(SystemTable->ConOut);

  ZeroMem (&Screen, sizeof (EFI_SCREEN_DESCRIPTOR));
  gST->ConOut->QueryMode (gST->ConOut, gST->ConOut->Mode->Mode, &Screen.RightColumn, &Screen.BottomRow);
  Screen.TopRow     = 3;
  Screen.BottomRow  = Screen.BottomRow - 3;

  Status = gBS->LocateProtocol (&gEfiFormBrowser2ProtocolGuid, NULL, (VOID **) &FormBrowser2);
  if (EFI_ERROR (Status)) {
	  return Status;
  }

  HiiHandle = HiiAddPackages (
          &mFormSetGuid,
          NULL,
          HiiExampleStrings,
          HiiExampleVfrBin,
          NULL);

  Status = FormBrowser2->SendForm (
		   FormBrowser2,
		   &HiiHandle,
		   1,
		   &mFormSetGuid,
		   1,
		   &Screen,
		   NULL);
  if (EFI_ERROR(Status)) {
	  return Status;
  }

  HiiRemovePackages (HiiHandle);

  return Status;
}
