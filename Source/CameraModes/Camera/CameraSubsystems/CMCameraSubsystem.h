// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CMCameraSubsystem.generated.h"


class UCMSpringArmComponent;
class AActor;
class APawn;
class APlayerController;

UCLASS(EditInlineNew, DefaultToInstanced, Abstract)
class UCMCameraModeSubsystem_BaseSettings : public UDataAsset
{
	GENERATED_BODY()
public:
	
};


struct FCMCameraSubsystemContext
{
public:
	bool bWithInterpolation = true;
	
};

UCLASS(Blueprintable, BlueprintType, Abstract, EditInlineNew, DefaultToInstanced)
class CAMERAMODES_API UCMCameraSubsystem : public UObject
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime);

	virtual void OnEnterToCameraMode(const FCMCameraSubsystemContext& Context);

	virtual void SetSubsystemSettings(UCMCameraModeSubsystem_BaseSettings* NewSettings);
	virtual UCMCameraModeSubsystem_BaseSettings* GetSubsystemSettings() const;
	
	void SetOwningSpringArm(UCMSpringArmComponent* SpringArm);
	UCMSpringArmComponent* GetOwningSpringArm() const;

	AActor* GetOwningActor() const;
	APawn* GetOwningPawn() const;

	APlayerController* GetOwningController() const;

	APlayerCameraManager* GetCameraManager() const;
private:
	
	UPROPERTY()
	UCMSpringArmComponent* OwningSpringArmComponent;
};
