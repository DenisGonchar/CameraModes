// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CMCameraSubsystem.generated.h"


class UCMSpringArmComponent;
class AActor;
class APawn;
class APlayerController;

UCLASS(Blueprintable, BlueprintType, Abstract, EditInlineNew, DefaultToInstanced)
class CAMERAMODES_API UCMCameraSubsystem : public UObject
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime);

	virtual void OnEnterToCameraMode();
	
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
