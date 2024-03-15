// Fill out your copyright notice in the Description page of Project Settings.

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
	void UpdateHUDShield();
	void UpdateHUDAmmo();
	void SpawnDefaultWeapon();
	
	virtual void PostInitializeComponents() override;
	void PlayFireMontage(bool bAiming);
	void PlayReloadMontage();
	void PlayElimMontage() const;
	void PlayThrowGrenadeMontage() const;
	
	FVector GetHitTarget() const;
	virtual void OnRep_ReplicatedMovement() override;

	void Eliminate();
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastEliminate();

	void SetIsInCoolDownState(bool NewState);

	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bIsAiming);
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
	void ReloadButtonPressed();
	void ThrowGrenadePressed();
	void AimPressed();
	void AimReleased();
	void CalculateAO_Pitch();
	void CalculateAimOffset(float DeltaTime);
	virtual void Jump() override;
	void FirePressed();
	void FireReleased();
	void PlayHitReactMontage() const;
	// poll init for some classes which is not ready when character BeginPlay()
	void PollInit();
private:
	UPROPERTY(VisibleAnywhere, Category=Camera)
	class USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, Category=Camera)
	class UCameraComponent* FollowCamera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	class UWidgetComponent* WidgetComponent;
	
	UPROPERTY(Replicated)
	class AWeapon* OverlappingWeapon;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	class UCombatComponent* Combat;
	
	UPROPERTY(VisibleAnywhere)
	class UBuffComponent* Buff;
	
	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* GrenadeMesh;

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

	/*
	 * Animation Montage
	 */
	UPROPERTY(EditAnywhere, Category=Combat)
	class UAnimMontage* FireWeaponMontage;
	UPROPERTY(EditAnywhere, Category=Combat)
	class UAnimMontage* ReloadMontage;
	UPROPERTY(EditAnywhere, Category=Combat)
	class UAnimMontage* HitReactMontage;
	UPROPERTY(EditAnywhere, Category=Combat)
	class UAnimMontage* EliminateMontage;
	UPROPERTY(EditAnywhere, Category=Combat)
	class UAnimMontage* ThrowGrenadeMontage;
	
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
	void OnRep_Health(float LastHealth);

	/*
	 * Player shield
	 */
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxShield = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Shield, EditAnywhere, Category = "Player Stats")
	float Shield = 0.f;

	UFUNCTION()
	void OnRep_Shield(float LastShield);

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

	// at this moment, character is undefeatable!
	UPROPERTY(Replicated)
	bool bInCoolDownTime;

	/** 
	* Default weapon
	*/

	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> DefaultWeaponClass;

	void DropOrDestroyWeapon(AWeapon* Weapon);
	void DropOrDestroyWeapons();
	
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
	FORCEINLINE void SetHealth(float Amount) { Health = Amount; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth;}
	ECombatState GetCombatState() const;
	FORCEINLINE UAnimMontage* GetReloadMontage() const{ return ReloadMontage;}
	FORCEINLINE UCombatComponent* GetCombat() const { return Combat;}
	FORCEINLINE UStaticMeshComponent* GetGrenadeMesh() const { return GrenadeMesh;}
	FORCEINLINE UBuffComponent* GetBuff() const { return Buff;}
	FORCEINLINE float GetShield() const { return Shield; }
	FORCEINLINE void SetShield(float Amount) { Shield = Amount; }
	FORCEINLINE float GetMaxShield() const { return MaxShield; }
};
