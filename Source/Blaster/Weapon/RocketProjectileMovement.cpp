// Fill out your copyright notice in the Description page of Project Settings.


#include "RocketProjectileMovement.h"

UProjectileMovementComponent::EHandleBlockingHitResult URocketProjectileMovement::HandleBlockingHit(
	const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining)
{
	Super::HandleBlockingHit(Hit, TimeTick, MoveDelta, SubTickTimeRemaining);
	return EHandleBlockingHitResult::AdvanceNextSubstep;
}

void URocketProjectileMovement::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	// 它默认使用根组件的碰撞来管理Actor的运动，但是我们并不需要，因为根组件(BoxCollision)已经注册了我们需要的碰撞逻辑
}

