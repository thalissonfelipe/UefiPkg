#ifndef _HII_EXAMPLE_STRUCT_
#define _HII_EXAMPLE_STRUCT_

#include <Guid/HiiPlatformSetupFormset.h>
#include <Guid/HiiFormMapMethodGuid.h>
#include <Guid/DriverSampleHii.h>
#include <Guid/ZeroGuid.h>

#pragma pack(1)
typedef struct {
	UINT8 Field01;
	UINT8 Field02;
	UINT8 Field03;
	UINT8 Field04;
	UINT8 Field05;
	UINT8 Field06;
} HII_EXAMPLE;

#pragma pack()

#endif
