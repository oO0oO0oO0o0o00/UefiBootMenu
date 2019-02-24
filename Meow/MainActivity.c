#include "MainActivity.h"
#include "MeowFunctions.h"

#define COLOR_BG {0xdd, 0x10, 0x10, 0x00}
#define COLOR_FG {0xff, 0x79, 0x29, 0x00}

#define DIM_ITEM_SEL_H_OFFSET 40
#define DIM_ITEM_SEL_HEIGHT 32
#define DIM_ITEM_TEXT_X_OFFSET 80
#define DIM_ITEM_TEXT_Y_OFFSET 8
#define DIM_PARA_SEP 16
#define DIM_TITLE_Y_OFFSET 40

void MainActivityDrawRect( EFI_GRAPHICS_OUTPUT_BLT_PIXEL *buffer, UINT32 width, RECT rect,
		EFI_GRAPHICS_OUTPUT_BLT_PIXEL color, OPTIONAL EFI_GRAPHICS_OUTPUT_BLT_PIXEL *bg ){
	UINT32 base = width * rect.y + rect.x;
	for( UINT32 y = 0; y<rect.h; y++ ){
		UINT32 offset = base + y * width;
		for( UINT32 x = 0; x<rect.w; x++ )
			buffer[offset + x] = bg ? bg[offset + x] : color;
	}
}

EFI_STATUS MainActivityDrawString( MAIN_ACTIVITY *this, CHAR16 *string, 
		UINTN x, UINTN y, BOOLEAN reverse ){
	EFI_FONT_DISPLAY_INFO displayInfo = { {0, 0, 0, 0}, {0, 0, 0, 0}, EFI_FONT_INFO_ANY_FONT,
		{EFI_HII_FONT_STYLE_NORMAL, 0, L'0'}
	};
	if( reverse )
		displayInfo.ForegroundColor = ( EFI_GRAPHICS_OUTPUT_BLT_PIXEL )COLOR_BG;
	else
		displayInfo.ForegroundColor = ( EFI_GRAPHICS_OUTPUT_BLT_PIXEL )COLOR_FG;
	
	EFI_IMAGE_OUTPUT output;
	output.Width = ( UINT16 ) this->Activity.width;
	output.Height = ( UINT16 ) this->Activity.height;
	output.Image.Bitmap = this->Activity.buffer;
	
	return DrawString( string, &displayInfo, &output, x, y );
}

EFI_STATUS MainActivityDrawBootOption( MAIN_ACTIVITY *this, UINTN index,
		BOOLEAN reverse, BOOLEAN update ){
	EFI_STATUS status;
	UINT32 y = this->BootOptionBaseY + DIM_ITEM_SEL_HEIGHT * ( UINT32 ) index;
	RECT rect;
	rect.x = DIM_ITEM_SEL_H_OFFSET;
	rect.y = y;
	rect.w = this->Activity.width - 2 * DIM_ITEM_SEL_H_OFFSET;
	rect.h = DIM_ITEM_SEL_HEIGHT;
	
	if( reverse )
		MainActivityDrawRect( this->Activity.buffer, this->Activity.width, rect,
			( EFI_GRAPHICS_OUTPUT_BLT_PIXEL )COLOR_FG, NULL );
	else if( update )
		MainActivityDrawRect( this->Activity.buffer, this->Activity.width, rect,
			( EFI_GRAPHICS_OUTPUT_BLT_PIXEL )COLOR_FG, this->bgBuffer );
	
	status = MainActivityDrawString( this, this->BootOptions[index].Description,
		DIM_ITEM_TEXT_X_OFFSET, y + DIM_ITEM_TEXT_Y_OFFSET, reverse );
	if( EFI_ERROR( status ) ) return status;
	
	if( update )
		ActivityInvalidate( ( ACTIVITY*)this, rect );
	
	return EFI_SUCCESS;
}

UINT32 MainActivityStrCpy( CHAR16 *dst, CHAR16 *src, UINT32 dstOffset ){
	for( UINT32 offset = 0; src[offset]; offset++ , dstOffset++ )
		dst[dstOffset] = src[offset];
	dst[dstOffset] = 0;
	return dstOffset;
}
	

EFI_STATUS MainActivityDrawTimerString( MAIN_ACTIVITY *this ){
	CHAR16 line[256];
	UINT32 offset = 0;
	RECT rect;
	rect.x = DIM_ITEM_SEL_H_OFFSET;
	rect.y = this->BootAutoBaseY;
	rect.w = this->Activity.width - 2 * DIM_ITEM_SEL_H_OFFSET;
	rect.h = DIM_ITEM_SEL_HEIGHT;
	
	MainActivityDrawRect( this->Activity.buffer, this->Activity.width,
		rect, ( EFI_GRAPHICS_OUTPUT_BLT_PIXEL )COLOR_BG, this->bgBuffer );
	
	offset = MainActivityStrCpy( line, this->staticTexts[3], offset );
	offset += SprintUint( ( UINT32 ) this->TimerCount, line, offset );
	MainActivityStrCpy( line, this->staticTexts[4], offset );
	MainActivityDrawString( this, line, DIM_ITEM_TEXT_X_OFFSET,
		rect.y + DIM_ITEM_TEXT_Y_OFFSET, FALSE );
	
	ActivityInvalidate(&this->Activity, rect );
	return EFI_SUCCESS;
}

//EFI_STATUS MainActivitySetTimer( MAIN_ACTIVITY *this ){
	//EFI_EVENT ev = this->Activity.events[1];

EFI_STATUS newMainActivity( IN UINT32 width, IN UINT32 height, OUT ACTIVITY** Activity ){
	EFI_STATUS status = gBS->AllocatePool( EfiLoaderData, sizeof( MAIN_ACTIVITY ), Activity );
	//Activity = ( MAIN_ACTIVITY*)malloc();
	if( EFI_ERROR( status ) )return status;
	
	ACTIVITY *super = ( ACTIVITY* ) *Activity;
	super->onStart = MainActivityOnStart;
	super->onEnd = MainActivityOnEnd;
	super->onEvent = MainActivityOnEvent;
	super->isNonFullScreen = FALSE;
	
	// Super init.
	status = ActivityInitialize( super, width, height );
	if( EFI_ERROR( status ) ) return status;
	
	// This init.
	MAIN_ACTIVITY *this = ( MAIN_ACTIVITY*)super;
	
	// Allocate memory for background buffer.
	status = gBS->AllocatePool( EfiLoaderData, sizeof( EFI_GRAPHICS_OUTPUT_BLT_PIXEL )*width*height, &this->bgBuffer );
	if( EFI_ERROR( status ) ) return status;
	
	// Init text resouces. Not using res file for now.
	this->staticTexts[0] = L"Meow Boot Menu";
	this->staticTexts[1] = L"System efi boot options:";
	this->staticTexts[2] = L"Boot failed.";
	this->staticTexts[3] = L"Auto booting in ";
	this->staticTexts[4] = L" second( s ).";
	
	// Fill bg buffer.
	EFI_GRAPHICS_OUTPUT_BLT_PIXEL color = COLOR_BG;
	RECT rect = {0, 0, 0, 0};
	rect.w = width;
	rect.h = height;
	MainActivityDrawRect( this->bgBuffer, width, rect, color, NULL );
	rect.x = DIM_ITEM_SEL_H_OFFSET;
	rect.w = width - DIM_ITEM_SEL_H_OFFSET * 2;
	rect.y = DIM_TITLE_Y_OFFSET;
	rect.h = DIM_TITLE_Y_OFFSET;
	color = ( EFI_GRAPHICS_OUTPUT_BLT_PIXEL )COLOR_FG;
	MainActivityDrawRect( this->bgBuffer, width, rect, color, NULL );
	
	return status;
}

void MainActivityOnStart( ACTIVITY *this ){
	UINT32 base = 0;
	MAIN_ACTIVITY *thiz = ( MAIN_ACTIVITY*) this;
	for( UINT32 y = 0; y < this->height; y++ ){
		for( UINT32 x = 0; x < this->width; x++ )
			this->buffer[base + x] = thiz->bgBuffer[base + x];
		base += this->width;
	}
	
	EfiBootManagerConnectAll();
	EfiBootManagerRefreshAllBootOption();
	 
	thiz->BootOptions = EfiBootManagerGetLoadOptions(&thiz->BootOptionCount, LoadOptionTypeBoot );
	
	
	thiz->BootOptionBaseY = DIM_TITLE_Y_OFFSET + 2 * DIM_ITEM_SEL_HEIGHT + 3 * DIM_PARA_SEP;
	thiz->BootFailedBaseY = thiz->BootOptionBaseY + DIM_PARA_SEP +
		DIM_ITEM_SEL_HEIGHT * ( UINT32 ) thiz->BootOptionCount;
	thiz->BootAutoBaseY = thiz->BootFailedBaseY + DIM_ITEM_SEL_HEIGHT + DIM_PARA_SEP;
	thiz->IfShowBootFailed = FALSE;
	
	MainActivityDrawString( thiz, thiz->staticTexts[0], DIM_ITEM_TEXT_X_OFFSET, 
		DIM_TITLE_Y_OFFSET + DIM_ITEM_TEXT_Y_OFFSET, TRUE );
	MainActivityDrawString( thiz, thiz->staticTexts[1], DIM_ITEM_SEL_H_OFFSET,
		DIM_TITLE_Y_OFFSET + DIM_ITEM_SEL_HEIGHT + 2 * DIM_PARA_SEP + DIM_ITEM_TEXT_Y_OFFSET,
		FALSE );
	
	if( thiz->BootOptionCount > 0 )
		MainActivityDrawBootOption( thiz, 0, TRUE, FALSE );
	
	for( UINTN i = 1; i < thiz->BootOptionCount; i++ )
		 MainActivityDrawBootOption( thiz, i, FALSE, FALSE );
	
	thiz->selection = 0;
	
	RECT rect = {0, 0, 0, 0};
	rect.w = this->width;
	rect.h = this->height;
	ActivityInvalidate( this, rect );
	
	thiz->TimerCount = 5;
	this->events[0] = GetKeyWaitor();
	EFI_STATUS status = GetOneSecWaitor( &this->events[1]);
	if( EFI_ERROR( status ) )
		this->evCount = 1;
	else{
		this->evCount = 2;
		MainActivityDrawTimerString( thiz );
		this->countInvalid = 1;
	}
}

void MainActivityOnEnd( ACTIVITY *this ){
	//
}

inline void moveDown( MAIN_ACTIVITY* this ){
	if( this->BootOptionCount > 0 ){
		MainActivityDrawBootOption( this, this->selection, FALSE, TRUE );
		this->selection++ ;
		if( this->selection >= this->BootOptionCount ) this->selection = 0;
		MainActivityDrawBootOption( this, this->selection, TRUE, TRUE );
	}
}

inline void moveUp( MAIN_ACTIVITY* this ){
	if( this->BootOptionCount > 0 ){
		MainActivityDrawBootOption( this, this->selection, FALSE, TRUE );
		if( this->selection == 0 ) this->selection = this->BootOptionCount - 1;
		else this->selection--;
		MainActivityDrawBootOption( this, this->selection, TRUE, TRUE );
	}
}

inline void onEnter( MAIN_ACTIVITY* this ){
	EfiBootManagerBoot(&this->BootOptions[this->selection]);
	ACTIVITY* super = ( ACTIVITY*) this;
	RECT rect;
	rect.x = DIM_ITEM_SEL_H_OFFSET;
	rect.y = this->BootFailedBaseY;
	rect.w = super->width - 2 * DIM_ITEM_SEL_H_OFFSET;
	rect.h = DIM_ITEM_SEL_HEIGHT;
	MainActivityDrawString( this, this->staticTexts[2], rect.x,
		rect.y + DIM_ITEM_TEXT_Y_OFFSET, FALSE );
	
	rect.x = 0;
	rect.y = 0;
	rect.w = super->width;
	rect.h = super->height;
	ActivityInvalidate( super, rect );
	this->IfShowBootFailed = TRUE;
}

void MainActivityClearTimer( MAIN_ACTIVITY *this ){
	CancelTimer(&this->Activity.events[1]);
	this->Activity.evCount = 1;
	
	RECT rect;
	rect.x = DIM_ITEM_SEL_H_OFFSET;
	rect.y = this->BootAutoBaseY;
	rect.w = this->Activity.width - 2 * DIM_ITEM_SEL_H_OFFSET;
	rect.h = DIM_ITEM_SEL_HEIGHT;
	
	MainActivityDrawRect( this->Activity.buffer, this->Activity.width,
		rect, ( EFI_GRAPHICS_OUTPUT_BLT_PIXEL )COLOR_BG, this->bgBuffer );
	ActivityInvalidate(&this->Activity, rect );
}

void onKey( MAIN_ACTIVITY *this, EFI_INPUT_KEY key ){
	ACTIVITY* super = ( ACTIVITY* ) this;
	MainActivityClearTimer( this );
	RECT rect;
	rect.x = DIM_ITEM_SEL_H_OFFSET;
	rect.y = this->BootFailedBaseY;
	rect.w = super->width - 2 * DIM_ITEM_SEL_H_OFFSET;
	rect.h = DIM_ITEM_SEL_HEIGHT;
	if( this->IfShowBootFailed ){
		MainActivityDrawRect( super->buffer, super->width, rect,
			( EFI_GRAPHICS_OUTPUT_BLT_PIXEL ) COLOR_BG, this->bgBuffer );
		ActivityInvalidate( super, rect );
		this->IfShowBootFailed = FALSE;
	}
	switch( key.ScanCode ){
		case SCAN_UP:
			moveUp( this );
			return;
		case SCAN_DOWN:
			moveDown( this );
			return;
		case SCAN_NULL:
			break;
		default:
			return;
	}
	switch( key.UnicodeChar ){
		case CHAR_LINEFEED:
		case CHAR_CARRIAGE_RETURN:
			onEnter( this );
			break;
	}
}

void MainActivityOnEvent( ACTIVITY *this ){
	EFI_STATUS status;
	switch( this->evIndex ){
		case 0:{
				EFI_INPUT_KEY key;
				status = GetKey( &key );
				if( EFI_ERROR( status ) ) break;
				onKey( ( MAIN_ACTIVITY* ) this, key );
			}
			break;
		case 1:{
				MAIN_ACTIVITY *thiz = ( MAIN_ACTIVITY* ) this;
				if( thiz->TimerCount > 1 ){
					thiz->TimerCount--;
					MainActivityDrawTimerString( thiz );
					//GetOneSecWaitor( this->events[1]);
				}else{
					MainActivityClearTimer( thiz );
					onEnter( thiz );
				}
			}
			break;
		default:
			break;
	}
	this->evIndex = 255;
}
