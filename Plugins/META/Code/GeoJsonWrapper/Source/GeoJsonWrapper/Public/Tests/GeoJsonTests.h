// META all rights received (c)

#pragma once

#if WITH_TESTS

constexpr auto Mask = EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGeoJsonFeatureCollectionTest, "Meta.GeoJsonTests.FeatureCollectionTest", Mask)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGeoJsonGeometryCollectionTest, "Meta.GeoJsonTests.GeometryCollectionTest", Mask)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGeoJsonSerializationTest, "Meta.GeoJsonTests.SerializationTest", Mask)

#endif
