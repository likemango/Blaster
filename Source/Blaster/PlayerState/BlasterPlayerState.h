// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BlasterPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	//服务端调用该函数
	void AddToScore(float ScoreAmount);
	//客户端调用
	virtual void OnRep_Score() override;
	
private:
	class ABlasterCharacter* BlasterCharacter;
	class ABlasterPlayerController* BlasterController;
};
