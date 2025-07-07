// META all rights received (c)

#include "PromptRulerPopupWidget.h"

void UPromptRulerPopupWidget::SetTextPopup(FText Text)
{
        if (!IsValid(PopupText))
        {
                return;
        }
        PopupText->SetText(MoveTemp(Text));
}