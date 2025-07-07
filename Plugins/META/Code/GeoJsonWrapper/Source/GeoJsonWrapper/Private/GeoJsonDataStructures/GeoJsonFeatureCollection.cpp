// META all rights received (c)

#include "GeoJsonDataStructures/GeoJsonFeatureCollection.h"
#include "GeoJsonDataStructures/GeoJsonObjectFeature.h"

EGeoJsonType UGeoJsonFeatureCollection::GetType() const
{
        return EGeoJsonType::FeatureCollection;
}

UGeoJsonObjectFeature* UGeoJsonFeatureCollection::GetOrCreateFeatureObject(int32 Idx)
{
        if (Idx < 0 || Idx >= Num())
        {
                return nullptr;
        }
        if (UGeoJsonObjectFeature* const* const Feature = CreatedFeatures.Find(Idx); Feature && IsValid(*Feature))
        {
                return *Feature;
        }
        UGeoJsonObjectFeature* const NewFeature = CreatedFeatures.Add(Idx, NewObject<UGeoJsonObjectFeature>());
        NewFeature->SetFeature(Collection.Features[Idx]);
        return NewFeature;
}

bool UGeoJsonFeatureCollection::IsEmpty() const
{
        return Collection.Features.IsEmpty();
}

int32 UGeoJsonFeatureCollection::Num() const
{
        return Collection.Features.Num();
}

const FJsonFeatureCollection& UGeoJsonFeatureCollection::GetCollection() const
{
        return Collection;
}

void UGeoJsonFeatureCollection::SetCollection(FJsonFeatureCollection&& NewCollection)
{
        Collection = MoveTemp(NewCollection);
}

void UGeoJsonFeatureCollection::SetCollection(const FJsonFeatureCollection& NewCollection)
{
        Collection = NewCollection;
}