//
////  Copyright META. All Rights Reserved.  \\\\
//

#pragma once

#include "CoreMinimal.h"

struct FPixelStreamingMethod
{

#define META_BIND_METHOD(InternalMethodName, ExternalMethodName) static constexpr auto InternalMethodName = TEXT(#ExternalMethodName)

        META_BIND_METHOD(SendTimerPolygonCoordinates, sendTimerPolygonCoordinates);
        META_BIND_METHOD(SendScreenTouchAsWGS, onScanResponseReceived);
        META_BIND_METHOD(SendSearchAreaStartScale, sendSearchAreaStartScale);
        META_BIND_METHOD(SendGeoPinClick, sendGeoPinClick);

        META_BIND_METHOD(OnUserChanged, onUserChanged);
        META_BIND_METHOD(OnGetSearchAreaScale, onGetSearchAreaScale);
        META_BIND_METHOD(OnChangeActiveStateSearchArea, onChangeActiveStateSearchArea);
        META_BIND_METHOD(OnBackToOrigin, onBackToOrigin);
        META_BIND_METHOD(OnScanResponseReceived, onScanResponseReceived);
        META_BIND_METHOD(OnGeoPinsReceived, onGeoPinsReceived);
        META_BIND_METHOD(OnTranslateCameraLocationReceived, onTranslateCameraLocationReceived);
        
#undef META_BIND_METHOD
};
