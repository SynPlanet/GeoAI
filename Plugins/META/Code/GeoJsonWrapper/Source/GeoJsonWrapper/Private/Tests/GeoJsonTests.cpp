// META all rights received (c)

#if WITH_TESTS

#include "Tests/GeoJsonTests.h"

#include "GeoJsonDeserializer.h"
#include "GeoJsonSerializer.h"
#include "GeoJsonDataStructures/GeoJsonFeatureCollection.h"
#include "GeoJsonDataStructures/GeoJsonGeometryCollection.h"
#include "GeoJsonDataStructures/GeoJsonObjectFeature.h"

namespace
{
        const FString PathToPlugin = FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("META"), TEXT("Code"), TEXT("GeoJsonWrapper"));
}

bool FGeoJsonFeatureCollectionTest::RunTest(const FString& Parameters)
{
        const FString PathToFile = FPaths::Combine(PathToPlugin, TEXT("Resources"), TEXT("TestFeatureCollection.geojson"));
        bool bTestPassed = true;
        UGeoJsonObject* const Object = UGeoJsonDeserializer::DeserializePath(PathToFile);
        bTestPassed &= TestNotNull("File deserialized", Object);
        UGeoJsonFeatureCollection* const FeatureCollection = Cast<UGeoJsonFeatureCollection>(Object);
        bTestPassed &= TestEqual("Features in collection", FeatureCollection->Num(), 2);
        const UGeoJsonObjectFeature* Feature = FeatureCollection->GetOrCreateFeatureObject(0);
        bTestPassed &= TestNotNull("Feature object created", Feature);
        FJsonPoint Point;
        // Check geometry
        bTestPassed &= TestTrue("Point extracted", Feature->TryGetPoint(Point));
        bTestPassed &= TestEqual("Point is correct", Point.Coordinates, FVector(-7988.154, -4902.187, 0.), 0.001);
        // Check properties
        bTestPassed &= TestTrue("Boolean property", Feature->GetBoolProperty("bool"));
        bTestPassed &= TestEqual("String property", Feature->GetStringProperty("string"), "not_empty");
        bTestPassed &= TestEqual("Double property", Feature->GetNumberProperty("double"), -1.45);
        bTestPassed &= TestEqual("Integer property", Feature->GetIntProperty("integer"), 42);
        bTestPassed &= TestEqual("Array property", Feature->GetStringProperty("array.0.value"), "first");
        bTestPassed &= TestEqual("Array property", Feature->GetIntProperty("array.0.integer"), 1);
        bTestPassed &= TestEqual("Array property", Feature->GetStringProperty("array.1.value"), "second");
        bTestPassed &= TestEqual("Array property", Feature->GetIntProperty("array.1.integer"), 10);
        bTestPassed &= TestEqual("Array property", Feature->GetStringProperty("array.2.value"), "third");
        bTestPassed &= TestEqual("Array property", Feature->GetIntProperty("array.2.integer"), 100);
        bTestPassed &= TestFalse("Object property", Feature->GetBoolProperty("object.bool"));
        bTestPassed &= TestEqual("Object property", Feature->GetStringProperty("object.string"), "good");

        Feature = FeatureCollection->GetOrCreateFeatureObject(1);
        bTestPassed &= TestNotNull("Feature object created", Feature);
        FJsonGeometry Geometry;
        bTestPassed &= TestTrue("Geometry extracted", Feature->TryGetGeometry(Geometry));
        bTestPassed &= TestTrue("Geometry is multipolygon", Geometry.Geometry.IsType<FJsonMultiPolygon>());
        const FJsonMultiPolygon& MultiPolygon = Geometry.Geometry.Get<FJsonMultiPolygon>();
        bTestPassed &= TestEqual("", MultiPolygon.Coordinates.Num(), 2);
        bTestPassed &= TestEqual("", MultiPolygon.Coordinates[0].Coordinates[0].Coordinates[0].Coordinates, FVector(30.5, 50.5, 0.));
        bTestPassed &= TestEqual("", MultiPolygon.Coordinates[1].Coordinates[0].Coordinates[0].Coordinates, FVector(40.5, 50.5, 0.));
        return bTestPassed;
}

bool FGeoJsonGeometryCollectionTest::RunTest(const FString& Parameters)
{
        const FString PathToFile = FPaths::Combine(PathToPlugin, TEXT("Resources"), TEXT("TestGeometryCollection.geojson"));
        bool bTestPassed = true;
        const UGeoJsonObject* const Object = UGeoJsonDeserializer::DeserializePath(PathToFile);
        bTestPassed &= TestNotNull("File deserialized", Object);
        const UGeoJsonGeometryCollection* const GeometryCollection = Cast<const UGeoJsonGeometryCollection>(Object);
        bTestPassed &= TestNotNull("Geometry Collection", GeometryCollection);
        const FJsonPoint& GeoJsonPoint = GeometryCollection->GetPoint(0);
        bTestPassed &= TestEqual("Point value", GeoJsonPoint.Coordinates, FVector(40.0, 10.0, 20.0));
        const FJsonMultiPoint& GeoJsonLine = GeometryCollection->GetLineString(1);
        bTestPassed &= TestEqual("Line size", GeoJsonLine.Coordinates.Num(), 3);
        bTestPassed &= TestEqual("Line points", GeoJsonLine.Coordinates[0].Coordinates, FVector(10.0, 10.0, 10.0));
        bTestPassed &= TestEqual("Line points", GeoJsonLine.Coordinates[1].Coordinates, FVector(20.0, 20.0, 20.0));
        bTestPassed &= TestEqual("Line points", GeoJsonLine.Coordinates[2].Coordinates, FVector(10.0, 40.0, 30.0));
        const FJsonMultiPoint& GeoJsonMultiPoint = GeometryCollection->GetMultiPoint(2);
        bTestPassed &= TestEqual("Multipoint and LineString equality", GeoJsonMultiPoint.Coordinates, GeoJsonLine.Coordinates);
        return bTestPassed;
}

bool FGeoJsonSerializationTest::RunTest(const FString& Parameters)
{
        bool bTestPassed = true;
        const FString PathToFile = FPaths::Combine(PathToPlugin, TEXT("Resources"), TEXT("TestFeatureCollection.geojson"));
        const UGeoJsonObject* const Object = UGeoJsonDeserializer::DeserializePath(PathToFile);
        const UGeoJsonFeatureCollection* const FeatureCollection = Cast<UGeoJsonFeatureCollection>(Object);
        const FJsonFeatureCollection& Collection = FeatureCollection->GetCollection();
        bTestPassed &= TestEqual("Deserialization succeeded", Collection.Features.Num(), 2);
        bTestPassed &= TestEqual("Deserialization succeeded", Collection.Features[1].GeometryType, EGeoJsonType::MultiPolygon);

        const FJsonMultiPolygon& MultiPolygon = Collection.Features[1].Geometry.Geometry.Get<FJsonMultiPolygon>();
        const TArray<TSharedPtr<FJsonValue>> Serialized = UGeoJsonSerializer::SerializeMultiPolygon(MultiPolygon);
        bTestPassed &= TestEqual("Serialization succeeded", MultiPolygon.Coordinates.Num(), Serialized.Num());

        for (int32 PolygonIdx = 0; PolygonIdx < MultiPolygon.Coordinates.Num(); ++PolygonIdx)
        {
                const FJsonPolygon& Polygon = MultiPolygon.Coordinates[PolygonIdx];
                const TArray<TSharedPtr<FJsonValue>>& SerializedPolygon = Serialized[PolygonIdx]->AsArray();
                bTestPassed &= TestEqual("Polygons equality", Polygon.Coordinates.Num(), SerializedPolygon.Num());
                for (int32 MultiPointIdx = 0; MultiPointIdx < Polygon.Coordinates.Num(); ++MultiPointIdx)
                {
                        const FJsonMultiPoint& MultiPoint = Polygon.Coordinates[MultiPointIdx];
                        const TArray<TSharedPtr<FJsonValue>>& SerializedMultiPoint = SerializedPolygon[MultiPointIdx]->AsArray();
                        bTestPassed &= TestEqual("Multipoints equality", MultiPoint.Coordinates.Num(), SerializedMultiPoint.Num());
                        for (int32 PointIdx = 0; PointIdx < MultiPoint.Coordinates.Num(); ++PointIdx)
                        {
                                const FJsonPoint& Point = MultiPoint.Coordinates[PointIdx];
                                const TArray<TSharedPtr<FJsonValue>>& SerializedPoint = SerializedMultiPoint[PointIdx]->AsArray();
                                bTestPassed &= TestEqual("Points equality", SerializedPoint.Num(), 3);
                                for (int32 CoordIdx = 0; CoordIdx < SerializedPoint.Num(); ++CoordIdx)
                                {
                                        bTestPassed &= TestEqual("Coords equality", SerializedPoint[CoordIdx]->AsNumber(), Point.Coordinates[CoordIdx]);
                                }
                        }
                }
        }

        return bTestPassed;
}

#endif