#ifndef H_MEOW_MEOWFUNCTIONS
#define H_MEOW_MEOWFUNCTIONS

#include <Protocol/DevicePath.h>

EFI_STATUS DrawString( CHAR16 *string, const EFI_FONT_DISPLAY_INFO *fontDisplayInfo,
	EFI_IMAGE_OUTPUT *imageOutput, UINTN x, UINTN y );
	
EFI_STATUS DrawLines( CHAR16 *string, const EFI_FONT_DISPLAY_INFO *fontDisplayInfo,
	EFI_IMAGE_OUTPUT *imageOutput, UINTN x, UINTN y );

void Log( IN CHAR16 *text );

UINT32 SprintUint( UINT32 decimal, CHAR16 *buffer, UINT32 offset );

UINT32 Max( UINT32 one, UINT32 another );

EFI_EVENT GetKeyWaitor();

EFI_STATUS GetOneSecWaitor( OUT EFI_EVENT *event );

EFI_STATUS CancelTimer( EFI_EVENT *event );

EFI_STATUS GetKey( EFI_INPUT_KEY *key );

CHAR16 *MeowPathToText( EFI_DEVICE_PATH_PROTOCOL *path );

#endif