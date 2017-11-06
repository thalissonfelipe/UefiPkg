#include <ComponentName.h>
#include <DriverBinding.h>
#include <Protocol/DriverSupportedEfiVersion.h>

GLOBAL_REMOVE_IF_UNREFERENCED
EFI_DRIVER_SUPPORTED_EFI_VERSION_PROTOCOL
gSimpleSupportedEfiVersion = {
		sizeof(EFI_DRIVER_SUPPORTED_EFI_VERSION_PROTOCOL),
		EFI_2_60_SYSTEM_TABLE_REVISION
};

EFI_STATUS
EFIAPI
SimpleDriverLoad (
	IN EFI_HANDLE		  ImageHandle,
	IN EFI_SYSTEM_TABLE   *SystemTable
)
{
	EFI_STATUS Status = EFI_SUCCESS;

	gSimpleDriverBinding.ImageHandle = ImageHandle;
	gSimpleDriverBinding.DriverBindingHandle = ImageHandle;

	Status = gBS->InstallMultipleProtocolInterfaces (
		&ImageHandle,
		&gEfiDriverSupportedEfiVersionProtocolGuid,
		&gSimpleSupportedEfiVersion,
		&gEfiDriverBindingProtocolGuid,
		&gSimpleDriverBinding,
		&gEfiComponentNameProtocolGuid,
		&gSimpleComponentName,
		&gEfiComponentName2ProtocolGuid,
		&gSimpleComponentName2,
		NULL
	);

	ASSERT_EFI_ERROR (Status);

	return Status;
}

EFI_STATUS
EFIAPI
SimpleDriverUnload (
	IN EFI_HANDLE ImageHandle
)
{
	EFI_STATUS Status = EFI_SUCCESS;

	Status = gBS->UninstallMultipleProtocolInterfaces (
		ImageHandle,
		&gEfiDriverSupportedEfiVersionProtocolGuid,
		&gSimpleSupportedEfiVersion,
		&gEfiDriverBindingProtocolGuid,
		&gSimpleDriverBinding,
		&gEfiComponentNameProtocolGuid,
		&gSimpleComponentName,
		&gEfiComponentName2ProtocolGuid,
		&gSimpleComponentName2,
		NULL
	);

	ASSERT_EFI_ERROR (Status);

	return Status;
}
