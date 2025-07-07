#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

#include "GeographicalContext.h"
#include "CartographicTransformsLibrary.h"

BEGIN_DEFINE_SPEC(FCartographicTransformsLibrarySpec,
                  TEXT("Meta.UserControl"),
                  EAutomationTestFlags::ClientContext | EAutomationTestFlags::HighPriority | EAutomationTestFlags::EngineFilter)

static constexpr double MockSphereRadius = 1'000'000'000.0;
static inline FGeographicalEllipsoid const MockSphere{ FVector{}, { MockSphereRadius, MockSphereRadius, MockSphereRadius } };

END_DEFINE_SPEC(FCartographicTransformsLibrarySpec)

void FCartographicTransformsLibrarySpec::Define()
{
        BeforeEach([this] { UGeographicalContext::Initialize(MockSphere, nullptr); });

        AfterEach([this] { UGeographicalContext::Reset(); });

        Describe(TEXT("Drag"),
                 [this]
                 {
                         It(TEXT("should not apply vertical offset"),
                            [this]
                            {
                                    constexpr auto TargetDistanceToOrigin = MockSphereRadius + 100'000.0;

                                    auto const Pivot = MockSphere.Center + MockSphereRadius * FVector::UpVector;
                                    auto const Target = Pivot + (TargetDistanceToOrigin - MockSphereRadius) * FVector::UpVector;
                                    auto const PivotDestination = Pivot + 1'000'000.0 * FVector::RightVector;

                                    TestEqual(TEXT("Distance to origin"),
                                              FVector::Dist(UCartographicTransformsLibrary::Drag(Target, Pivot, PivotDestination), MockSphere.Center),
                                              TargetDistanceToOrigin,
                                              1);

                                    TestEqual(TEXT("Target position after vertical delta"),
                                              UCartographicTransformsLibrary::Drag(Target, Pivot, Pivot + 1000.0 * FVector::UpVector),
                                              Target);
                            });

                         It(TEXT("should not move if pivot offset is zero"),
                            [this]
                            {
                                    constexpr FVector Pivot{};
                                    const auto Target = Pivot + 1000.0 * FVector::UpVector;

                                    TestEqual(TEXT("Target position after vertical delta"), UCartographicTransformsLibrary::Drag(Target, Pivot, Pivot), Target);
                            });
                 });

        Describe(TEXT("Orbit"),
                 [this]
                 {
                         It(TEXT("should uphold radius invariance"),
                            [this]
                            {
                                    FVector const Pivot{ 100.0, 50.0, 30.0 };
                                    FVector const RelativeLocation{ 517.0, 981.0, 221.0 };

                                    FRotator const AngularVelocity{ 0, 120.0, 0.0 };
                                    FVector const Target = Pivot + RelativeLocation;

                                    TestEqual(TEXT("Initial and final radii"),
                                              FVector::DistSquared(
                                                      Pivot,
                                                      UCartographicTransformsLibrary::Orbit(Target, Pivot, AngularVelocity, (Pivot - Target).GetSafeNormal())),
                                              RelativeLocation.SquaredLength());
                            });

                         It(TEXT("should correctly mirror relative location"),
                            [this]
                            {
                                    FVector const Pivot{ 42.0, 240.0, 843.0 };
                                    FVector const RelativeLocation{ 420.0, 7.0, 7915.0 };

                                    FVector const HorizontalProjection =
                                            FVector::VectorPlaneProject(RelativeLocation, UGeographicalContext::CalculateSurfaceNormal(Pivot));

                                    FVector const Target = Pivot + HorizontalProjection;

                                    TestEqual(TEXT("Actual and expected horizontally mirrored locations"),
                                              UCartographicTransformsLibrary::Orbit(Target, Pivot, { 0.0, 180.0, 0.0 }, (Pivot - Target).GetSafeNormal()),
                                              Pivot - HorizontalProjection);
                            });
                 });
}

#endif
