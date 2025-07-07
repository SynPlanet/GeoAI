#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

#if WITH_AUTOMATION_TESTS

#include "GeographicalTransforms.h"
#include <mutex>

BEGIN_DEFINE_SPEC(FGeographicalTransformsSpec,
                  TEXT("Meta.GeographicalTransforms"),
                  EAutomationTestFlags::ClientContext | EAutomationTestFlags::HighPriority | EAutomationTestFlags::ProductFilter)

const FGeographicalLocation OriginWgs84{ 37.615600, 55.752200, 144.000000 };
const FVector OriginEcef{ 2849872.7276374628, 2195933.8292116085, 5249083.7522441726 };
const FVector OriginMsk77{ 4187349.4378834614, 7509243.01057922, 144.0 };
const FVector OriginUnreal{ 0.0, 0.0, 0.0 };

END_DEFINE_SPEC(FGeographicalTransformsSpec)

namespace
{
        const FString MainMap = TEXT("/Game/MetaMoscow/Maps/MetaMoscow_map/BP_MetaMoscow_map");

        const auto CheckOpenMap = [](const UWorld* World) { return !IsValid(World) || World->GetName() != TEXT("BP_MetaMoscow_map"); };

        UWorld* GetTestGameWorld()
        {
                const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
                for (const FWorldContext& Context : WorldContexts)
                {
                        if ((Context.WorldType == EWorldType::PIE || Context.WorldType == EWorldType::Game) && Context.World())
                        {
                                return Context.World();
                        }
                }

                return nullptr;
        }

} // namespace

void FGeographicalTransformsSpec::Define()
{
        Describe(TEXT("CRS"),
                 [this]
                 {
                         BeforeEach(
                                 [this]
                                 {
                                         static std::once_flag Once;
                                         std::call_once(Once,
                                                        [this]
                                                        {
                                                                AutomationOpenMap(MainMap);
                                                                ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(2.0f));
                                                        });
                                         auto const& Contexts = GEngine->GetWorldContexts();
                                         auto const* const World = Contexts[0].World();
                                         if (CheckOpenMap(GetTestGameWorld()))
                                         {
                                                 AutomationOpenMap(MainMap);
                                                 ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(2.0f));

                                                 if (CheckOpenMap(GetTestGameWorld()))
                                                 {
                                                         AddError(FString::Printf(TEXT("Loaded world name is '%s', but expected '%s'."),
                                                                                  *GetTestGameWorld()->GetName(),
                                                                                  TEXT("BP_MetaMoscow_map")));
                                                         return;
                                                 }
                                         }
                                 });

                         It(TEXT("should transform between WGS84 and ECEF"),
                            EAsyncExecution::TaskGraph,
                            [this]
                            {
                                    TestEqual(TEXT("WGS84 -> ECEF"), FVector(UGeographicalTransforms::EcefToWgs84(OriginEcef)), FVector(OriginWgs84));
                                    TestEqual(TEXT("ECEF -> WGS84"), UGeographicalTransforms::Wgs84ToEcef(OriginWgs84), OriginEcef);
                            });

                         It(TEXT("should transform between ECEF and Unreal"),
                            EAsyncExecution::TaskGraph,
                            [this]
                            {
                                    TestEqual(TEXT("ECEF -> UE"), UGeographicalTransforms::EcefToUnreal(OriginEcef), OriginUnreal);
                                    TestEqual(TEXT("UE -> ECEF"), UGeographicalTransforms::UnrealToEcef(OriginUnreal), OriginEcef);
                            });

                         It(TEXT("should transform between WGS84 and Unreal"),
                            EAsyncExecution::TaskGraph,
                            [this]
                            {
                                    TestEqual(TEXT("WGS84 -> UE"), UGeographicalTransforms::Wgs84ToUnreal(OriginWgs84), OriginUnreal);
                                    TestEqual(TEXT("UE -> WGS84"), FVector(UGeographicalTransforms::UnrealToWgs84(OriginUnreal)), FVector(OriginWgs84));
                            });

                         It(TEXT("should transform between WGS84 and MSK77"),
                            EAsyncExecution::TaskGraph,
                            [this]
                            {
                                    TestEqual(TEXT("WGS84 -> MSK77"), UGeographicalTransforms::Wgs84ToMsk77(OriginWgs84), OriginMsk77);
                                    TestEqual(TEXT("MSK77 -> WGS84"), FVector(UGeographicalTransforms::Msk77ToWgs84(OriginMsk77)), FVector(OriginWgs84));
                            });

                         It(TEXT("should transform between ECEF and MSK77"),
                            EAsyncExecution::TaskGraph,
                            [this]
                            {
                                    TestEqual(TEXT("ECEF -> MSK77"), UGeographicalTransforms::EcefToMsk77(OriginEcef), OriginMsk77);
                                    TestEqual(TEXT("MSK77 -> ECEF"), UGeographicalTransforms::Msk77ToEcef(OriginMsk77), OriginEcef);
                            });

                         It(TEXT("should transform between Unreal and MSK77"),
                            EAsyncExecution::TaskGraph,
                            [this]
                            {
                                    TestEqual(TEXT("UE -> MSK77"), UGeographicalTransforms::UnrealToMsk77(OriginUnreal), OriginMsk77);
                                    TestEqual(TEXT("MSK77 -> UE"), UGeographicalTransforms::Msk77ToUnreal(OriginMsk77), OriginUnreal);
                            });
                 });
}

#endif