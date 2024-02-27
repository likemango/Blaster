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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//客户端调用
	virtual void OnRep_Score() override;
	UFUNCTION()
	virtual void OnRep_Defeats();
	
	//服务端调用该函数
	void AddToScore(float ScoreAmount);
	void AddToDefeats(int32 DefeatAmount);
	
private:
	UPROPERTY()
	class ABlasterCharacter* BlasterCharacter;
	UPROPERTY()
	class ABlasterPlayerController* BlasterController;

	UPROPERTY(ReplicatedUsing=OnRep_Defeats)
	int32 Defeats;
};
