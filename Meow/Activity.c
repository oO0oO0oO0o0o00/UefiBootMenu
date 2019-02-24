#include "Activity.h"

EFI_STATUS ActivityInitialize( ACTIVITY *this, UINT32 width, UINT32 height ){
	EFI_STATUS status;
	this->width = width;
	this->height = height;
	this->countInvalid = 0;
	this->evCount = 0;
	this->evIndex = 255;
	status = gBS->AllocatePool( EfiLoaderData, sizeof( EFI_GRAPHICS_OUTPUT_BLT_PIXEL )*width*height, &this->buffer );
	//this->buffer = ( EFI_GRAPHICS_OUTPUT_BLT_PIXEL*)malloc();
	return status;
}

void ActivityInvalidate( ACTIVITY *this, RECT rect ){
	//-1 means redraw whole Activity.
	if( this->countInvalid == 0xff )return;
	//Too many! Just redraw whole Activity.
	if( this->countInvalid >= 64 ){
		this->countInvalid = 0xff;
		return;
	}
	//Check fully collapsed areas.
	for( UINT8 i = 0; i < this->countInvalid; i++ ){
		RECT curr = this->invalids[i];
		if( curr.x <= rect.x && curr.y <= rect.y
			&& curr.x + curr.w >= rect.x + rect.w && curr.y + curr.h >= rect.y + rect.h )
				return;
		if( curr.x > rect.x && curr.y > rect.y
			&& curr.x + curr.w < rect.x + rect.w && curr.y + curr.h < rect.y + rect.h ){
				this->invalids[i] = rect;
				return;
		}
	}
	this->invalids[this->countInvalid] = rect;
	this->countInvalid++ ;
	//Ignore partial-collapsed areas for now.
}

EFI_STATUS ActivityRenderArea( ACTIVITY *this, EFI_GRAPHICS_OUTPUT_PROTOCOL *graphProtocol,
		RECT area ){
	return graphProtocol->Blt( graphProtocol, this->buffer, EfiBltBufferToVideo,
		area.x, area.y, area.x, area.y, area.w, area.h,
		this->width * sizeof( EFI_GRAPHICS_OUTPUT_BLT_PIXEL )
	);
}

EFI_STATUS ActivityRender( ACTIVITY *this, EFI_GRAPHICS_OUTPUT_PROTOCOL *graphProtocol ){
	EFI_STATUS status;
	if( this->countInvalid >= 64 ){
		RECT rect;
		rect.x = 0;
		rect.y = 0;
		rect.w = this->width;
		rect.h = this->height;
		return ActivityRenderArea( this, graphProtocol, rect );
	}
	for( UINT8 i = 0; i < this->countInvalid; i++ ){
		status = ActivityRenderArea( this, graphProtocol, this->invalids[i]);
		if( EFI_ERROR( status ) ) return status;
	}
	this->countInvalid = 0;
	return EFI_SUCCESS;
}
