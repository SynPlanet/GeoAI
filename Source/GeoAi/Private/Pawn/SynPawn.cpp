
#include "Pawn/SynPawn.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/SynCartographicTransformationActorComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/GameplayMessageTags.h"

#include <Kismet/KismetMathLibrary.h>

ASynPawn::ASynPawn()
{
        PrimaryActorTick.bCanEverTick = true;

        CartographicTransformationComponent = CreateDefaultSubobject<USynCartographicTransformationActorComponent>(TEXT("CartographicTransformationComponent"));
}

void ASynPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
        Super::SetupPlayerInputComponent(PlayerInputComponent);
        UEnhancedInputComponent& EnhancedInputComponent = *CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

        EnhancedInputComponent.BindAction(CartographicDragInputAction, ETriggerEvent::Started, this, &ASynPawn::User_CartographicDrag_Initialize);
        EnhancedInputComponent.BindAction(CartographicDragInputAction, ETriggerEvent::Triggered, this, &ASynPawn::User_CartographicDrag_Apply);
        EnhancedInputComponent.BindAction(CartographicDragInputAction, ETriggerEvent::Completed, this, &ASynPawn::User_CartographicDrag_Reset);

        EnhancedInputComponent.BindAction(CartographicZoomInputAction, ETriggerEvent::Started, this, &ASynPawn::User_CartographicZoom_Initialize);
        EnhancedInputComponent.BindAction(CartographicZoomInputAction, ETriggerEvent::Triggered, this, &ASynPawn::User_CartographicZoom_Apply);
        EnhancedInputComponent.BindAction(CartographicZoomInputAction, ETriggerEvent::Completed, this, &ASynPawn::User_CartographicZoom_Reset);

        // EnhancedInputComponent.BindAction(CartographicOrbitInputAction, ETriggerEvent::Started, this, &ASynPawn::User_CartographicOrbit_Initialize);
        EnhancedInputComponent.BindAction(CartographicOrbitInputAction, ETriggerEvent::Triggered, this, &ASynPawn::User_CartographicOrbit_Apply);
        EnhancedInputComponent.BindAction(CartographicOrbitInputAction, ETriggerEvent::Completed, this, &ASynPawn::User_CartographicOrbit_Reset);

        APlayerController* const PC = CastChecked<APlayerController>(GetController());

        PC->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>()->AddMappingContext(CartographicMappingContext, 5);
        PC->SetShowMouseCursor(true);
}

FHitResult ASynPawn::SweepAlongVector(const FVector& DeltaLocation, double Scale)
{
        const auto Offset = Scale * DeltaLocation;

        FHitResult HitResult;
        AddActorWorldOffset(Offset, true, &HitResult);

        return HitResult;
}

void ASynPawn::UpdateMovementAvailabilityState(bool bIsAvailable)
{
        bMovementAvailable = bIsAvailable;
}

void ASynPawn::SendTouchScreenVisualizationMessage(APlayerController* const PC)
{
        if (!FingerPositions.IsSet() || !IsValid(PC))
        {
                return;
        }

        FVisualizationScreenTouchInfo NewMessage;
        NewMessage.InstigatingController = PC;
        NewMessage.ScreenPosition = FVector2D((FingerPositions->Key.X + FingerPositions->Value.X) / 2, (FingerPositions->Key.Y + FingerPositions->Value.Y) / 2);

        GetGameInstance()->GetSubsystem<UGameplayMessageSubsystem>()->BroadcastMessage(GameplayTagsRouter::ShowScreenTouchInfo, MoveTemp(NewMessage));
}
void ASynPawn::User_CartographicDrag_Initialize()
{
        check(IsValid(CartographicTransformationComponent));

        FingerPositions = RetrieveFingerPositions();

        auto* const PC = CastChecked<APlayerController>(GetController());

        SendTouchScreenVisualizationMessage(PC);

        bool bTwoFingersPressed;
        FVector2f _;
        PC->GetInputTouchState(ETouchIndex::Touch2, _.X, _.Y, bTwoFingersPressed);
        if (bTwoFingersPressed)
        {
                return;
        }

        CartographicTransformationComponent->InitializeCartographicPivot();
}

void ASynPawn::User_CartographicZoom_Initialize()
{
        FingerPositions = RetrieveFingerPositions();

        check(IsValid(CartographicTransformationComponent));

        CartographicTransformationComponent->InitializeCartographicPivot();

        if (APlayerController* const LocalViewPC = GetLocalViewingPlayerController())
        {
                SendTouchScreenVisualizationMessage(LocalViewPC);

                int32 ViewportSizeX, ViewportSizeY;
                LocalViewPC->GetViewportSize(ViewportSizeX, ViewportSizeY);

                ToleranceFromViewportSize = UKismetMathLibrary::Min(ViewportSizeY * CartographicOrbitFingerOffsetTolerance / 100.f,
                                                                    ViewportSizeX * CartographicOrbitFingerOffsetTolerance / 100.f);
        }
}

void ASynPawn::User_CartographicOrbit_Initialize()
{
        check(IsValid(CartographicTransformationComponent));

        FingerPositions = RetrieveFingerPositions();
        CartographicTransformationComponent->InitializeCartographicPivot();

        SendTouchScreenVisualizationMessage(CastChecked<APlayerController>(GetController()));
}

void ASynPawn::User_CartographicDrag_Apply()
{
        if (!bMovementAvailable)
        {
                return;
        }

        check(IsValid(CartographicTransformationComponent));

        bool bTwoFingersPressed;
        FVector2f _;
        CastChecked<APlayerController>(GetController())->GetInputTouchState(ETouchIndex::Touch2, _.X, _.Y, bTwoFingersPressed);
        if (bTwoFingersPressed)
        {
                CartographicTransformationComponent->SetCartographicDragLocked(true);
                return;
        }

        CartographicTransformationComponent->ApplyCartographicDrag(GetWorld()->GetDeltaSeconds(), bIsForwardMoving);
}

void ASynPawn::User_CartographicOrbit_Apply(const FInputActionValue& InputActionValue)
{
        check(IsValid(CartographicTransformationComponent));

        SendTouchScreenVisualizationMessage(CastChecked<APlayerController>(GetController()));

        if (bCartographicOrbitVerticalRotationActive && FingerPositions.IsSet())
        {
                const TPair<FVector2f, FVector2f>& LastFingerPositions = FingerPositions.GetValue();
                const TPair<FVector2f, FVector2f> CurrentFingerPositions = RetrieveFingerPositions();

                const FVector2f Finger1Offset = CurrentFingerPositions.Key - LastFingerPositions.Key;
                const FVector2f Finger2Offset = CurrentFingerPositions.Value - LastFingerPositions.Value;

                if (!Finger1Offset.IsNearlyZero(ToleranceFromViewportSize) && !Finger2Offset.IsNearlyZero(ToleranceFromViewportSize))
                {
                        const float OffsetY = FMath::Min(Finger1Offset.Y, Finger2Offset.Y);

                        const bool bIsOffsetVerySmall = FMath::IsNearlyZero(OffsetY, ToleranceFromViewportSize);
                        const bool bIsFinger1HorizontalOffsetVerySmall = FMath::IsNearlyZero(Finger1Offset.X, ToleranceFromViewportSize);
                        const bool bIsFinger2HorizontalOffsetVerySmall = FMath::IsNearlyZero(Finger2Offset.X, ToleranceFromViewportSize);

                        if (bCartographicOrbitVerticalRotationActive.GetValue()
                            || (!bIsOffsetVerySmall && bIsFinger1HorizontalOffsetVerySmall && bIsFinger2HorizontalOffsetVerySmall))
                        {
                                CartographicTransformationComponent->ApplyCartographicOrbit(
                                        { -OffsetY * CartographicOrbitVerticalRotationSensitivity, 0.0, 0.0 });

                                bCartographicOrbitVerticalRotationActive = true;
                                return;
                        }
                }
        }

        if (bCartographicOrbitVerticalRotationActive.Get(false))
        {
                return;
        }

        const float Angle = InputActionValue.Get<FVector2d>().X;
        if (FMath::IsNearlyZero(Angle, 0.5))
        {
                return;
        }

        CartographicTransformationComponent->ApplyCartographicOrbit({ 0.0, -Angle, 0.0 });
}

void ASynPawn::User_CartographicZoom_Apply(const FInputActionValue& InputActionValue)
{
        if (bCartographicOrbitVerticalRotationActive.IsSet())
        {
                if (bCartographicOrbitVerticalRotationActive.GetValue())
                {
                        return;
                }
        }

        float Scale = InputActionValue.Get<float>();
        Scale = Scale <= 1 ? Scale - 1 : Scale / 15;

        if (FMath::IsNearlyZero(Scale, 0.03f))
        {
                return;
        }

        check(IsValid(CartographicTransformationComponent));

        if (!bCartographicZoomActive.IsSet())
        {
                return;
        }

        bCartographicZoomActive = true;

        CartographicTransformationComponent->ApplyCartographicZoom(Scale);
}

void ASynPawn::User_CartographicDrag_Reset()
{
        check(IsValid(CartographicTransformationComponent));
        CartographicTransformationComponent->ResetCartographicPivot();
}

void ASynPawn::User_CartographicOrbit_Reset()
{
        check(IsValid(CartographicTransformationComponent));
        CartographicTransformationComponent->ResetCartographicPivot();

        FingerPositions.Reset();

        bCartographicOrbitVerticalRotationActive = false;
}

void ASynPawn::User_CartographicZoom_Reset()
{
        check(IsValid(CartographicTransformationComponent));
        CartographicTransformationComponent->ResetCartographicPivot();
        FingerPositions.Reset();

        bCartographicZoomActive = false;

        if (bZoomBlock)
        {
                InteractableActor = nullptr;
                bZoomBlock = false;
        }
}
void ASynPawn::ApplyCartographicZoom2x()
{
        User_CartographicZoom_Initialize();

        check(IsValid(CartographicTransformationComponent));

        if (bCartographicZoomActive.IsSet())
        {
                if (bCartographicZoomActive.GetValue())
                {
                        UE_LOG(LogTemp, Log, TEXT("ApplyCartographicZoom2x(): common zoom active, cancel 2x zoom"));
                        return;
                }
        }

        CartographicTransformationComponent->ApplyCartographic2xZoom();
}

TPair<FVector2f, FVector2f> ASynPawn::RetrieveFingerPositions() const
{
        TPair<FVector2f, FVector2f> Fingers;
        const APlayerController* const PC = CastChecked<APlayerController>(GetController());

        bool First, Second;
        PC->GetInputTouchState(ETouchIndex::Touch1, Fingers.Key.X, Fingers.Key.Y, First);
        PC->GetInputTouchState(ETouchIndex::Touch2, Fingers.Value.X, Fingers.Value.Y, Second);

        if (First && !Second)
        {
                Fingers.Value.X = Fingers.Key.X;
                Fingers.Value.Y = Fingers.Key.Y;
        }

#if WITH_EDITOR
        if (!First && !Second)
        {
                PC->GetMousePosition(Fingers.Key.X, Fingers.Key.Y);
        }
#endif

        return Fingers;
}
