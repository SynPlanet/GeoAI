// META all rights received (c)

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MetaMeasurementComponent.generated.h"

class ARulerMeasurement;
class AQuadMeasurement;

UCLASS(ClassGroup = (Custom), Category = "MetaMeasurements", meta = (BlueprintSpawnableComponent))
class METAMEASUREMENT_API UMetaMeasurementComponent : public UActorComponent
{
        GENERATED_BODY()

        UPROPERTY(EditDefaultsOnly)
        TSubclassOf<ARulerMeasurement> RulerMeasurementClass;

        UPROPERTY()
        TObjectPtr<ARulerMeasurement> RulerMeasurement;

        TArray<FGuid> CreatedRulerGuid;

        UPROPERTY(EditDefaultsOnly)
        TSubclassOf<AQuadMeasurement> QuadMeasurementClass;

        UPROPERTY()
        TObjectPtr<AQuadMeasurement> QuadMeasurement;

public:
        /**
        * @brief Returns the current ARulerMeasurement instance. If the instance is invalid, a new one is
        * created and assigned.
        *
        * @return A valid pointer to the ARulerMeasurement instance.
        */
        UFUNCTION(BlueprintPure, Category = "Config|Getter")
        ARulerMeasurement* GetRulerMeasurement();

        UFUNCTION(BlueprintPure, Category = "Config|Getter")
        bool IsRulerExisted() const;

        UFUNCTION(BlueprintPure, Category = "Config|Getter")
        bool IsQuadAreaExisted() const;

        UFUNCTION(BlueprintPure, Category = "Config|Getter")
        float GetNormalizedQuadAreaScaledSize() const;

        UFUNCTION(BlueprintCallable, Category = "MetaMeasurements|Ruler")
        void CreateRuler();

        UFUNCTION(BlueprintCallable, Category = "MetaMeasurements|Ruler")
        void CreateOrDestroyQuadArea();

        UFUNCTION(BlueprintCallable, Category = "MetaMeasurements|Ruler")
        bool CreateQuadArea();

        UFUNCTION(BlueprintCallable, Category = "MetaMeasurements|Ruler")
        void DestroyQuadArea();

        UFUNCTION(BlueprintCallable, Category = "MetaMeasurements|Ruler")
        void ChangeQuadAreaSize(float Size);

        UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MetaMeasurements|Ruler")
        TArray<FVector2D> GetQuadScreenLocations();
        UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MetaMeasurements|Ruler")
        TArray<FVector> GetQuadWorldLocations();

        UFUNCTION(BlueprintCallable, Category = "MetaMeasurements|Ruler")
        void DeleteRuler();

        UFUNCTION(BlueprintCallable, Category = "MetaMeasurements|Ruler")
        void AddMeasurementPoint();
};
