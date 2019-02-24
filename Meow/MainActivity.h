#ifndef __MEOW_MAIN_ACTIVITY_H__
#define __MEOW_MAIN_ACTIVITY_H__

#include "Activity.h"
#include <Protocol/HiiFont.h>
#include <Library/UefiBootManagerLib.h>

typedef struct{
	ACTIVITY Activity;
	EFI_GRAPHICS_OUTPUT_BLT_PIXEL *bgBuffer;
	CHAR16 *staticTexts[8];
	UINTN BootOptionCount;
	EFI_BOOT_MANAGER_LOAD_OPTION *BootOptions;
	UINT32 BootOptionBaseY;
	UINT32 BootFailedBaseY;
	UINT32 BootAutoBaseY;
	UINTN TimerCount;
	UINTN selection;
	BOOLEAN IfShowBootFailed;
} MAIN_ACTIVITY;

EFI_STATUS newMainActivity( IN UINT32 width, IN UINT32 height, OUT ACTIVITY** Activity );

void MainActivityOnStart( ACTIVITY *this );

void MainActivityOnEnd( ACTIVITY *this );

void MainActivityOnEvent( ACTIVITY *this );

#endif
