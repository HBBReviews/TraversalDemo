// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PhysicsVehicleBase.generated.h"

class UPhysicsConstraintComponent;

UENUM()
enum class EVehicleGearType : uint8
{
	TYPE_REGULAR,
	TYPE_REVERSE,
	TYPE_NEUTRAL
};

USTRUCT()
struct FVehicleGearInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	float Ratio = 0.f;

	UPROPERTY(EditDefaultsOnly)
	EVehicleGearType GearType = EVehicleGearType::TYPE_REGULAR;
};

UCLASS()
class TRAVERSALDEMO_API APhysicsVehicleBase : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APhysicsVehicleBase();

#pragma region Components

	UPROPERTY(EditDefaultsOnly, Category = "Vehicle Properties|Components")
	USceneComponent* RootSceneComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Vehicle Properties|Components")
	UStaticMeshComponent* MyPhysicsBody;

	UPROPERTY(EditDefaultsOnly, Category = "Vehicle Properties|Components")
	UStaticMeshComponent* FrontLeftWheel;

	UPROPERTY(EditDefaultsOnly, Category = "Vehicle Properties|Components")
	UStaticMeshComponent* FrontRightWheel;

	UPROPERTY(EditDefaultsOnly, Category = "Vehicle Properties|Components")
	UStaticMeshComponent* RearLeftWheel;

	UPROPERTY(EditDefaultsOnly, Category = "Vehicle Properties|Components")
	UStaticMeshComponent* RearRightWheel;

#pragma endregion Components

protected:
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

#pragma region Input
	
	float CurrentThrottleInput = 0.f;

	bool bIsBraking = false;
	
	float CurrentSteeringInput = 0.f;

	void RefreshThrottleInput(float NewThrottleInput);
	void RefreshSteeringInput(float NewSteeringInput);
	
#pragma endregion Input
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void UpdatePhysics(float DeltaTime);

	virtual void UpdateDriveForce(float DeltaTime);
	virtual void UpdateThrottleForce(float DeltaTime);
	virtual void UpdateBrakeForce(float DeltaTime);

	virtual void UpdateRollingResistanceForce(float DeltaTime);
	virtual void UpdateAirResistanceForce(float DeltaTime);

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

#pragma region Wheels

	UPROPERTY(EditDefaultsOnly, Category = "Vehicle Properties|Wheels")
	float WheelRadius = 30.f;

#pragma endregion Wheels

#pragma region EngineProperties

	// The current RPM of the engine
	UPROPERTY(VisibleInstanceOnly, Category = "Vehicle Properties|Engine")
	float CurrentEngineRPM = 0.f;

	// The minimum RPM the engine can reach
	UPROPERTY(EditDefaultsOnly, Category = "Vehicle Properties|Engine")
	float MinimumEngineRPM = 500.f;

	// The maximum RPM the engine can reach
	UPROPERTY(EditDefaultsOnly, Category = "Vehicle Properties|Engine")
	float MaximumEngineRPM = 6000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Vehicle Properties|Engine")
	UCurveFloat* EngineTorqueCurve;

	void UpdateEngineAndGearbox(float DeltaTime);
	
#pragma endregion EngineProperties

#pragma region Gears

	UPROPERTY(EditDefaultsOnly, Category = "Vehicle Properties|Gears")
	TArray<FVehicleGearInfo> AllGearInfos;

	UPROPERTY(VisibleInstanceOnly, Category = "Vehicle Properties|Gears")
	int CurrentGearIndex = 0;

	// We should only have one reverse gear.
	UPROPERTY(VisibleInstanceOnly, Category = "Vehicle Properties|Gears")
	int IndexOfReverseGear = -1;

	UPROPERTY(VisibleInstanceOnly, Category = "Vehicle Properties|Gears")
	int IndexOfNeutralGear = -1;
	
	void InitializeGears();
	
#pragma endregion Gears

#pragma region Transmission

	UPROPERTY(EditDefaultsOnly, Category = "Vehicle Properties|Transmission")
	float DifferentialRatio = 2.25f;

	UPROPERTY(EditDefaultsOnly, Category = "Vehicle Properties|Transmission")
	float TransmissionEfficiency = 0.7f;

#pragma endregion Transmission

#pragma region Braking

	UPROPERTY(EditDefaultsOnly, Category = "Vehicle Properties|Braking")
	float CoefficientForBrakingForce = 2000.f;

#pragma endregion Braking

#pragma region RollingResistance

	UPROPERTY(EditDefaultsOnly, Category = "Vehicle Properties|Rolling Resistance")
	float CoefficientForRollingResistanceForce = 1000.f;

#pragma endregion RollingResistance

#pragma region AirResistance

	UPROPERTY(EditDefaultsOnly, Category = "Vehicle Properties|Air Resistance")
	float CoefficientForAirResistanceForce = 0.5f;

#pragma endregion AirResistance
	
};
