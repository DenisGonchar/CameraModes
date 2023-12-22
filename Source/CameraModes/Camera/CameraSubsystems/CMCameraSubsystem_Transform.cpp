// Fill out your copyright notice in the Description page of Project Settings.


#include "CMCameraSubsystem_Transform.h"

#include "GameFramework/Pawn.h"
#include "CollisionQueryParams.h"
#include "WorldCollision.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "CameraModes/Camera/CMCameraMode.h"
#include "CameraModes/Camera/CMSpringArmComponent.h"
#include "UObject/StrongObjectPtr.h"


void UCMCameraSubsystem_Transform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateDesiredArmLocation(bDoCollisionTest, bEnableCameraLag,
		bEnableCameraRotationLag, DeltaTime);
}

FRotator UCMCameraSubsystem_Transform::GetDesiredRotation() const
{
	return GetCameraRotation();
}

FRotator UCMCameraSubsystem_Transform::GetTargetRotation() const
{
	FRotator DesiredRot = GetDesiredRotation();

	if (bUsePawnControlRotation)
	{
		if (APawn* OwningPawn = GetOwningPawn())
		{
			const FRotator PawnViewRotation = OwningPawn->GetViewRotation();
			if (DesiredRot != PawnViewRotation)
			{
				DesiredRot = PawnViewRotation;
			}
		}
	}

	// If inheriting rotation, check options for which components to inherit
	if (!GetOwningSpringArm()->IsUsingAbsoluteRotation())
	{
		const FRotator LocalRelativeRotation = GetSocketTransform(NAME_None, RTS_Component).Rotator();
		if (!bInheritPitch)
		{
			DesiredRot.Pitch = LocalRelativeRotation.Pitch;
		}

		if (!bInheritYaw)
		{
			DesiredRot.Yaw = LocalRelativeRotation.Yaw;
		}

		if (!bInheritRoll)
		{
			DesiredRot.Roll = LocalRelativeRotation.Roll;
		}
	}

	return DesiredRot;
}

FVector UCMCameraSubsystem_Transform::GetCameraLocation() const
{
	return GetCameraTransform().GetLocation();
}

FRotator UCMCameraSubsystem_Transform::GetCameraRotation() const
{
	return GetCameraTransform().Rotator();
}

FTransform UCMCameraSubsystem_Transform::GetCameraTransform() const
{
	return GetSocketTransform(NAME_None, RTS_World);
}

void UCMCameraSubsystem_Transform::UpdateDesiredArmLocation(bool bDoTrace, bool bDoLocationLag, bool bDoRotationLag, float DeltaTime)
{
	FRotator DesiredRot = GetTargetRotation();

	// Apply 'lag' to rotation if desired
	if(bDoRotationLag)
	{
		if (bUseCameraLagSubstepping && DeltaTime > CameraLagMaxTimeStep
			&& CameraRotationLagSpeed > 0.f)
		{
			const FRotator ArmRotStep = (DesiredRot - PreviousDesiredRot).GetNormalized() * (1.f / DeltaTime);
			FRotator LerpTarget = PreviousDesiredRot;
			float RemainingTime = DeltaTime;
			while (RemainingTime > KINDA_SMALL_NUMBER)
			{
				const float LerpAmount = FMath::Min(CameraLagMaxTimeStep, RemainingTime);
				LerpTarget += ArmRotStep * LerpAmount;
				RemainingTime -= LerpAmount;

				DesiredRot = FRotator(FMath::QInterpTo(FQuat(PreviousDesiredRot), FQuat(LerpTarget),
					LerpAmount, CameraRotationLagSpeed));
				PreviousDesiredRot = DesiredRot;
			}
		}
		else
		{
			DesiredRot = FRotator(FMath::QInterpTo(FQuat(PreviousDesiredRot), FQuat(DesiredRot),
				DeltaTime, CameraRotationLagSpeed));
		}
	}
	PreviousDesiredRot = DesiredRot;

	// Get the spring arm 'origin', the target we want to look at
	//FVector ArmOrigin = GetCameraLocation() + TargetOffset;
	FVector ArmOrigin = GetOwningSpringArm()->GetComponentLocation() + TargetOffset;
	// We lag the target, not the actual camera position, so rotating the camera around does not have lag
	FVector DesiredLoc = ArmOrigin;
	if (bDoLocationLag)
	{
		if (bUseCameraLagSubstepping && DeltaTime > CameraLagMaxTimeStep
			&& CameraLagSpeed > 0.f)
		{
			const FVector ArmMovementStep = (DesiredLoc - PreviousDesiredLoc) * (1.f / DeltaTime);
			FVector LerpTarget = PreviousDesiredLoc;

			float RemainingTime = DeltaTime;
			while (RemainingTime > KINDA_SMALL_NUMBER)
			{
				const float LerpAmount = FMath::Min(CameraLagMaxTimeStep, RemainingTime);
				LerpTarget += ArmMovementStep * LerpAmount;
				RemainingTime -= LerpAmount;

				DesiredLoc = FMath::VInterpTo(PreviousDesiredLoc, LerpTarget, LerpAmount, CameraLagSpeed);
				PreviousDesiredLoc = DesiredLoc;
			}
		}
		else
		{
			DesiredLoc = FMath::VInterpTo(PreviousDesiredLoc, DesiredLoc, DeltaTime, CameraLagSpeed);
		}

		// Clamp distance if requested
		bool bClampedDist = false;
		if (CameraLagMaxDistance > 0.f)
		{
			const FVector FromOrigin = DesiredLoc - ArmOrigin;
			if (FromOrigin.SizeSquared() > FMath::Square(CameraLagMaxDistance))
			{
				DesiredLoc = ArmOrigin + FromOrigin.GetClampedToMaxSize(CameraLagMaxDistance);
				bClampedDist = true;
			}
		}		

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
		if (bDrawDebugLagMarkers)
		{
			DrawDebugSphere(GetWorld(), ArmOrigin, 5.f, 8, FColor::Green);
			DrawDebugSphere(GetWorld(), DesiredLoc, 5.f, 8, FColor::Yellow);

			const FVector ToOrigin = ArmOrigin - DesiredLoc;
			DrawDebugDirectionalArrow(GetWorld(), DesiredLoc, DesiredLoc + ToOrigin * 0.5f, 7.5f, bClampedDist ? FColor::Red : FColor::Green);
			DrawDebugDirectionalArrow(GetWorld(), DesiredLoc + ToOrigin * 0.5f, ArmOrigin,  7.5f, bClampedDist ? FColor::Red : FColor::Green);
		}
#endif
	}

	PreviousArmOrigin = ArmOrigin;
	PreviousDesiredLoc = DesiredLoc;

	// Now offset camera position back along our rotation
	DesiredLoc -= DesiredRot.Vector() * TargetArmLength;
	// Add socket offset in local space
	DesiredLoc += FRotationMatrix(DesiredRot).TransformVector(SocketOffset);

	// Do a sweep to ensure we are not penetrating the world
	FVector ResultLoc;
	if (bDoTrace && (TargetArmLength != 0.0f))
	{
		bIsCameraFixed = true;
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SpringArm), false, GetOwningActor());

		FHitResult Result;
		GetWorld()->SweepSingleByChannel(Result, ArmOrigin, DesiredLoc, FQuat::Identity, ProbeChannel, FCollisionShape::MakeSphere(ProbeSize), QueryParams);
		
		UnfixedCameraPosition = DesiredLoc;

		ResultLoc = BlendLocations(DesiredLoc, Result.Location, Result.bBlockingHit, DeltaTime);

		if (ResultLoc == DesiredLoc) 
		{	
			bIsCameraFixed = false;
		}
	}
	else
	{
		ResultLoc = DesiredLoc;
		bIsCameraFixed = false;
		UnfixedCameraPosition = ResultLoc;
	}

	// Form a transform for new world transform for camera
	FTransform WorldCamTM(DesiredRot, ResultLoc);
	// Convert to relative to component
	FTransform RelCamTM = WorldCamTM.GetRelativeTransform(GetOwningSpringArm()->GetComponentTransform());

	// Update socket location/rotation
	RelativeSocketLocation = RelCamTM.GetLocation();
	RelativeSocketRotation = RelCamTM.GetRotation();

}

FVector UCMCameraSubsystem_Transform::BlendLocations(const FVector& DesiredArmLocation, const FVector& TraceHitLocation, bool bHitSomething, float DeltaTime)
{
	return bHitSomething ? TraceHitLocation : DesiredArmLocation;
}

FTransform UCMCameraSubsystem_Transform::GetSocketTransform(FName InSocketName, ERelativeTransformSpace TransformSpace) const
{
	FTransform RelativeTransform(RelativeSocketRotation, RelativeSocketLocation);

	switch(TransformSpace)
	{
		case RTS_World:
		{
			return RelativeTransform * GetOwningSpringArm()->GetComponentTransform();
			break;
		}
		case RTS_Actor:
		{
			if( const AActor* Actor = GetOwningActor())
			{
				FTransform SocketTransform = RelativeTransform * GetOwningSpringArm()->GetComponentTransform();
				return SocketTransform.GetRelativeTransform(Actor->GetTransform());
			}
			break;
		}
		case RTS_Component:
		{
			return RelativeTransform;
		}
	}
	return RelativeTransform;
}

FVector UCMCameraSubsystem_Transform::GetUnfixedCameraPosition() const
{
	return UnfixedCameraPosition;
}

bool UCMCameraSubsystem_Transform::IsCollisionFixApplied() const
{
	return bIsCameraFixed;
}