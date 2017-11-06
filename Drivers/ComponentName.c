#include <ComponentName.h>

EFI_STATUS
EFIAPI
SimpleComponentNameGetDriverName (
	IN  EFI_COMPONENT_NAME2_PROTOCOL *ComponentName2,
	IN  CHAR8				 		 *Language,
	OUT CHAR16						 **DriverName
)
{
	*DriverName = L"SimpleDriver";
	return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SimpleComponentNameGetControllerName (
    IN  EFI_COMPONENT_NAME2_PROTOCOL *ComponentName2,
    IN  EFI_HANDLE 					 ControllerHandle,
    IN  EFI_HANDLE 					 ChildHandle OPTIONAL,
    IN  CHAR8 						 *Language,
    OUT CHAR16 						 **ControllerName
)
{
    *ControllerName = L"SimpleController";
    return EFI_SUCCESS;
}

GLOBAL_REMOVE_IF_UNREFERENCED
EFI_COMPONENT_NAME_PROTOCOL
gSimpleComponentName = {
    (EFI_COMPONENT_NAME_GET_DRIVER_NAME) SimpleComponentNameGetDriverName,
    (EFI_COMPONENT_NAME_GET_CONTROLLER_NAME) SimpleComponentNameGetControllerName,
    "eng"
};

GLOBAL_REMOVE_IF_UNREFERENCED
EFI_COMPONENT_NAME2_PROTOCOL
gSimpleComponentName2 = {
    SimpleComponentNameGetDriverName,
    SimpleComponentNameGetControllerName,
    "en"
};
