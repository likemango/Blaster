// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SKGAnimInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEquipUnEquipEventDispatch);

/**
 * 
 */
UCLASS()
class BLASTER_API USKGAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, BlueprintCallable, BlueprintAssignable)
	FEquipUnEquipEventDispatch OnUnequipComplete;
	UPROPERTY(BlueprintReadWrite, BlueprintCallable, BlueprintAssignable)
	FEquipUnEquipEventDispatch OnEquipComplete;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bCrouched = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bUnequipping = false;

	void StartUnequip();
	
};
