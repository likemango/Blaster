// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverHeadWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UOverHeadWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;

public:
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* DisplayText;

private:

	FString GetRemoteNetRole(APawn* InPawn);
	FString GetLocalNetRole(APawn* InPawn);
	FString GetPlayerName(APawn* InPawn);
	
	UFUNCTION(BlueprintCallable)
	void ShowNetRole(APawn* InPawn);

	UFUNCTION(BlueprintCallable)
	void ShowPlayerName(APlayerController* PlayerController);
};
