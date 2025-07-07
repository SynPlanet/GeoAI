#include "Components/GeographicalSurfaceAnchor.h"

#include "GeographicalContext.h"
#include "GeographicalTransforms.h"

UGeographicalSurfaceAnchor::UGeographicalSurfaceAnchor()
{
        PrimaryComponentTick.bCanEverTick = true;
}

void UGeographicalSurfaceAnchor::BeginPlay()
{
        Super::BeginPlay();
        ApplyDefaultTransform();
}

void UGeographicalSurfaceAnchor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
        Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
        if (!bAlignWithSurfaceNormalOnTick)
        {
                return;
        }

        AlignWithSurfaceNormal();
}

void UGeographicalSurfaceAnchor::AlignWithSurfaceNormal() const
{
        if (!UGeographicalContext::IsInitialized())
        {
                return;
        }

        const auto Owner = GetOwner();
        check(IsValid(Owner));

        const auto SurfaceNormalRotation = UGeographicalContext::CalculateSurfaceNormalRotation(Owner->GetActorLocation());
        Owner->SetActorRotation(DefaultLocalRotation.Quaternion() * SurfaceNormalRotation);
}

void UGeographicalSurfaceAnchor::ResetDefaultCoordinates()
{
        if (!UGeographicalContext::IsInitialized())
        {
                return;
        }

        const auto Owner = GetOwner();
        check(IsValid(Owner));

        DefaultCoordinates = UGeographicalTransforms::UnrealToWgs84(Owner->GetActorLocation(), Owner);
}

void UGeographicalSurfaceAnchor::TranslateToDefaultCoordinates() const
{
        if (!UGeographicalContext::IsInitialized())
        {
                return;
        }

        const auto Owner = GetOwner();
        check(IsValid(Owner));

        Owner->SetActorLocation(UGeographicalTransforms::Wgs84ToUnreal(DefaultCoordinates, Owner));
}

void UGeographicalSurfaceAnchor::ApplyDefaultTransform() const
{
        TranslateToDefaultCoordinates();
        AlignWithSurfaceNormal();
}
