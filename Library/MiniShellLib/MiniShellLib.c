#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/ShellParameters.h>

EFI_STATUS
EFIAPI
GetShellInput (
	OUT UINTN  *Argc,
	OUT CHAR16 ***Argv
)
{
	EFI_STATUS Status = EFI_SUCCESS;
	EFI_SHELL_PARAMETERS_PROTOCOL *ShellParameters;

	Status = gBS->HandleProtocol (
			gImageHandle,
			&gEfiShellParametersProtocolGuid,
			(VOID**)&ShellParameters);
	if (EFI_ERROR(Status)) {
		Print(L"Error: %r\n", Status);
		return Status;
	}

	*Argc = ShellParameters->Argc;
	*Argv = ShellParameters->Argv;

	return Status;
}

