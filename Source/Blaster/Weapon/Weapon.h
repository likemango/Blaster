// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponTypes.h"
#include "Weapon.generated.h"


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

UCLASS()
class BLASTER_API AWeapon : public AActor
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
	virtual void OnSphereOverlap(UPrimitiveComponent* OverLappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	virtual void OnSphereOverlapEnd(UPrimitiveComponent* OverLappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

	UPROPERTY(EditAnywhere)
	bool bUseServerSideRewind = false;

	UPROPERTY()
	class ABlasterCharacter* BlasterCharacter;

	UPROPERTY()
	class ABlasterPlayerController* BlasterPlayerController;
	
private:
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

	UPROPERTY(EditAnywhere)
	EBlasterWeaponType WeaponType;

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
	FORCEINLINE float GetDamage() { return Damage;}

};
