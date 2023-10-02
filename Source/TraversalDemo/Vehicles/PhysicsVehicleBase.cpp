// Fill out your copyright notice in the Description page of Project Settings.


#include "TraversalDemo/Vehicles/PhysicsVehicleBase.h"

#include "DrawDebugHelpers.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

// Sets default values
APhysicsVehicleBase::APhysicsVehicleBase()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root Scene Component"));
	RootComponent = RootSceneComponent;

	MyPhysicsBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("My Physics Body"));
	MyPhysicsBody->SetupAttachment(RootComponent);

	FrontLeftWheel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SM_Front Left Wheel"));
	FrontLeftWheel->SetupAttachment(MyPhysicsBody);
	
	FrontRightWheel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SM_Front Right Wheel"));
	FrontRightWheel->SetupAttachment(MyPhysicsBody);
	
	RearLeftWheel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SM_Rear Left Wheel"));
	RearLeftWheel->SetupAttachment(MyPhysicsBody);
	
	RearRightWheel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SM_Rear Right Wheel"));
	RearRightWheel->SetupAttachment(MyPhysicsBody);
}

// Called when the game starts or when spawned
void APhysicsVehicleBase::BeginPlay()
{
	Super::BeginPlay();

	InitializeGears();
}

void APhysicsVehicleBase::RefreshThrottleInput(float NewThrottleInput)
{
	// Check if we are currently braking
	if (FMath::Sign(CurrentThrottleInput) != FMath::Sign(NewThrottleInput))
	{
		bIsBraking = true;
	}
	
	CurrentThrottleInput = NewThrottleInput;
}

void APhysicsVehicleBase::RefreshSteeringInput(float NewSteeringInput)
{
	CurrentSteeringInput = NewSteeringInput;
}

// Called every frame
void APhysicsVehicleBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Output all of the current input values
	const float CurrentVehicleSpeed = MyPhysicsBody->GetPhysicsLinearVelocity().Size();
	float SpeedInMPH = CurrentVehicleSpeed * 0.0223694;
	
	GEngine->AddOnScreenDebugMessage(0, DeltaTime * 1.05f, FColor::Red, FString::Printf(TEXT(
		"THROTTLE: %f, STEERING: %f, ENGINE RPM: %f, SPEED: %f"), CurrentThrottleInput, CurrentSteeringInput, CurrentEngineRPM,
		SpeedInMPH));

	UpdateEngineAndGearbox(DeltaTime);
	UpdatePhysics(DeltaTime);
}

void APhysicsVehicleBase::UpdatePhysics(float DeltaTime)
{
	UpdateDriveForce(DeltaTime);
}

void APhysicsVehicleBase::UpdateDriveForce(float DeltaTime)
{
	/*if (bIsBraking)
	{
		UpdateBrakeForce(DeltaTime);
	}
	else
	{
		UpdateThrottleForce(DeltaTime);
	}*/

	UpdateThrottleForce(DeltaTime);
}

void APhysicsVehicleBase::UpdateThrottleForce(float DeltaTime)
{
	// First, calculate the current maximum torque that the engine can produce
	const float CurrentMaxTorque = EngineTorqueCurve->GetFloatValue(CurrentEngineRPM);
	// This will account for whether we should move forward or backward
	const float CurrentEngineTorque = CurrentThrottleInput * CurrentMaxTorque;

	const float GearRatio = AllGearInfos[CurrentGearIndex].Ratio;
	const float CurrentDriveTorque = CurrentEngineTorque * DifferentialRatio * GearRatio * TransmissionEfficiency;

	GEngine->AddOnScreenDebugMessage(1, DeltaTime * 1.05f, FColor::Red, FString::Printf(TEXT(
		"CURRENT DRIVE TORQUE: %f"), CurrentDriveTorque));

	// ToDo: Change this to the proper substepped methods later on
	const FVector DirectionOfMovement = MyPhysicsBody->GetForwardVector();

	FVector ForceToApply = CurrentDriveTorque * DirectionOfMovement;
	ForceToApply /= WheelRadius;
	ForceToApply *= MyPhysicsBody->GetMass();

	// We will apply the force at the two rear wheels, making sure they are at the center of mass height.
	const float CenterOfMassHeight = MyPhysicsBody->GetCenterOfMass().Z;

	FVector LocationOfRearLeftWheel = RearLeftWheel->GetComponentLocation();
	FVector LocationOfRearRightWheel = RearRightWheel->GetComponentLocation();

	// ToDo: Change this to the proper substepped methods later on
	MyPhysicsBody->AddForceAtLocation(ForceToApply, LocationOfRearLeftWheel);
	MyPhysicsBody->AddForceAtLocation(ForceToApply, LocationOfRearRightWheel);

	// MyPhysicsBody->AddForce(ForceToApply * MyPhysicsBody->GetMass(), NAME_Name, false);
}

void APhysicsVehicleBase::UpdateBrakeForce(float DeltaTime)
{
	FVector ForceToApply = -MyPhysicsBody->GetPhysicsLinearVelocity().GetSafeNormal();
	ForceToApply *= CoefficientForBrakingForce;

	MyPhysicsBody->AddForce(ForceToApply, NAME_Name, true);
}

void APhysicsVehicleBase::UpdateRollingResistanceForce(float DeltaTime)
{
	FVector RollingResistanceForce = -MyPhysicsBody->GetPhysicsLinearVelocity().GetSafeNormal();
	RollingResistanceForce *= CoefficientForRollingResistanceForce;

	MyPhysicsBody->AddForce(RollingResistanceForce, NAME_Name, true);
}

void APhysicsVehicleBase::UpdateAirResistanceForce(float DeltaTime)
{
	const float CurrentVehicleSpeed = MyPhysicsBody->GetPhysicsLinearVelocity().Size();
	FVector AirResistanceForce = CurrentVehicleSpeed * CurrentVehicleSpeed * CoefficientForAirResistanceForce * -MyPhysicsBody->GetPhysicsLinearVelocity().GetSafeNormal();

	MyPhysicsBody->AddForce(AirResistanceForce, NAME_Name, true);
}

// Called to bind functionality to input
void APhysicsVehicleBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &APhysicsVehicleBase::RefreshThrottleInput);
	PlayerInputComponent->BindAxis(TEXT("MoveSideways"), this, &APhysicsVehicleBase::RefreshSteeringInput);
}

void APhysicsVehicleBase::UpdateEngineAndGearbox(float DeltaTime)
{
	// First, we need to derive the current engine RPM from the current wheel speed
	// We will just use the linear velocity of the entire vehicle for this, instead of individual wheels.
	float CurrentLinearSpeed = MyPhysicsBody->GetPhysicsLinearVelocity().Size();

	// Hopefully, this is in radians!
	float CurrentWheelRotationSpeed = CurrentLinearSpeed / WheelRadius;

	// rpm = wheel rotation rate * gear ratio * differential ratio * 60 / 2 pi
	CurrentEngineRPM = CurrentWheelRotationSpeed * AllGearInfos[CurrentGearIndex].Ratio * DifferentialRatio * 60.0f / (2.0f * PI);

	// We are going to make sure that our current engine RPM is within the min and max RPM values
	CurrentEngineRPM = FMath::Clamp(CurrentEngineRPM, MinimumEngineRPM, MaximumEngineRPM);
}

void APhysicsVehicleBase::InitializeGears()
{
	// First, we will go through all of our gear infos and locate the index of the reverse and neutral gear.
	for (int Index_Gear = 0; Index_Gear < AllGearInfos.Num(); Index_Gear++)
	{
		FVehicleGearInfo CurrentGearInfo = AllGearInfos[Index_Gear];

		switch (CurrentGearInfo.GearType)
		{
		case EVehicleGearType::TYPE_REGULAR: break;
		case EVehicleGearType::TYPE_NEUTRAL: IndexOfNeutralGear = Index_Gear; break;
		case EVehicleGearType::TYPE_REVERSE: IndexOfReverseGear = Index_Gear; break;
		}
	}

	// Validate that we have a neutral and reverse gear
	if (IndexOfNeutralGear == -1)
	{
		UE_LOG(LogTemp, Error, TEXT("No neutral gear was found!"));
	}

	if (IndexOfReverseGear == -1)
	{
		UE_LOG(LogTemp, Error, TEXT("No reverse gear was found!"));
	}
}
