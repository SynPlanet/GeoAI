// Fill out your copyright notice in the Description page of Project Settings.

#include "QuadMeasurement.h"

#include "CartographicTransformsLibrary.h"

#include <Kismet/KismetMathLibrary.h>

#if WITH_EDITOR
#include "Kismet/KismetSystemLibrary.h"
#endif

// Sets default values
AQuadMeasurement::AQuadMeasurement()
{
        // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
        PrimaryActorTick.bCanEverTick = true;
        bAllowTickBeforeBeginPlay = false;

        bMoveAvailable = bMoveAlongPlayerViewpoint;
}

// Called when the game starts or when spawned
void AQuadMeasurement::BeginPlay()
{
        Super::BeginPlay();

        if (bUpdateSizeOnBeginPlay)
        {
                UpdateSize();
        }
}

void AQuadMeasurement::InterpActorScale(float DeltaTime)
{
        if (CurrentScale != TargetScale)
        {
                CurrentScale = UKismetMathLibrary::FInterpTo(CurrentScale, TargetScale, DeltaTime, ScalingSpeed);

                OnScaleUpdated(CurrentScale);

                SetActorScale3D(FVector(CurrentScale));
        }
}
// Called every frame
void AQuadMeasurement::Tick(float DeltaTime)
{
        Super::Tick(DeltaTime);

        if (bMoveAvailable)
        {
                UpdateLocationFromPlayerViewPoint();
        }

        InterpActorScale(DeltaTime);
}

void AQuadMeasurement::UpdateSize_Implementation(FVector2D OriginRealSize)
{
        // Multiply 1000 cm -> km, divide 2 half size (box/radius)
        CalculatedLimit = FVector2D(SizeLimit.X * 1000 / OriginRealSize.X / 2, SizeLimit.Y * 1000 / OriginRealSize.Y / 2);

        UE_LOG(LogTemp, VeryVerbose, TEXT("Calculated Limits %f, %f"), CalculatedLimit.X, CalculatedLimit.Y);

        const auto PC = Cast<APlayerController>(GetOwner());
        if (!PC)
        {
                return;
        }

        int32 ViewportX, ViewportY;
        PC->GetViewportSize(ViewportX, ViewportY);

        FCollisionQueryParams NewParams;
        NewParams.AddIgnoredActor(GetInstigator());
        NewParams.bTraceComplex = false;

        FVector WorldLocation;
        FRotator Rotation;
        PC->GetPlayerViewPoint(WorldLocation, Rotation);

        if (!UpdateLocationFromPlayerView(NewParams, WorldLocation, Rotation.Vector()))
        {
                return;
        }

        const int32 MinViewportSize = FMath::Min(ViewportX, ViewportY);

        const float Dot = FMath::Abs(FVector::DotProduct(PC->GetPawn()->GetActorForwardVector(), PC->GetControlRotation().Vector()));
        const float YawModifire = 1 - Dot;

        const int32 CentreX = ViewportX / 2;
        const int32 CentreY = ViewportY / 2;

        FVector WorldNorth, WorldSouth, WorldEast, WorldWest;
        FVector NewWorldDirection;
        int32 NY = CentreY - MinViewportSize / 2 * UsableScreenArea * YawModifire;
        int32 SY = CentreY + MinViewportSize / 2 * UsableScreenArea * YawModifire;
        int32 EX = CentreX + MinViewportSize / 2 * UsableScreenArea;
        int32 WX = CentreX - MinViewportSize / 2 * UsableScreenArea;
        FHitResult HitNY, HitSy, HitEx, HitWX;

        PC->DeprojectScreenPositionToWorld(CentreX, NY, WorldNorth, NewWorldDirection);
        GetWorld()->LineTraceSingleByChannel(HitNY, WorldNorth, WorldNorth + NewWorldDirection * FLT_MAX, ECC_Visibility, NewParams);
        PC->DeprojectScreenPositionToWorld(CentreX, SY, WorldSouth, NewWorldDirection);
        GetWorld()->LineTraceSingleByChannel(HitSy, WorldSouth, WorldSouth + NewWorldDirection * FLT_MAX, ECC_Visibility, NewParams);
        PC->DeprojectScreenPositionToWorld(EX, CentreY, WorldEast, NewWorldDirection);
        GetWorld()->LineTraceSingleByChannel(HitEx, WorldEast, WorldEast + NewWorldDirection * FLT_MAX, ECC_Visibility, NewParams);
        PC->DeprojectScreenPositionToWorld(WX, CentreY, WorldWest, NewWorldDirection);
        GetWorld()->LineTraceSingleByChannel(HitWX, WorldWest, WorldWest + NewWorldDirection * FLT_MAX, ECC_Visibility, NewParams);

        QuadAreaInfo.ScreenPoints = { FVector2D(CentreX, NY), FVector2D(CentreX, SY), FVector2D(EX, CentreY), FVector2D(WX, CentreY) };
        QuadAreaInfo.WorldPoints = { HitNY.Location, HitSy.Location, HitEx.Location, HitWX.Location };

        const float EWDistance = FVector::Distance(HitEx.Location, HitWX.Location);
        const float NSDistance = FVector::Distance(HitNY.Location, HitSy.Location);

        const float TargetScreenScale = UKismetMathLibrary::Min(EWDistance, NSDistance) / UKismetMathLibrary::Min(OriginRealSize.X, OriginRealSize.Y);

        TargetScale = UKismetMathLibrary::Clamp(TargetScreenScale, CalculatedLimit.X, CalculatedLimit.Y);

#if WITH_EDITOR
        if (bDrawDebug)
        {
                UKismetSystemLibrary::DrawDebugPoint(this, WorldNorth, 50.f, FLinearColor::Blue, 10.f);
                UKismetSystemLibrary::DrawDebugPoint(this, WorldSouth, 50.f, FLinearColor::Red, 10.f);
                UKismetSystemLibrary::DrawDebugPoint(this, WorldEast, 50.f, FLinearColor::Green, 10.f);
                UKismetSystemLibrary::DrawDebugPoint(this, WorldWest, 50.f, FLinearColor::Yellow, 10.f);

                UKismetSystemLibrary::DrawDebugLine(this, WorldNorth, WorldEast, FLinearColor::Black, 10.f);
                UKismetSystemLibrary::DrawDebugLine(this, WorldNorth, WorldWest, FLinearColor::Black, 10.f);
                UKismetSystemLibrary::DrawDebugLine(this, WorldSouth, WorldEast, FLinearColor::Black, 10.f);
                UKismetSystemLibrary::DrawDebugLine(this, WorldSouth, WorldWest, FLinearColor::Black, 10.f);
        }
#endif
}

float AQuadMeasurement::GetCurrentNormalizedAreaSizeScale() const
{
        return TargetScale / CalculatedLimit.Y;
}

void AQuadMeasurement::UpdateMovementAvailabilityState(bool bIsAvailable)
{
        bMoveAvailable = bIsAvailable;
}

void AQuadMeasurement::UpdateLocationFromPlayerViewPoint()
{
        const auto* const PC = Cast<APlayerController>(GetOwner());
        if (!IsValid(PC))
        {
                return;
        }

        int32 ViewportX, ViewportY;
        PC->GetViewportSize(ViewportX, ViewportY);

        FCollisionQueryParams NewParams;
        NewParams.AddIgnoredActor(GetInstigator());
        NewParams.bTraceComplex = false;

        FVector WorldLocation;

        FVector Direction;
        if (bMoveAlongPlayerViewpoint)
        {
                FRotator Rotation;
                PC->GetPlayerViewPoint(WorldLocation, Rotation);
                Direction = Rotation.Vector();
        }
        else
        {
                bool IsPressed = false;
                double X, Y;
                if (IsPressed)
                {
                        PC->GetInputTouchState(ETouchIndex::Touch1, X, Y, IsPressed);
                }
                else
                {
                        PC->GetMousePosition(X, Y);
                }

                PC->DeprojectScreenPositionToWorld(X, Y, WorldLocation, Direction);
        }

        UpdateLocationFromPlayerView(NewParams, WorldLocation, Direction);
}

bool AQuadMeasurement::UpdateLocationFromPlayerView(const FCollisionQueryParams& NewParams, const FVector& WorldLocation, const FVector& WorldDirection)
{
        FHitResult Hit;

        GetWorld()->LineTraceSingleByChannel(Hit, WorldLocation, WorldLocation + WorldDirection * FLT_MAX, ECC_Visibility, NewParams);

        if (!Hit.bBlockingHit)
        {
                const FVector PawnLocation = GetInstigator()->GetActorLocation();
                GetWorld()->LineTraceSingleByChannel(Hit, PawnLocation, PawnLocation + FVector::DownVector * FLT_MAX, ECC_Visibility, NewParams);

                if (!Hit.bBlockingHit)
                {
                        UE_LOG(LogTemp, Error, TEXT("Cant correct place quad area"));
                        return false;
                }
        }

#if WITH_EDITOR
        if (bDrawDebug)
        {
                UKismetSystemLibrary::DrawDebugPoint(this, Hit.Location, 50.f, FLinearColor::Red);
        }
#endif
        SetActorLocation(Hit.Location);

        return true;
}

void AQuadMeasurement::ChangeScale(float NewScale)
{
        TargetScale = UKismetMathLibrary::MapRangeClamped(NewScale, 0.f, 1.f, CalculatedLimit.X, CalculatedLimit.Y);
}

FQuadAreaInfo AQuadMeasurement::GetMeasurementInfo()
{
        return QuadAreaInfo;
}
