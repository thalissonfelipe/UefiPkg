#ifndef _HII_EXAMPLE_STRUCT_
#define _HII_EXAMPLE_STRUCT_

#include <Guid/HiiPlatformSetupFormset.h>
#include <Guid/HiiFormMapMethodGuid.h>
#include <Guid/DriverSampleHii.h>

#pragma pack(1)
typedef struct {
	UINT8 	Checkbox;
	UINT8 	OneOf;
	UINT8 	OrderedList[3];
	UINT16  String1[40];
	UINT16  String2[40];
	UINT8 	FavoriteNumber;
	UINT8 	Numeric;
} HII_EXAMPLE;

#pragma pack()

#endif
