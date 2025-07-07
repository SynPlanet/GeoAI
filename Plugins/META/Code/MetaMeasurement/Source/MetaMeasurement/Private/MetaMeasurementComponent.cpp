// META all rights received (c)

#include "MetaMeasurementComponent.h"

#include "RulerMeasurement.h"
#include "Kismet/KismetGuidLibrary.h"
#include "QuadMeasurement.h"

ARulerMeasurement* UMetaMeasurementComponent::GetRulerMeasurement()
{
        if (!IsValid(RulerMeasurement))
        {
                RulerMeasurement = GetWorld()->SpawnActor<ARulerMeasurement>(RulerMeasurementClass);
                RulerMeasurement->SetController(Cast<APlayerController>(GetOwner()));
        }

        return RulerMeasurement;
}

bool UMetaMeasurementComponent::IsRulerExisted() const
{
        return IsValid(RulerMeasurement);
}

bool UMetaMeasurementComponent::IsQuadAreaExisted() const
{
        return IsValid(QuadMeasurement);
}
float UMetaMeasurementComponent::GetNormalizedQuadAreaScaledSize() const
{
        if (!IsQuadAreaExisted())
        {
                return 1.f;
        }

        return QuadMeasurement->GetCurrentNormalizedAreaSizeScale();
}

void UMetaMeasurementComponent::CreateRuler()
{
        const FGuid RulerGuid = UKismetGuidLibrary::NewGuid();

        CreatedRulerGuid.Add(RulerGuid);

        ARulerMeasurement* const Measurement = GetRulerMeasurement();
        if (!IsValid(Measurement) || Measurement->GetRulerMode() == ERulerMode::RulerEdit)
        {
                return;
        }

        Measurement->SetRulerMode(ERulerMode::RulerEdit);
        Measurement->CreateRuler(RulerGuid.ToString(), false, true);
}
void UMetaMeasurementComponent::CreateOrDestroyQuadArea()
{
        if (CreateQuadArea())
        {
                return;
        }

        DestroyQuadArea();
}

bool UMetaMeasurementComponent::CreateQuadArea()
{
        if (IsQuadAreaExisted())
        {
                return false;
        }

        APlayerController* const PC = Cast<APlayerController>(GetOwner());
        if (!PC)
        {
                return false;
        }

        FActorSpawnParameters NewParams;
        NewParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        NewParams.Owner = PC;
        NewParams.Instigator = PC->GetPawn();
        QuadMeasurement = GetWorld()->SpawnActor<AQuadMeasurement>(QuadMeasurementClass, NewParams);

        return IsValid(QuadMeasurement);
}
void UMetaMeasurementComponent::DestroyQuadArea()
{
        if (!IsQuadAreaExisted())
        {
                return;
        }

        QuadMeasurement->Destroy();
}

void UMetaMeasurementComponent::ChangeQuadAreaSize(float Size)
{
        if (!IsQuadAreaExisted())
        {
                return;
        }

        QuadMeasurement->ChangeScale(Size);
}

TArray<FVector2D> UMetaMeasurementComponent::GetQuadScreenLocations()
{
        if (!IsQuadAreaExisted())
        {
                return TArray<FVector2D>();
        }

        return QuadMeasurement->GetMeasurementInfo().ScreenPoints;
}

TArray<FVector> UMetaMeasurementComponent::GetQuadWorldLocations()
{
        if (!IsQuadAreaExisted())
        {
                return TArray<FVector>();
        }

        return QuadMeasurement->GetMeasurementInfo().WorldPoints;
}

void UMetaMeasurementComponent::DeleteRuler()
{
        const auto LastGuid = CreatedRulerGuid.Last();

        if (!LastGuid.IsValid())
        {
                return;
        }

        ARulerMeasurement* const Measurement = GetRulerMeasurement();

        if (!Measurement)
        {
                return;
        }

        Measurement->DestroyRuler(LastGuid.ToString());

        CreatedRulerGuid.Remove(LastGuid);
}

void UMetaMeasurementComponent::AddMeasurementPoint()
{
        ARulerMeasurement* const Measurement = GetRulerMeasurement();
        if (!IsValid(Measurement) || Measurement->GetRulerMode() == ERulerMode::RulerEdit)
        {
                return;
        }

        Measurement->CreateRulerPoint();
}
