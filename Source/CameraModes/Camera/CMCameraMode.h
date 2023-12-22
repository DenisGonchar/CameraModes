// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CMCameraMode.generated.h"

class UCMCameraSubsystem;

UCLASS()
class CAMERAMODES_API UCMCameraMode : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<UCMCameraSubsystem>> CameraSubsystems;
};
