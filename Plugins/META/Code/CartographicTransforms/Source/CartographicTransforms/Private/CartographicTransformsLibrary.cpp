#include "CartographicTransformsLibrary.h"

DEFINE_LOG_CATEGORY(LogCartographicTransforms);

FVector UCartographicTransformsLibrary::Drag(const FVector& Target, const FVector& Pivot, const FVector& PivotDestination)
{
        return Target + PivotDestination - Pivot;
}

FVector UCartographicTransformsLibrary::Zoom(const FVector& Target, const FVector& Pivot, double Scale)
{
        return Target * (1.0 - Scale) + Pivot * Scale;
}

FVector UCartographicTransformsLibrary::Orbit(const FVector& Target, const FVector& Pivot, const FRotator& AngularVelocity, const FVector& TargetDirection)
{
        FQuat const YawRotation(FVector::UpVector, FMath::DegreesToRadians(AngularVelocity.Yaw));
        FQuat const PitchRotation(FVector::UpVector.Cross(TargetDirection).GetSafeNormal(), FMath::DegreesToRadians(-AngularVelocity.Pitch));

        return Pivot + (YawRotation * PitchRotation).RotateVector(Target - Pivot);
}
