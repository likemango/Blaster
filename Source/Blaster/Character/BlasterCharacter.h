﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "Blaster/Interface/InteractWithCrosshairsInterface.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Character.h"
#include "BlasterCharacter.generated.h"

class USoundCue;
class UTimelineComponent;

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	ABlasterCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void UpdateHealthHUD();
	virtual void PostInitializeComponents() override;
	void PlayFireMontage(bool bAiming);
	void PlayElimMontage();
	
	FVector GetHitTarget() const;
	virtual void OnRep_ReplicatedMovement() override;

	void Eliminate();
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastEliminate();

protected:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;
	
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void EquipButtonPressed();
	void CrouchPressed();
	void CrouchReleased();
	void AimPressed();
	void AimReleased();
	void CalculateAO_Pitch();
	void CalculateAimOffset(float DeltaTime);
	virtual void Jump() override;
	void FirePressed();
	void FireReleased();
	void PlayHitReactMontage();
	// poll init for some classes which is not ready when character BeginPlay()
	void PollInit();
private:
	UPROPERTY(VisibleAnywhere, Category=Camera)
	class USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, Category=Camera)
	class UCameraComponent* FollowCamera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	class UWidgetComponent* WidgetComponent;
	
	UPROPERTY(ReplicatedUsing=OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;
	UPROPERTY(VisibleAnywhere)
	class UCombatComponent* Combat;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	// RPC(remote process call, client call server to execute)
	// reliable: if failed, it will send again to make sure it received. Use it sparing.
	// unreliable: if failed, failed.
	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator LastAimingRotator;

	ETurningInPlace TurningInPlace;
	void SetTurningInPlaceType(float DeltaTime);

	UPROPERTY(EditAnywhere, Category=Combat)
	class UAnimMontage* FireWeaponMontage;
	UPROPERTY(EditAnywhere, Category=Combat)
	class UAnimMontage* HitReactMontage;
	UPROPERTY(EditAnywhere, Category=Combat)
	class UAnimMontage* EliminateMontage;
	
	void HideCameraIfCharacterClose();
	UPROPERTY(EditAnywhere)
	float CameraHideDistance = 200.f;

	bool bRotateRootBone;
	void ProxyAimOffset();
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyTurnYawThreshold = 0.5f;
	float ProxyAO_Yaw;
	float TimeSinceLastMovementReplication;
	float CalculateSpeed() const;

	/*
	 * Health
	 */
	UPROPERTY(VisibleAnywhere, Category="PlayerStats")
	float MaxHealth = 100.f;
	UPROPERTY(ReplicatedUsing=OnRep_Health, VisibleAnywhere, Category="PlayerStats")
	float Health = 100.f;
	UFUNCTION()
	void OnRep_Health();

	UPROPERTY()
	ABlasterPlayerController* BlasterPlayerController;

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
	
	bool bEliminated = false;
	FTimerHandle RespawnTimer;
	void OnRespawnTimerFinished();
	UPROPERTY(EditDefaultsOnly)
	float RespawnTime = 3.0f;

	/*
	 * Dissolve effect
	 */
	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;
	FOnTimelineFloat DissolveTrack;

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;
	
	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);
	void StartDissolve();

	// Dynamic instance that we can change at runtime
	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;

	// Material instance set on the Blueprint, used with the dynamic material instance
	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* DissolveMaterialInstance;

	// Elim bot particle effects
	UPROPERTY(EditAnywhere)
	UParticleSystem* ElimBotEffect;
	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* ElimBotComponent;
	UPROPERTY(EditAnywhere)
	USoundCue* ElimBotSound;

	UPROPERTY()
	class ABlasterPlayerState* BlasterPlayerState;
	
public:
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsEquippedWeapon();
	bool IsAiming();
	AWeapon* GetEquippedWeapon() const;
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw;}
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch;}
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace;}
	FORCEINLINE UCameraComponent* GetCamera() const { return FollowCamera;}
	FORCEINLINE bool ShouldRotateRootBone() const {return bRotateRootBone;}
	FORCEINLINE bool IsEliminated() const { return bEliminated;}
	FORCEINLINE float GetHealth() const {return Health;}
	FORCEINLINE float GetMaxHealth() const { return MaxHealth;}
};
