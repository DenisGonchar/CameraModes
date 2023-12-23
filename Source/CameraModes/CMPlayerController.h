// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CMPlayerController.generated.h"


UCLASS()
class CAMERAMODES_API ACMPlayerController : public APlayerController
{
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnRotationInputTickDelegate, FRotator /*RotationInput*/)
	GENERATED_BODY()

public:
	virtual void ProcessPlayerInput(const float DeltaTime, const bool bGamePaused) override;
public:
	FOnRotationInputTickDelegate OnRotationInputTickDelegate;
};
