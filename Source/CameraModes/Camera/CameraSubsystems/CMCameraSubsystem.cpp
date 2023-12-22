// Fill out your copyright notice in the Description page of Project Settings.


#include "CMCameraSubsystem.h"

#include "CameraModes/Camera/CMSpringArmComponent.h"

void UCMCameraSubsystem::Tick(float DeltaTime)
{
	
}

void UCMCameraSubsystem::SetOwningSpringArm(UCMSpringArmComponent* SpringArm)
{
	check(OwningSpringArmComponent == nullptr)
	OwningSpringArmComponent = SpringArm;
}

UCMSpringArmComponent* UCMCameraSubsystem::GetOwningSpringArm() const
{
	return OwningSpringArmComponent;
}

AActor* UCMCameraSubsystem::GetOwningActor() const
{
	return GetOwningSpringArm()->GetOwner();
}

APawn* UCMCameraSubsystem::GetOwningPawn() const
{
	return GetOwningSpringArm()->GetOwner<APawn>();
}

APlayerController* UCMCameraSubsystem::GetOwningController() const
{
	const auto owningPawn = GetOwningPawn();
	return owningPawn != nullptr ? owningPawn->GetController<APlayerController>() : nullptr;
}