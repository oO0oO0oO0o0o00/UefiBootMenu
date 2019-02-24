#ifndef __MEOW_ACTIVITY_H__
#define __MEOW_ACTIVITY_H__

#include <Uefi.h>
#include <Protocol/GraphicsOutput.h>
#include <Library/UefiBootServicesTableLib.h>

//Typedefs, struct declares.

typedef struct RECT RECT;
typedef struct ACTIVITY ACTIVITY;

//Function declares.

EFI_STATUS ActivityInitialize( ACTIVITY *this, UINT32 width, UINT32 height );

void ActivityInvalidate( ACTIVITY *this, RECT rect );

EFI_STATUS ActivityRender( ACTIVITY *this, EFI_GRAPHICS_OUTPUT_PROTOCOL *graphProtocol );

//Struct defines.

struct RECT{
	UINT32 x;
	UINT32 y;
	UINT32 w;
	UINT32 h;
};

struct ACTIVITY{
	void (*onStart )( ACTIVITY*);
	void (*onResume )( ACTIVITY*);
	void (*onPause )( ACTIVITY*);
	void (*onEnd )( ACTIVITY*);
	void (*onEvent )( ACTIVITY*);
	ACTIVITY *parent;
	BOOLEAN isNonFullScreen;
	UINT32 width;
	UINT32 height;
	EFI_GRAPHICS_OUTPUT_BLT_PIXEL *buffer;
	UINT8 countInvalid;
	RECT invalids[64];
	UINTN evCount;
	UINTN evIndex;
	EFI_EVENT events[5];
};

#endif
