// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/BlasterTypes/TeamTypes.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "Blaster/Interface/InteractWithCrosshairsInterface.h"
#include "Blaster/Interface/PawnInterface.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Character.h"
#include "BlasterCharacter.generated.h"

struct FSKGProceduralPoseReplicationData;
class USKGAttachmentManagerComponent;
class USKGAnimInstance;
class USKGShooterFrameworkAnimInstance;
class USKGShooterPawnComponent;
class UNiagaraSystem;
class UNiagaraComponent;
class USoundCue;
class UTimelineComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerLeft);

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter, public IInteractWithCrosshairsInterface, public IPawnInterface
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

	// IPawnInterface
	virtual bool IsDead() override;
	virtual void PickUpActor(AActor* Actor) override;
	//

	// SKG
	UPROPERTY(Replicated)
	AActor* FirearmOnBack;
	UPROPERTY(Replicated)
	AActor* PistolInHolster;
	UPROPERTY(ReplicatedUsing="OnRep_FirearmToSwitchTo")
	AActor* FirearmToSwitchTo;
	UFUNCTION()
	void OnRep_FirearmToSwitchTo();
	UPROPERTY()
	USKGAnimInstance* SKGAnimInstance;
	UPROPERTY(EditAnywhere)
	FName FirearmAttachSockName = FName("ik_hand_gun");
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> InitialFirearm;
	
	bool IsHoldingActor() const;
	void AttachEquipActor(AActor* InFirearmToEquip);
	bool AttachNonEquippedActor(AActor* Actor, FName& OutSocketName);
	FAttachmentTransformRules CreateFirearmAttachmentRules() const;
	bool ShouldSwapWeapons() const;
	void SwapWeapons();
	void StartAiming();
	void StopAiming();
	USKGAnimInstance* GetSKGAnimInstance();
	UFUNCTION(Server, Reliable)
	void Server_SwapWeapons();
	
	UFUNCTION()
	void OnHeldActorSet(AActor* NewHeldActor, AActor* OldHeldActor);
	UFUNCTION()
	void OnPoseComplete(const FSKGProceduralPoseReplicationData& CurrentPoseData);
	void BindToFirearmEvents(AActor* Actor);
	void UnbindFromFirearmEvents(AActor* Actor);
	UFUNCTION()
	void OnFired(const FRotator& RecoilControlRotationMultiplier, const FVector& RecoilLocationMultiplier,const FRotator& RecoilRotationMultiplier, const FAnimationMontageData& AnimationData);
	UFUNCTION()
	void OnReloading(UAnimMontage* Montage);
	UFUNCTION()
	void OnActionCycled(UAnimMontage* Montage);
	UFUNCTION()
	void OnMagnifierFlipped(UAnimMontage* Montage, FName SectionName);

	void SetOnlyTickPoseWhenRenderedDedicated();
	void SetupAnimationBudgetAllocator();
	void SpawnInitialFirearm();
	void BindToAnimBPEvent();
	void BindToPlayerControllerEvent();
	UFUNCTION()
	void UnequipComplete();
	UFUNCTION()
	void EquipComplete();
	//
	
	virtual void PostInitializeComponents() override;
	void PlayFireMontage(bool bAiming);
	void PlayReloadMontage();
	void PlayElimMontage() const;
	void PlayThrowGrenadeMontage() const;
	
	FVector GetHitTarget() const;
	virtual void OnRep_ReplicatedMovement() override;

	void Eliminate(bool bLeftGame);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastEliminate(bool bLeftGame);

	void SetIsInCoolDownState(bool NewState);

	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bIsAiming);

	UPROPERTY()
	TMap<FName, class UBoxComponent*> HitCollisionBoxes;

	FOnPlayerLeft OnPlayerLeft;
	
	UPROPERTY()
	bool bPlayerLeft;
	UFUNCTION(Server, Reliable)
	void ServerPlayerLeftGame();
	UFUNCTION(NetMulticast, Reliable)
	void OnLeadTheCrown();
	UFUNCTION(NetMulticast, Reliable)
	void OnLoseTheCrown();

	void SetTeamColor(ETeamTypes Team);
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
	void StartFire();
	void StopFire();
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
	class ULagCompensationComponent* LagCompensation;
	
	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* GrenadeMesh;

	// SKG
	UPROPERTY()
	bool bIsPerformingAction = false;
	UPROPERTY()
	bool bWantsToAim = false;
	UPROPERTY(VisibleAnywhere)
	USKGShooterPawnComponent* ShooterPawnComponent;
	UPROPERTY(VisibleAnywhere)
	USKGAttachmentManagerComponent* AttachmentManagerComponent;
	// UPROPERTY(VisibleAnywhere)
	// UAIPerceptionStimuliSourceComponent* PerceptionStimuliSourceComponent;
	UPROPERTY(EditAnywhere)
	TSubclassOf<USKGShooterFrameworkAnimInstance> SKGAnimLayerArmed;
	UPROPERTY(EditAnywhere)
	TSubclassOf<USKGShooterFrameworkAnimInstance> SKGAnimLayerUnarmed;

	//

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

	UPROPERTY(EditAnywhere)
	UNiagaraSystem* CrownCompSystem;
	UPROPERTY()
	UNiagaraComponent* CrownComponent;
	
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
	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstance* DissolveMaterialInst;

	/*
	 * Team Color
	 */
	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* RedDissolveMatInst;

	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* RedMaterial;

	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* BlueDissolveMatInst;

	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* BlueMaterial;

	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* OriginalMaterial;

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

	/** 
	* Hit boxes used for server-side rewind
	*/

	UPROPERTY(EditAnywhere)
	class UBoxComponent* head;

	UPROPERTY(EditAnywhere)
	UBoxComponent* pelvis;

	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_02;

	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_03;

	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* backpack;

	UPROPERTY(EditAnywhere)
	UBoxComponent* blanket;

	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_r;
	
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
	FORCEINLINE USKGShooterPawnComponent* GetShooterPawnComp() const {return ShooterPawnComponent;}
	FORCEINLINE UStaticMeshComponent* GetGrenadeMesh() const { return GrenadeMesh;}
	FORCEINLINE UBuffComponent* GetBuff() const { return Buff;}
	FORCEINLINE float GetShield() const { return Shield; }
	FORCEINLINE void SetShield(float Amount) { Shield = Amount; }
	FORCEINLINE float GetMaxShield() const { return MaxShield; }
	bool IsLocallyReloading();
	FORCEINLINE ULagCompensationComponent* GetLagCompensation() const { return LagCompensation; }
	FORCEINLINE bool IsPerformingAction() const {return bIsPerformingAction;}
	FORCEINLINE void SetIsPerformingAction(bool bNewState) { bIsPerformingAction = bNewState;} 
};
