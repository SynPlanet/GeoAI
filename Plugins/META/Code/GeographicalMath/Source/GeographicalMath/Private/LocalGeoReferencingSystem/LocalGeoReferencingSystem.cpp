#include "LocalGeoReferencingSystem/LocalGeoReferencingSystem.h"

#include "LocalGeoReferencingSystem/LocalGeoReferencingSettings.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(LogLocalGeoReferencingSystem);

ALocalGeoReferencingSystem::ALocalGeoReferencingSystem()
{
        const ULocalGeoReferencingSettings* const Settings = ULocalGeoReferencingSettings::Get();
        if (!Settings)
        {
                UE_LOG(LogLocalGeoReferencingSystem, Error, TEXT("Could not retrieve LocalGeoReferencingSettings in LocalGeoReferencingSystem's constructor."));
                return;
        }

        PlanetShape = Settings->PlanetShape;
        ProjectedCRS = Settings->ProjectedCrs;
        GeographicCRS = Settings->GeographicCrs;
        bOriginLocationInProjectedCRS = Settings->bOriginInProjectedCrs;

        const auto& [OriginLon, OriginLat, OriginAlt] = Settings->Origin;
        Tie(OriginLongitude, OriginLatitude, OriginAltitude) = Tie(OriginLon, OriginLat, OriginAlt);
}

ALocalGeoReferencingSystem* ALocalGeoReferencingSystem::Get(UObject const* WorldContext)
{
        if (Singleton.IsValid())
        {
                return Singleton.Get();
        }

        UWorld* const World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull);
        if (!IsValid(World))
        {
                return nullptr;
        }

        if (ALocalGeoReferencingSystem* const Grs = Cast<ALocalGeoReferencingSystem>(UGameplayStatics::GetActorOfClass(World, StaticClass())); IsValid(Grs))
        {
                return Grs;
        }

        Singleton = World->SpawnActor<ALocalGeoReferencingSystem>(StaticClass());
        return Singleton.Get();
}
