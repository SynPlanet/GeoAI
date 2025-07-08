#include "Components/CameraTranslationComponent.h"

#include "CesiumGeoreference.h"
#include "GeographicalContext.h"
#include "System/GeoAiUtilitiesLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogCameraTranslationComponent, Log, All);

UCameraTranslationComponent::UCameraTranslationComponent()
{
        PrimaryComponentTick.bCanEverTick = true;
        PrimaryComponentTick.bStartWithTickEnabled = false;
        PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UCameraTranslationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
        Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
        
        if (!TargetPawn.IsValid())
        {
                return;
        }

        auto* PC = Cast<APlayerController>(GetOwner());
        
        const double Alpha = FMath::SmoothStep(0.0f, 1.0f, CurrentDuration / TotalDuration);

        if (FVector TargetLocation = FMath::Lerp(InitialTransform.GetLocation(), TargetTransform.GetLocation(), Alpha); !TargetLocation.ContainsNaN())
        {
                const float ZAdjustment = 4.0 * ElevationFactor * Alpha * (1.0 - Alpha);

                TargetLocation += ZAdjustment * UGeoAiUtilitiesLibrary::GetCesiumGeoreference(this)->ComputeEastSouthUpToUnrealTransformation(TargetLocation).GetUnitAxis(EAxis::Z);

                const FQuat TargetRotation = FQuat::Slerp(InitialTransform.GetRotation(), TargetTransform.GetRotation(), Alpha);

                TargetPawn->SetActorLocation(TargetLocation);
                PC->SetControlRotation(TargetRotation.Rotator());
        }
        
        CurrentDuration += DeltaTime;
        if (CurrentDuration >= TotalDuration)
        {
                TargetPawn->SetActorLocation(TargetTransform.GetLocation());
                PC->SetControlRotation(TargetTransform.GetRotation().Rotator());
        }
        
        if (TrySendNotifications())
        {
                SetComponentTickEnabled(false);
        }
}

bool UCameraTranslationComponent::TrySendNotifications()
{
        if (!TargetPawn.IsValid())
        {
                return false;
        }

        const FVector CurrentLocation = TargetPawn->GetActorLocation();

        const double DistanceSqr = FVector::DistSquared(TargetTransform.GetLocation(), CurrentLocation);
        const double NormalizedDistance = DistanceSqr / MaxDistanceSqr;

        if (!bNotifySend)
        {
                if (NormalizedDistance <= CalculatedDistanceTriggerSqr)
                {
                        bNotifySend = true;

                        SendNotification(true);
                }

                return false;
        }

        if (NormalizedDistance > KINDA_SMALL_NUMBER)
        {
                return false;
        }

        SendNotification(false);

        bNotifySend = false;

        return true;
}

void UCameraTranslationComponent::SetTargetLocation(const FVector& NewTargetPosition)
{
        TargetTransform.SetLocation(NewTargetPosition);

        auto* OwnerAsPC =Cast<APlayerController>(GetOwner());
        check(OwnerAsPC);
        
        APawn* Pawn = OwnerAsPC->GetPawn();
        if (!IsValid(Pawn))
        {
                UE_LOG(LogCameraTranslationComponent, Error, TEXT("%s: can't set translation info without pawn"), *GetNameSafe(OwnerAsPC));
                return;
        }

        TargetPawn = Pawn;

        
        FVector CurrentLocation = TargetPawn->GetActorLocation();
        FVector Direction = (NewTargetPosition - CurrentLocation).GetSafeNormal();
        
        FRotator LookAtRotation = Direction.Rotation();
        
        TargetTransform.SetRotation(FQuat(LookAtRotation));

        TryEnableCameraTranslation();
}

void UCameraTranslationComponent::SetTargetTransform(const FTransform& NewTargetTransform)
{
        TargetTransform = NewTargetTransform;

        TryEnableCameraTranslation();
}

void UCameraTranslationComponent::UpdateRequestInfo(const FGeographicalLocation& NewTargetPosition, const FGuid& NewTranslationId)
{
        TargetGeoPosition = NewTargetPosition;

        CurrentTranslationId = NewTranslationId;

        // TODO: Here we can send notification abort previous translation
}

void UCameraTranslationComponent::SendNotification(const bool bIsUpClose) const
{
        UE_LOG(LogCameraTranslationComponent, Log, TEXT("%hs"), bIsUpClose ? "Camera close up to point" : "Camera arrived to point");
}

bool UCameraTranslationComponent::TryEnableCameraTranslation()
{
        auto* OwnerAsPC =Cast<APlayerController>(GetOwner());
        check(OwnerAsPC);
        
        APawn* Pawn = OwnerAsPC->GetPawn();
        if (!IsValid(Pawn))
        {
                UE_LOG(LogCameraTranslationComponent, Error, TEXT("%s: can't start translation without pawn"), *GetNameSafe(OwnerAsPC));
                return false;
        }

        TargetPawn = Pawn;

        InitialTransform.SetLocation(TargetPawn->GetActorLocation());
        InitialTransform.SetRotation(FQuat(TargetPawn->GetControlRotation()));
        
        const double MaxDistance = FVector::Distance(TargetTransform.GetLocation(), TargetPawn->GetActorLocation());
        check(MaxDistance > 0.f);

        if (MaxDistance <= CalculatedMinApproachNotificationThreshold)
        {
                bNotifySend = true;

                SendNotification(true);
        }

        MaxDistanceSqr = FMath::Square(MaxDistance);

        TotalDuration = FMath::GetMappedRangeValueClamped(InRange, OutRangeDuration, MaxDistance);
        ElevationFactor = FMath::GetMappedRangeValueClamped(InRange, OutRangeElevation, MaxDistance) * MaxDistance;
        CurrentDuration = 0.0f;
        
        Pawn->DisableInput(OwnerAsPC);
        
        SetComponentTickEnabled(true);

        return true;
}
