// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"


UENUM()
enum class EWeaponState : int8
{
	EWS_Initial UMETA(DisplayName = "Iniitial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_MAX UMETA(DisplayName = "DefaultMAX")
	
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
	virtual void OnRep_Owner() override;
	void SetHUDAmmo();

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
	
protected:
	virtual void BeginPlay() override;

	// OverLappedComponent: component who bound this event
	// OtherComp: the component this comp bind
	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverLappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	virtual void OnSphereOverlapEnd(UPrimitiveComponent* OverLappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

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

	/*
	 * Weapon Ammo Amount
	 */
	UPROPERTY(EditAnywhere, ReplicatedUsing=OnRep_WeaponAmmo)
	int32 WeaponAmmo;
	UPROPERTY(EditAnywhere)
	int32 MagCapacity;
	UFUNCTION()
	void OnRep_WeaponAmmo();
	void SpendRound();

	UPROPERTY()
	class ABlasterCharacter* BlasterCharacter;

	UPROPERTY()
	class ABlasterPlayerController* BlasterPlayerController;
	
public:
	void SetWeaponState(EWeaponState NewState);
	
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere;}
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh;}
	FORCEINLINE float GetAimFOV() const { return ZoomedFOV;}
	FORCEINLINE float GetAimChangeSpeed() const {return ZoomInterpSpeed;}
	FORCEINLINE bool IsAutomatic() const { return bAutomatic;}
	FORCEINLINE float GetFireInterval() const { return FireInterval;}
};
