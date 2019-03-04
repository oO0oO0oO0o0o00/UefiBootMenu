#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/SimplePointer.h>
#include <Protocol/SimpleTextInEx.h>
#include <Protocol/HiiFont.h>
#include <Protocol/DevicePathToText.h>
#include "Activity.h"
#include "MainActivity.h"
#include "MeowFunctions.h"

///////////////

UINT32				IsRunning;

EFI_SYSTEM_TABLE	*SysTable;

EFI_HANDLE			EscHandle;

ACTIVITY			*TopActivity;

EFI_GRAPHICS_OUTPUT_PROTOCOL *GraphProtocol;

EFI_HII_FONT_PROTOCOL *FontProtocol;

EFI_DEVICE_PATH_TO_TEXT_PROTOCOL *PathConvProtocol;

///////////////

EFI_STATUS EscNotify( EFI_KEY_DATA *key );

EFI_STATUS Loop();

///////////////

EFI_STATUS UefiMain( IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable ){
	
	EFI_STATUS				status;
	UINT32					Width;
	UINT32					Height;
	CHAR16					Line[32];
	UINT32					Tmp;
	EFI_KEY_DATA			EscKey = { 0 };
	
	// Save for latter use.
	SysTable = SystemTable;
	
	// Graphic access.
	status = gBS->LocateProtocol( &gEfiGraphicsOutputProtocolGuid,
		NULL, ( VOID **) &GraphProtocol
	);
	if( EFI_ERROR( status ) ){
		// Well, if graphic's not available, where comes the shell?
		Log( L"Cannot locate graphics output protocol.\r\n" );
		return status;
	}
	
	// Assuming screen size won't change.
	// After all, this is just a boot menu.
	Width = ( UINT32 ) GraphProtocol->Mode->Info->HorizontalResolution;
	Height = ( UINT32 ) GraphProtocol->Mode->Info->VerticalResolution;
	Log( L"Got resolution " );
	Tmp = SprintUint( Width, Line, 0 );
	Line[Tmp] = 'x';
	Tmp++ ;
	Tmp += SprintUint( Height, Line, Tmp );
	Log( Line );
	Log( L".\r\n" );

	// Bind ESC key for exit.
	// Why do we need this?
	// Locate the protocol first.
	EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *inputProtocol;
	status = gBS->LocateProtocol( &gEfiSimpleTextInputExProtocolGuid,
		NULL, ( VOID **) &inputProtocol
	);
	if( EFI_ERROR( status ) ){
		Log( L"Cannot locate input protocol.\r\n" );
		return status;
	}
	EscKey.Key.ScanCode = SCAN_ESC;
	
	// Register notify.
	status = inputProtocol->RegisterKeyNotify( inputProtocol, &EscKey,
		EscNotify, ( VOID **) &EscHandle
	);
	if( EFI_ERROR( status ) ){
		Log( L"Cannot bind esc key.\r\n" );
		return status;
	}
	
	Log( L"Bond esc key.\r\n" );
	
	// Locate font protocol, we use it to draw strings.
	status = gBS->LocateProtocol( &gEfiHiiFontProtocolGuid, NULL, ( VOID **) &FontProtocol );
	if( EFI_ERROR( status ) ){
		Log( L"cannot locate font protocol.\r\n" );
		return status;
	}
	
	// 0 means running.
	IsRunning = 0;
	
	// Create the Main Activity.
	status = newMainActivity( Width, Height, &TopActivity );
	if( EFI_ERROR( status ) ){
		Log( L"Cannot allocate MainActivity.\r\n" );
		return status;
	}
	
	status = gBS->LocateProtocol( &gEfiDevicePathToTextProtocolGuid,
		NULL, ( VOID **) &PathConvProtocol );
	if( EFI_ERROR( status ) ){
		Log( L"Cannot locate device path to text protocol." );
		PathConvProtocol = NULL;
	}
	
	// Start the Activity.
	TopActivity->onStart( TopActivity );
	
	// Enter event loop.
	Log( L"-> loop.\r\n" );
	status = Loop();
	Log( L"<- loop.\r\n" );
	
	// Unregister key notify.
	inputProtocol->UnregisterKeyNotify( inputProtocol, EscHandle );
	
	// Log previous exception from event loop.
	if( EFI_ERROR( status ) ){
		Log( L"Cannot draw frame.\r\n");
		return status;
	}
	
	// Bye.
	return EFI_SUCCESS;
}

UINT32 SprintUint( UINT32 Decimal, CHAR16 *Buffer, UINT32 Offset ){
	CHAR16				Tmp[10];
	UINT32				MaxVal;
	
	MaxVal = 0;
	for( UINT32 i = 0; 0 != Decimal; i++ ){
		UINT32 digit = Decimal % 10;
		Tmp[i] = ( CHAR16 ) ( '0' + digit );
		Decimal = Decimal / 10;
		// Trust the compiler it would optimize:)
		if( digit != 0 ) MaxVal=i+ 1;
	}
	for( UINT32 i = 0; i < MaxVal; i++ )
		Buffer[Offset + i] = Tmp[MaxVal - i - 1];
	if( MaxVal == 0 ){
		Buffer[Offset] = '0';
		return 1;
	}
	return MaxVal;
}

UINT32 Max( UINT32 One, UINT32 Another ){
	return ( One > Another ) ? One : Another;
}

void Log( IN CHAR16 *Text ){
	SysTable->ConOut->OutputString( SysTable->ConOut, Text );
}

EFI_STATUS DrawString( CHAR16 *string, const EFI_FONT_DISPLAY_INFO *fontDisplayInfo,
	EFI_IMAGE_OUTPUT *imageOutput, UINTN x, UINTN y ){
	return FontProtocol->StringToImage( FontProtocol,
		EFI_HII_OUT_FLAG_CLIP | EFI_HII_OUT_FLAG_CLIP_CLEAN_X
		| EFI_HII_OUT_FLAG_CLIP_CLEAN_Y | EFI_HII_IGNORE_LINE_BREAK | EFI_HII_OUT_FLAG_TRANSPARENT,
		string, fontDisplayInfo, &imageOutput, x, y, NULL, NULL, NULL );
}

EFI_STATUS DrawLines( CHAR16 *string, const EFI_FONT_DISPLAY_INFO *fontDisplayInfo,
	EFI_IMAGE_OUTPUT *imageOutput, UINTN x, UINTN y ){
	return FontProtocol->StringToImage( FontProtocol, EFI_HII_OUT_FLAG_TRANSPARENT,
		string, fontDisplayInfo, &imageOutput, x, y, NULL, NULL, NULL );
}

EFI_EVENT GetKeyWaitor(){
	return SysTable->ConIn->WaitForKey;
}

EFI_STATUS GetKey( EFI_INPUT_KEY *key ){
	return SysTable->ConIn->ReadKeyStroke( SysTable->ConIn, key );
}

EFI_STATUS GetOneSecWaitor( OUT EFI_EVENT *event ){
	EFI_STATUS status = gBS->CreateEvent( EVT_TIMER, TPL_APPLICATION, ( EFI_EVENT_NOTIFY ) NULL,
		( VOID*) NULL, event );
	status = gBS->SetTimer( *event, TimerPeriodic, 10000000 );
	return status;
}

EFI_STATUS CancelTimer( EFI_EVENT *event ){
	return gBS->SetTimer( *event, TimerCancel, 10000000 );
}

CHAR16 *MeowPathToText( EFI_DEVICE_PATH_PROTOCOL *path ){
	if( PathConvProtocol ) 
		return PathConvProtocol->ConvertDevicePathToText( path, TRUE, TRUE );
	return NULL;
}

EFI_STATUS Loop(){
	EFI_STATUS status;
	while( IsRunning == 0 ){
		if( TopActivity == NULL ){
			Log( L"TopActivity == NULL\r\n");
			return EFI_SUCCESS;
		}
		//Now we only draw top most Activity.
		TopActivity->onEvent( TopActivity );
		status = ActivityRender( TopActivity, GraphProtocol );
		if( EFI_ERROR( status ) ){
			Log( L"Cannot render Activity.\r\n");
			return status;
		}
		gBS->WaitForEvent( TopActivity->evCount, TopActivity->events, &TopActivity->evIndex );
	}
	return EFI_SUCCESS;
}

EFI_STATUS EscNotify( EFI_KEY_DATA *key ){
	IsRunning = 1;
	return EFI_SUCCESS;
}
