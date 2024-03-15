// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Components/ActorComponent.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "CombatComponent.generated.h"

class AProjectile;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();
	friend class ABlasterCharacter;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void EquipWeapon(class AWeapon* WeaponToEquip);
	void SwapWeapons();
	void Reload();
	void ShotgunReloadJumpToEnd();
	void ThrowGrenade();
	void PickupAmmo(int32 AddAmmoNum, EBlasterWeaponType AddWeaponType);
protected:
	virtual void BeginPlay() override;

	void SetIsAiming(bool bIsAim);

	UFUNCTION(Server, Reliable)
	void Server_SetIsAiming(bool bIsAim);

	UFUNCTION()
	void OnRep_EquipWeapon() const;
	
	UFUNCTION()
	void OnRep_SecondaryWeapon();
	
	void Fire();

	void FireButtonPressed(bool bPressed);
	
	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& HitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& HitTarget);

	void LocalFire(const FVector_NetQuantize& HitTarget);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerReload();

	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();

	void DropEquippedWeapon();
	void AttachActorToRightHand(AActor* ActorToAttach) const;
	void AttachActorToLeftHand(AActor* ActorToAttach) const;
	void AttachActorToBackpack(AActor* ActorToAttach);
	void UpdateCarriedAmmo();
	void PlayEquipWeaponSound(AWeapon* WeaponToEquip) const;
	void ReloadEmptyWeapon();
	void SetGrenadeVisibility(bool bVisible) const;
	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);
	void EquipSecondaryWeapon(AWeapon* WeaponToEquip);
	
private:
	UPROPERTY()
	class ABlasterCharacter* Character;
	UPROPERTY()
	class ABlasterPlayerController* Controller;
	UPROPERTY()
	class ABlasterHUD* HUD;

	UPROPERTY(ReplicatedUsing=OnRep_EquipWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
	AWeapon* SecondaryWeapon;
	
	UPROPERTY(Replicated)
	bool bIsAiming;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;
	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed;

	// Crosshair
	float CrosshairVelocityFactory;
	float CrosshairInAirFactory;
	float CrosshairAimFactory;
	float CrosshairShootingFactory;
	FHUDPackage HUDPackage;
	FVector_NetQuantize LocallyHitTarget;

	// Automatic fire
	FTimerHandle FireTimerHandle;
	bool bCanFire = true;
	void StartFireTimer();
	void OnFireTimerFinished();
	
	// Aiming and FOV
	// Field of view when not aiming; set to the camera's base FOV in BeginPlay
	float DefaultFOV;
	float CurrentFOV;
	// When not zoom, interp back to default FOV. Same for all weapon.
	UPROPERTY(EditAnywhere)
	float ZoomOffInterpSpeed = 20.f;
	void InterpFOV(float DeltaTime);

	bool CanFire() const;

	//当前持有的武器的弹药
	UPROPERTY(ReplicatedUsing=OnRep_CarriedAmmo)
	int32 CarriedAmmo;
	UFUNCTION()
	void OnRep_CarriedAmmo();

	UPROPERTY(EditAnywhere)
	TSubclassOf<AProjectile> GrenadeClass;
	UPROPERTY(EditAnywhere)
	int32 StartingAssaultRifleAmmo = 0; // Assult Rifle
	UPROPERTY(EditAnywhere)
	int32 StartingRocketAmmo = 0; // rocket
	UPROPERTY(EditAnywhere)
	int32 StartingPistolAmmo = 0;
	UPROPERTY(EditAnywhere)
	int32 StartingSMGAmmo = 0;
	UPROPERTY(EditAnywhere)
	int32 StartingShotgunAmmo = 0;
	UPROPERTY(EditAnywhere)
	int32 StartingSniperAmmo = 0;
	UPROPERTY(EditAnywhere)
	int32 StartingGrenadeLauncherAmmo = 0;

	UPROPERTY(ReplicatedUsing=OnRep_ThrowGrenadeAmmo)
	int32 ThrowGrenadeAmmo = 0;
	UFUNCTION()
	void OnRep_ThrowGrenadeAmmo();
	UPROPERTY(EditAnywhere)
	int32 MaxThrowGrenadeAmmo = 4;
	void UpdateHUDThrowGrenadeAmmo();
	
	// because hash algorithm result changed on server and client, can't be replicated!
	TMap<EBlasterWeaponType, int32> CarriedAmmoMap;
	UPROPERTY(EditAnywhere)
	int32 MaxCarriedAmmo = 500;
	void InitializeCarriedAmmo();

	UPROPERTY(ReplicatedUsing=OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	void HandleReload() const;

	int32 AmountToReload();

	void UpdateAmmoValues();
	void UpdateShotgunAmmoValues();
	
	UFUNCTION(BlueprintCallable)
	void ReloadFinished();

	UFUNCTION(BlueprintCallable)
	void ShotgunShellReload();

	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();

	UFUNCTION(BlueprintCallable)
	void SpawnGrenade();

	UFUNCTION(Server, Reliable)
	void ServerSpawnGrenade(const FVector_NetQuantize& Target);

public:	
	FORCEINLINE int32 GetGrenades() const { return ThrowGrenadeAmmo; }
	bool ShouldSwapWeapons();
};
