// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponTypes.h"
#include "Blaster/Interface/InteractInterface.h"
#include "Blaster/Interface/WeaponInterface.h"
#include "Components/SKGMuzzleComponent.h"
#include "Weapon.generated.h"


class UNiagaraSystem;
class USKGPDAProjectile;
class USKGAttachmentManagerComponent;
class USKGOffHandIKComponent;
class USKGProceduralAnimComponent;
class USKGFirearmComponent;
class USoundCue;

UENUM()
enum class EWeaponState : int8
{
	EWS_Initial UMETA(DisplayName = "Iniitial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_EquippedSecondary UMETA(DisplayName = "Equipped Secondary"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_MAX UMETA(DisplayName = "DefaultMAX")
	
};

UENUM(BlueprintType)
enum class EFireType : uint8
{
	EFT_HitScan UMETA(DisplayName = "Hit Scan Weapon"),
	EFT_Projectile UMETA(DisplayName = "Projectile Weapon"),
	EFT_Shotgun UMETA(DisplayName = "Shotgun Weapon"),

	EFT_MAX UMETA(DisplayName = "DefaultMAX")
};

USTRUCT(BlueprintType)
struct FProjectileReplicationData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	bool bIsFiring;
	UPROPERTY(BlueprintReadOnly)
	FSKGMuzzleTransform MuzzleTransform;
};

USTRUCT(BlueprintType)
struct FAnimationMontageData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* Montage;	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName Section;
};

USTRUCT(BlueprintType)
struct FPickedUpRepData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	USceneComponent* SceneComponent;
	UPROPERTY(BlueprintReadOnly)
	FName Socket;
	UPROPERTY(BlueprintReadOnly)
	uint8 Count;
};

USTRUCT(BlueprintType)
struct FLocationRotationData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FVector_NetQuantize Location;
	UPROPERTY(BlueprintReadOnly)
	FVector_NetQuantize Rotation;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnFired, const FRotator&, RecoilControlRotationMultiplier, const FVector&, RecoilLocationMultiplier,
	const FRotator&, RecoilRotationMultiplier, const FAnimationMontageData&, AnimationData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReload, UAnimMontage*, Montage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActionCycled, UAnimMontage*, Montage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMagnifierFlipped, UAnimMontage*, Montage, FName, SectionName);

UCLASS()
class BLASTER_API AWeapon : public AActor, public IWeaponInterface, public IInteractInterface
{
	GENERATED_BODY()

public:
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void ShowPickupWidget(bool bShow);
	virtual void Fire(const FVector& HitTarget);
	void Dropped();
	void AddAmmo(int32 AmmoToAdd);
	virtual void OnRep_Owner() override;
	void SetHUDAmmo();

	void SpendRound();
	void EnableCustomDepth(bool bEnable) const;

	UPROPERTY(EditAnywhere)
	EFireType FireType;
	UPROPERTY(EditAnywhere)
	EBlasterWeaponType WeaponType;
	UPROPERTY(EditAnywhere)
	EBlasterWeaponPriorityType PriorityType;

	// SKG
	void ConstructFromPreset();
	void InterpolateActorToTargetTransform(float DeltaTime);
	void SetupFireRate(float InFireRate);
	UFUNCTION()
	void OnAttachmentComponentAttachmentAdded(AActor* Attachment);
	UFUNCTION()
	void OnAttachmentComponentAttachmentRemoved(AActor* Attachment);
	UFUNCTION()
	void OnProceduralAnimComponentsUpdated();
	UFUNCTION()
	void OnAttachmentChanged();
	UFUNCTION()
	void OnMagnifierFlippedCallback(UAnimMontage* Montage, FName SectionName);

	UPROPERTY()
	FOnFired OnFired;
	UPROPERTY()
	FOnReload OnReload;
	UPROPERTY()
	FOnActionCycled OnActionCycled;
	UPROPERTY()
	FOnMagnifierFlipped OnMagnifierFlipped;
	//
	
	// IWeaponInterface
	virtual void Fire() override;
	virtual void StopFire() override;
	virtual bool CanFire() override;
	virtual void Reload() override;
	UFUNCTION(BlueprintCallable)
	virtual void ReloadComplete() override;
	virtual void CycleFireMode() override;
	virtual EBlasterWeaponPriorityType GetWeaponPriorityType() override;
	virtual void ActionFinishedCycling() override;
	//

	// IInteractInterface
	virtual FName GetAttachSocket_Implementation() override;
	virtual void Interact_Implementation(APawn* Pawn) override;
	virtual bool IsPickup_Implementation() override;
	virtual void OnPickup_Implementation(USceneComponent* Component, FName Socket) override;
	virtual void OnDrop_Implementation() override;
	virtual bool CanBePickedUp_Implementation() override;
	//
	
	UFUNCTION(Server, Reliable)
	void Server_Fire(const FSKGMuzzleTransform& MuzzleTransform);
	void FireLocal();
	UFUNCTION(Server, Reliable)
	void Server_StopFire();
	UFUNCTION(Server, Reliable)
	void Server_Reload();
	UFUNCTION(Server, Reliable)
	void Server_CycleFireMode();
	void OnPickedup(USceneComponent* Component, FName Socket);
	void StopTickAndPhysics();
	void StartTickAndPhysics();
	void ReplicateDropPhysics();
	void SetActorAtTargetTransform();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString PresetString;
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Ammo")
	int32 AmmoRemaining;
	UPROPERTY(BlueprintReadOnly, Category="Ammo")
	AActor* AmmoSource;
	UPROPERTY(EditAnywhere, Category="Shooting|FireMode")
	int32 FireModeIndex = 1;
	UPROPERTY(EditAnywhere, Category="Shooting|FireMode")
	TArray<FGameplayTag> FireModes;

	UPROPERTY(EditAnywhere, Category="Interaction")
	FName AttachSocket = FName("S_Weapon_Back");

	// WeaponDrop
	UPROPERTY()
	FTimerHandle TActorDroppedPhysicsHandle;
	UPROPERTY()
	FTimerHandle TActorDroppedReplicationTimeLimitHandle;
	UPROPERTY()
	FVector DroppedActorTargetLocation = FVector::ZeroVector;
	UPROPERTY()
	FRotator DroppedActorTargetRotator = FRotator::ZeroRotator;
	
	UPROPERTY(EditAnywhere, Category="Interaction")
	bool bCanGetPickedup = true;
	UPROPERTY(ReplicatedUsing="OnRep_PickedUpData")
	FPickedUpRepData PickedUpData;
	UFUNCTION()
	void OnRep_PickedUpData();
	UPROPERTY(ReplicatedUsing="OnRep_DropPhysicsData")
	FLocationRotationData DropPhysicsData;
	UFUNCTION()
	void OnRep_DropPhysicsData();
	
	/**
	* Trace end with scatter
	*/
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool bUseScatter = true;
	
	FVector TraceEndWithScatter(const FVector& HitTarget);

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float DistanceToSphere = 800.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float SphereRadius = 75.f;
	
	// Textures for the weapon crosshairs
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	class UTexture2D* CrosshairsCenter;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsLeft;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsRight;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsTop;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsBottom;

	UPROPERTY()
	bool bDestroyWeapon = false;
	
protected:
	virtual void BeginPlay() override;
	virtual void OnWeaponStateSet();

	virtual void OnEquipped();
	virtual void OnDropped();
	virtual void OnEquippedSecondary();
	
	UFUNCTION()
	void OnPingTooHigh(bool bHighPing);
	
	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverLappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	virtual void OnSphereOverlapEnd(UPrimitiveComponent* OverLappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

	UPROPERTY(Replicated, EditAnywhere)
	bool bUseServerSideRewind = false;

	UPROPERTY()
	class ABlasterCharacter* BlasterCharacter;

	UPROPERTY()
	class ABlasterPlayerController* BlasterPlayerController;
	
private:
	// SKG components
	UPROPERTY(VisibleAnywhere, Category="SKGComponents")
	USkeletalMeshComponent* SkeletalMeshComponent;
	UPROPERTY(VisibleAnywhere, Category="SKGComponents")
	USKGFirearmComponent* FirearmComponent;
	UPROPERTY(VisibleAnywhere, Category="SKGComponents")
	USKGProceduralAnimComponent* ProceduralAnimComponent;
	UPROPERTY(VisibleAnywhere, Category="SKGComponents")
	USKGOffHandIKComponent* OffHandIKComponent; 
	UPROPERTY(VisibleAnywhere, Category="SKGComponents")
	USKGAttachmentManagerComponent* AttachmentManagerComponent;
	//
	FSKGMuzzleTransform GetMuzzleTransform() const;
	void ReplicateFireData(const FSKGMuzzleTransform& MuzzleTransform, bool bIsFiring);
	UPROPERTY(EditAnywhere, ReplicatedUsing="OnRep_ProjectileReplicationData", Category="Shooting|Replicated")
	FProjectileReplicationData ProjectileReplicationData;
	UFUNCTION()
	void OnRep_ProjectileReplicationData();
	bool IsLocallyControlled();
	void FireRemote();
	void FireShot(FTransform MuzzleTransform, const TFunction<void()>& Func);
	void LaunchProjectile(USKGPDAProjectile* Projectile, FTransform MuzzleTransform);
	UFUNCTION()
	void OnImpact(const FHitResult& HitResult, const FVector& Direction, const int32 ProjectileID);
	float CalculateDamageFromProjectile(int32 ProjectileID);
	void PerformImpactEffect(const FHitResult& HitResult);
	void PlayFireAnimation();
	void PlayShotEffects();
	USoundCue* GetShotSound() const;
	bool IsSuppressed() const;
	UNiagaraSystem* GetShotMuzzleParticle() const;
	void PlayReloadMontage();

	UPROPERTY(EditAnywhere, Category="Animations")
	UAnimMontage* CharacterShootMontage;
	UPROPERTY(EditAnywhere, Category="Animations")
	UAnimMontage* FirearmShootMontage;
	UPROPERTY(EditAnywhere, Category="Animations")
	UAnimMontage* CharacterReloadMontage;
	UPROPERTY(EditAnywhere, Category="Animations")
	UAnimMontage* FirearmReloadMontage;
	
	UPROPERTY(EditAnywhere, Category="Shooting|Effects")
	USoundCue* ShotSound;
	UPROPERTY(EditAnywhere, Category="Shooting|Effects")
	USoundCue* SuppressedShotSound;
	UPROPERTY(EditAnywhere, Category="Shooting|Effects")
	UNiagaraSystem* ShotMuzzleParticle;
	UPROPERTY(EditAnywhere, Category="Shooting|Effects")
	UNiagaraSystem* SuppressedShotMuzzleParticle;
	UPROPERTY(EditAnywhere, ReplicatedUsing="OnRep_bIsReloading", Category="Replicated")
	bool bIsReloading;
	UFUNCTION()
	void OnRep_bIsReloading();

	UPROPERTY(EditAnywhere, Category="Shooting|Recoil")
	FRotator ControlRotationMultiplier;
	UPROPERTY(EditAnywhere, Category="Shooting|Recoil")
	FVector RecoilLocationMultiplier;
	UPROPERTY(EditAnywhere, Category="Shooting|Recoil")
	FRotator RecoilRotationMultiplier;
	
	UPROPERTY()
	FTimerHandle TFullAutoHandle;
	UPROPERTY(Replicated)
	USKGPDAProjectile* FirearmProjectile;
	UPROPERTY(Replicated)
	FGameplayTag CurrentFireModeTag;
	UPROPERTY()
	float LastShotTime;
	UPROPERTY(EditAnywhere, Category="Shooting|Time")
	float FireRate = 600.f;
	UPROPERTY(EditAnywhere, Category="Shooting|Time")
	float FireRateDelay = 650.f;
	
	UPROPERTY(VisibleAnywhere, Category="Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category="Weapon Properties")
	class USphereComponent* AreaSphere;

	UPROPERTY(VisibleAnywhere, Category="Weapon Properties")
	class UWidgetComponent* PickupWidget;
	
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_WeaponState)
	EWeaponState WeaponState;

	UPROPERTY(EditAnywhere, Category=Combat)
	bool bAutomatic = true;

	UPROPERTY(EditAnywhere, Category=Combat)
	float FireInterval = 0.1f;

	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(EditAnywhere)
	UAnimationAsset* FireAnimation;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class ACasing> CasingClass;

	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.f;

	UPROPERTY(EditAnywhere)
	USoundCue* EquipSound;
	/*
	 * Weapon Ammo Amount
	 */
	UPROPERTY(EditAnywhere)
	int32 Ammo;

	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);

	UFUNCTION(Client, Reliable)
	void ClientAddAmmo(int32 AmmoToAdd);

	// The number of unprocessed server requests for Ammo.
	// Incremented in SpendRound, decremented in ClientUpdateAmmo.
	int32 Sequence = 0;
	
	UPROPERTY(EditAnywhere)
	int32 MagCapacity;

public:
	void SetWeaponState(EWeaponState NewState);
	
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere;}
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh;}
	FORCEINLINE float GetAimFOV() const { return ZoomedFOV;}
	FORCEINLINE float GetAimChangeSpeed() const {return ZoomInterpSpeed;}
	FORCEINLINE bool IsAutomatic() const { return bAutomatic;}
	FORCEINLINE float GetFireInterval() const { return FireInterval;}
	bool IsEmpty() const;
	bool IsFull() const;
	FORCEINLINE EBlasterWeaponType GetWeaponType() const { return WeaponType;}
	FORCEINLINE int32 GetAmmo() const { return Ammo;}
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity;}
	FORCEINLINE USoundCue* GetEquipSound() const { return EquipSound;}
	FORCEINLINE float GetDamage() const { return Damage;}

};
