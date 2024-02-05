// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAnimInstance.h"

#include "BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/MovementComponent.h"

void UBlasterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
	
}

void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if(!BlasterCharacter)
	{
		BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
	}
	if(!BlasterCharacter) return;
	UCharacterMovementComponent* CharacterMovementComponent = BlasterCharacter->GetCharacterMovement();
	if(!CharacterMovementComponent) return;
	
	FVector Velocity = BlasterCharacter->GetVelocity();
	Velocity.Z = 0;
	Speed = Velocity.Size();

	bIsInAir = CharacterMovementComponent->IsFalling();
	
	bIsAccelerating = CharacterMovementComponent->GetCurrentAcceleration().Size() > 0 ? true : false;

	bEquippedWeapon = BlasterCharacter->IsEquippedWeapon();
}
