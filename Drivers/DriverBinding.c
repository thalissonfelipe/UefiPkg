#include <DriverBinding.h>

EFI_STATUS
EFIAPI
SimpleDriverBindingSupported (
	IN EFI_DRIVER_BINDING_PROTOCOL *This,
	IN EFI_HANDLE				   ControllerHandle,
	IN EFI_DEVICE_PATH_PROTOCOL    *RemainingDevicePath OPTIONAL
)
{
	return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
SimpleDriverBindingStart (
    IN  EFI_DRIVER_BINDING_PROTOCOL *This,
    IN  EFI_HANDLE				    ControllerHandle,
    IN  EFI_DEVICE_PATH_PROTOCOL 	*RemainingDevicePath OPTIONAL
)
{
	Print(L"Hello World!\n");
    return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
SimpleDriverBindingStop (
    IN EFI_DRIVER_BINDING_PROTOCOL *This,
    IN EFI_HANDLE 				   ControllerHandle,
    IN UINTN 					   NumberOfChildren,
    IN EFI_HANDLE 				   *ChildHandleBuffer OPTIONAL
)
{
    return EFI_UNSUPPORTED;
}

EFI_DRIVER_BINDING_PROTOCOL
gSimpleDriverBinding = {
    SimpleDriverBindingSupported,
    SimpleDriverBindingStart,
    SimpleDriverBindingStop,
    SIMPLE_DRIVER_VERSION,
    NULL,
    NULL,
};
