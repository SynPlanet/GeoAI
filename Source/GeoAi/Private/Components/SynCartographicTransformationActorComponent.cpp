#include "Components/SynCartographicTransformationActorComponent.h"

#include "CartographicTransformsLibrary.h"
#include "Math/SynShapeIntersectionLibrary.h"
#include "Pawn/SynPawn.h"

#include <Kismet/KismetMathLibrary.h>

DEFINE_LOG_CATEGORY(LogSynCartographicTransformationActorComponent);

USynCartographicTransformationActorComponent::USynCartographicTransformationActorComponent()
{
        PrimaryComponentTick.bCanEverTick = true;
}

void USynCartographicTransformationActorComponent::BeginPlay()
{
        Super::BeginPlay();

        ResetTargetRotation();
}

void USynCartographicTransformationActorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
        Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

        /*if (!bSmoothRotation)
        {
                return;
        }

        const APawn* const Owner = CastChecked<ASynPawn>(GetOwner());
        APlayerController* const Controller = CastChecked<APlayerController>(Owner->GetController());
        const FRotator OldControllerRotation = Controller->GetControlRotation();

        const FRotator CurrentTargetRotation = OldControllerRotation + TargetRotation;
        CurrentRotation = UKismetMathLibrary::RInterpTo(OldControllerRotation, CurrentTargetRotation, DeltaTime, InterpSpeed);

        TargetRotation = UKismetMathLibrary::RInterpTo(TargetRotation, FRotator::ZeroRotator, DeltaTime, InterpSpeed);

        CurrentRotation.Roll = 0.f;
        CurrentRotation.Pitch = UKismetMathLibrary::FClamp(CurrentRotation.Pitch, -89.9f, 0.f);

        if (CurrentRotation != OldControllerRotation)
        {
                Controller->SetControlRotation(CurrentRotation);
        }*/
}

bool USynCartographicTransformationActorComponent::InitializeCartographicPivot()
{
        ASynPawn const* const Owner = CastChecked<ASynPawn>(GetOwner());
        APlayerController const* const Controller = CastChecked<APlayerController>(Owner->GetController());

        FVector2f ScreenLocation;

        bool bPressed;

        Controller->GetInputTouchState(ETouchIndex::Touch1, ScreenLocation.X, ScreenLocation.Y, bPressed);

        // check(bPressed);

        FVector2f SecondFingerPosition;
        Controller->GetInputTouchState(ETouchIndex::Touch2, SecondFingerPosition.X, SecondFingerPosition.Y, bPressed);

        if (bPressed)
        {
                ScreenLocation /= 2.0f;
                ScreenLocation += SecondFingerPosition / 2.0f;
        }

#if WITH_EDITOR
        if (!bPressed)
        {
                Controller->GetMousePosition(ScreenLocation.X, ScreenLocation.Y);

                ScreenLocation /= 2.0f;
                ScreenLocation += SecondFingerPosition / 2.0f;
        }
#endif

        FVector WorldLocation, WorldDirection;
        if (!Controller->DeprojectScreenPositionToWorld(ScreenLocation.X, ScreenLocation.Y, WorldLocation, WorldDirection))
        {
                SetCartographicDragLocked(true);

                return false;
        }

        FHitResult Hit;
        if (!GetWorld()->LineTraceSingleByChannel(Hit, WorldLocation, WorldDirection * CartographicPivotTraceDistance, ECC_Visibility))
        {
                SetCartographicDragLocked(true);

                return false;
        }

        CartographicPivot = Hit.Location;
        PreviousDesiredPivotDestination = Hit.Location;

        return true;
}

void USynCartographicTransformationActorComponent::ResetCartographicPivot()
{
        CartographicPivot.Reset();
        CartographicZoomPreviousDeltaScale.Reset();
        CartographicOrbitPreviousAngularVelocity.Reset();
        ProjectiveCartographicDragSurfacePlane.Reset();
        bCartographicDragLocked = false;
}

void USynCartographicTransformationActorComponent::ApplyCartographicDrag(float DeltaSeconds, bool bForward)
{
        if (bCartographicDragLocked || !CartographicPivot)
        {
                return;
        }

        DeltaSeconds = FMath::Min(CartographicDragMaxDeltaSeconds, DeltaSeconds);

        auto const LockCartographicDrag = [this] { SetCartographicDragLocked(true); };

        ASynPawn* const Owner = CastChecked<ASynPawn>(GetOwner());
        APlayerController* const Controller = CastChecked<APlayerController>(Owner->GetController());

        FRotator const ControlRotation = Controller->GetControlRotation();
        FVector const Target = Owner->GetActorLocation();

        FVector const Pivot = CartographicPivot.GetValue();
        FVector const PivotSurfaceNormal = FVector::UpVector;
        FVector const OrthogonalNormal = PivotSurfaceNormal.Cross(ControlRotation.RotateVector(FVector::RightVector));

        double const ProjectivePlaneAngleCosine = (Pivot - Target).GetSafeNormal().Dot(PivotSurfaceNormal);

        if (!ProjectiveCartographicDragSurfacePlane)
        {
                bPerformOrthogonalDrag = FMath::IsNearlyZero(ProjectivePlaneAngleCosine, ProjectiveOrthogonalCartographicDragAngleCosine);
                ProjectiveCartographicDragSurfacePlane = { Pivot, bPerformOrthogonalDrag ? OrthogonalNormal : PivotSurfaceNormal };
        }

        FVector2f ScreenLocation;
        if (!Controller->GetMousePosition(ScreenLocation.X, ScreenLocation.Y))
        {
                return LockCartographicDrag();
        }

        FVector WorldLocation, WorldDirection;
        if (!Controller->DeprojectScreenPositionToWorld(ScreenLocation.X, ScreenLocation.Y, WorldLocation, WorldDirection))
        {
                return LockCartographicDrag();
        }

        TOptional<FVector> const MaybeDesiredPivotDestination = USynShapeIntersectionLibrary::CalculateRayPlaneIntersection(
                { WorldLocation, WorldDirection }, ProjectiveCartographicDragSurfacePlane.GetValue());

        if (!MaybeDesiredPivotDestination)
        {
                return LockCartographicDrag();
        }

        FVector const DesiredPivotDestination = MaybeDesiredPivotDestination.GetValue();

        FVector const TargetPivotDelta = Target - Pivot;
        double const TargetPivotDistance = TargetPivotDelta.Length();

        double const SurfaceAngleCosine = FMath::IsNearlyZero(TargetPivotDistance) ? 0.0 : PivotSurfaceNormal.Dot(TargetPivotDelta / TargetPivotDistance);
        double const TargetPivotDestinationDistance = FVector::Dist(Target, DesiredPivotDestination);

        if (!bPerformOrthogonalDrag && TargetPivotDestinationDistance > TargetPivotDistance * SurfaceAngleCosine * CartographicDragPivotDistanceMultiplier)
        {
                return LockCartographicDrag();
        }

        double const RelativeDistanceToPivot = TargetPivotDistance / CartographicDragMaxPivotDistance;
        double const InterpSpeedScale = 1.0 + FMath::Min(1.0, RelativeDistanceToPivot) * (CartographicDragMaxRelativeDistanceToPivot - 1.0);

        FVector const FinalPivotDestination =
                FMath::VInterpTo(PreviousDesiredPivotDestination,
                                 DesiredPivotDestination,
                                 bPerformOrthogonalDrag ? FMath::Min(DeltaSeconds, CartographicOrthogonalDragMaxDeltaSeconds) : DeltaSeconds,
                                 CartographicDragInterpSpeed * InterpSpeedScale);

        FVector const DesiredDragOffset = Pivot - FinalPivotDestination;

        if (DesiredDragOffset.IsNearlyZero())
        {
                return;
        }

        if (!bPerformOrthogonalDrag)
        {
                Owner->SweepAlongVector(bForward ? DesiredDragOffset : DesiredDragOffset * -0.01f);
        }
        else
        {
                double const ScaleRelativeDistanceToPivot = 1.0 + RelativeDistanceToPivot * (CartographicOrthogonalDragRelativeDistanceScale - 1.0);

                FVector const RotatedDragOffset = FQuat::FindBetweenNormals(OrthogonalNormal, FVector::UpVector).RotateVector(DesiredDragOffset);

                PreviousDesiredPivotDestination = FinalPivotDestination;

                if (RotatedDragOffset.Dot(std::exchange(LastCartographicDragOrthogonalOffset, RotatedDragOffset)) < 0)
                {
                        return;
                }

                Owner->SweepAlongVector(CartographicDragOrthogonalPullMultiplier / ScaleRelativeDistanceToPivot * RotatedDragOffset);
        }
}

void USynCartographicTransformationActorComponent::SetCartographicDragLocked(bool bLocked)
{
        if (bCartographicDragLocked = bLocked; bLocked)
        {
                OnCartographicDragEnd.Broadcast();
        }
}

void USynCartographicTransformationActorComponent::ApplyCartographicOrbit(FRotator const& DesiredAngularVelocity)
{
        if (!CartographicPivot.IsSet())
        {
                return;
        }

        ASynPawn* const Owner = CastChecked<ASynPawn>(GetOwner());
        APlayerController* const Controller = CastChecked<APlayerController>(Owner->GetController());
        FRotator const ControlRotation = Controller->GetControlRotation();

        FVector const Target = GetOwner()->GetActorLocation();
        FVector const Pivot = CartographicPivot.GetValue();
        FVector const Direction = ControlRotation.Vector();

        if (!CartographicPivot.IsSet())
        {
                return;
        }

        FRotator const FinalAngularVelocity = DesiredAngularVelocity - CartographicOrbitPreviousAngularVelocity.Get(DesiredAngularVelocity);
        CartographicOrbitPreviousAngularVelocity = DesiredAngularVelocity;

        if (FinalAngularVelocity.IsNearlyZero())
        {
                return;
        }

        FVector const Offset = UCartographicTransformsLibrary::Orbit(Target, Pivot, FinalAngularVelocity, Direction) - Target;

        if (FHitResult const Hit = Owner->SweepAlongVector(Offset); Hit.bBlockingHit && Hit.bStartPenetrating)
        {
                return;
        }

        // if (bSmoothRotation)
        //{
        //         TargetRotation = TargetRotation + FinalAngularVelocity;
        // }
        // else
        //{
        FRotator CalculatedRotation = ControlRotation + FinalAngularVelocity;
        CalculatedRotation.Pitch = UKismetMathLibrary::FClamp(CalculatedRotation.Pitch, -89.9f, 0.f);

        Controller->SetControlRotation(CalculatedRotation);
        //}
}

void USynCartographicTransformationActorComponent::ApplyCartographicZoom(double DesiredDeltaScale)
{
        if (!DesiredDeltaScale || !CartographicPivot.IsSet())
        {
                return;
        }

        double const FinalDeltaScale = DesiredDeltaScale - CartographicZoomPreviousDeltaScale.Get(DesiredDeltaScale);
        CartographicZoomPreviousDeltaScale = DesiredDeltaScale;

        if (FMath::IsNearlyZero(FinalDeltaScale))
        {
                return;
        }

        ASynPawn* const Owner = CastChecked<ASynPawn>(GetOwner());

        FVector const Target = Owner->GetActorLocation();
        FVector const Pivot = CartographicPivot.GetValue();

        FVector const Offset = UCartographicTransformsLibrary::Zoom(Target, Pivot, FinalDeltaScale * CartographicZoomScaleMultiplier) - Target;

        Owner->SweepAlongVector(Offset);
}

void USynCartographicTransformationActorComponent::ApplyCartographic2xZoom()
{
        if (!CartographicPivot.IsSet())
        {
                UE_LOG(LogSynCartographicTransformationActorComponent, VeryVerbose, TEXT("ApplyCartographic2xZoom(): Cartographic Pivot doesn't set"));
                return;
        }

        ASynPawn* const Owner = CastChecked<ASynPawn>(GetOwner());

        FVector const Target = Owner->GetActorLocation();
        FVector const Pivot = CartographicPivot.GetValue();

        FVector const Offset = UCartographicTransformsLibrary::Zoom(Target, Pivot, 0.5) - Target;

        UE_LOG(LogSynCartographicTransformationActorComponent,
               VeryVerbose,
               TEXT("ApplyCartographic2xZoom(): Sweep start with offset %f, %f, %f"),
               Offset.X,
               Offset.Y,
               Offset.Z);
        Owner->SweepAlongVector(Offset);
}

void USynCartographicTransformationActorComponent::ResetTargetRotation()
{
        const APawn* const Owner = CastChecked<APawn>(GetOwner());

        CurrentRotation = Owner->GetControlRotation();
        TargetRotation = FRotator::ZeroRotator;
}
