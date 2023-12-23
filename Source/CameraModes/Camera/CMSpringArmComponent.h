// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "CameraSubsystems/CMCameraSubsystem.h"
#include "Components/SceneComponent.h"
#include "CMSpringArmComponent.generated.h"

class UCMCameraSubsystem;
class UCMCameraMode;

UCLASS(meta=(BlueprintSpawnableComponent), hideCategories=(Mobility))
class UCMSpringArmComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UCMSpringArmComponent();

	virtual void BeginPlay() override;
	
	// UActorComponent interface
	virtual void OnRegister() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	// End of UActorComponent interface

	// USceneComponent interface
	virtual bool HasAnySockets() const override;
	virtual FTransform GetSocketTransform(FName InSocketName, ERelativeTransformSpace TransformSpace = RTS_World) const override;
	virtual void QuerySupportedSockets(TArray<FComponentSocketDescription>& OutSockets) const override;
	// End of USceneComponent interface

	
	UFUNCTION(BlueprintCallable)
	void SetCameraMode(FGameplayTag CameraModeTag);
	
	UFUNCTION(BlueprintPure)
    UCMCameraMode* GetCurrentCameraMode() const;

	UFUNCTION(BlueprintPure)
	const TArray<UCMCameraSubsystem*>& GetCameraSubsystems() const;

	template<typename TSubsystem>
	TSubsystem* GetCameraSubsystem() const
	{
		for (auto subsystem : CameraSubsystems)
		{
			if (subsystem != nullptr)
			{
				if (subsystem->GetClass()->IsChildOf(TSubsystem::StaticClass()))
				{
					return Cast<TSubsystem>(subsystem);
				}
				
			}
		}
		return nullptr;
	}

	APlayerController* GetOwningController() const;

	FRotator GetPlayerRotationInput() const;
	
public:
	UPROPERTY(EditAnywhere, Category="Camera Modes")
	TArray<UCMCameraMode*> CameraModes;

	UPROPERTY(EditAnywhere, Category="Camera Modes")
	FGameplayTag InitialCameraModeTag;

private:
	void OnControllerRotationInput(FRotator InPlayerInput);

	void SetCameraMode(UCMCameraMode* NewCameraMode);
	
private:
	UPROPERTY(Transient)
	UCMCameraMode* CurrentCameraMode;

	UPROPERTY(Transient)
	TArray<UCMCameraSubsystem*> CameraSubsystems;

	FRotator PlayerRotationInput;
	
};


