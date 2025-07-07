// META all rights received (c)

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "PromptRulerPopupWidget.generated.h"

/**
 *
 */
UCLASS()
class METAMEASUREMENT_API UPromptRulerPopupWidget : public UUserWidget
{
        GENERATED_BODY()
public:
        /**
        * Sets the text of the popup widget.
        * Updates the display text in the popup widget with the specified text.
        *
        * @param Text The text to be set in the popup widget.
        */
        UFUNCTION(BlueprintCallable)
        void SetTextPopup(FText Text);

protected:
        UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (BindWidget))
        TObjectPtr<UTextBlock> PopupText;
};
