#include "GameFramework/GameplayMessageTags.h"

namespace GameplayTagsRouter
{
        UE_DEFINE_GAMEPLAY_TAG_COMMENT(OnUserChanged, "JsonRpc.Message.OnUserChange", "");
        UE_DEFINE_GAMEPLAY_TAG_COMMENT(OnChangeActiveStateSearchArea, "JsonRpc.Message.OnChangeActiveStateSearchArea", "");
        UE_DEFINE_GAMEPLAY_TAG_COMMENT(OnGetSearchAreaScale, "JsonRpc.Message.OnGetSearchAreaScale", "");
        UE_DEFINE_GAMEPLAY_TAG_COMMENT(OnBackToOrigin, "JsonRpc.Message.OnBackToOrigin", "");
        UE_DEFINE_GAMEPLAY_TAG_COMMENT(OnScanResponseReceived, "JsonRpc.Message.onScanResponseReceived", "");
        UE_DEFINE_GAMEPLAY_TAG_COMMENT(OnGeoPinsReceived, "JsonRpc.Message.onGeoPinsReceived", "");
        
        UE_DEFINE_GAMEPLAY_TAG_COMMENT(ShowScreenTouchInfo, "HUD.Event.Visualization.ScreenTouch", "");
}
