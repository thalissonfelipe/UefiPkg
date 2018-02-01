#ifndef _MINI_SHELL_LIB_
#define _MINI_SHELL_LIB_

EFI_STATUS
EFIAPI
GetShellInput (
	OUT UINTN  *Argc,
	OUT CHAR16 ***Argv
);

#endif
