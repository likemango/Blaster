// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Components/ActorComponent.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "CombatComponent.generated.h"

#define TRACE_LINE_LENGTH 80000.0f;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();
	friend class ABlasterCharacter;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	// only execute on the server
	void EquipWeapon(class AWeapon* WeaponToEquip);
	void Reload();
	
protected:
	virtual void BeginPlay() override;

	void SetIsAiming(bool bIsAim);

	UFUNCTION(Server, Reliable)
	void Server_SetIsAiming(bool bIsAim);

	UFUNCTION()
	void OnRep_EquipWeapon() const;
	void Fire();

	void FireButtonPressed(bool bPressed);
	
	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& HitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& HitTarget);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerReload();

private:
	UPROPERTY()
	class ABlasterCharacter* Character;
	UPROPERTY()
	class ABlasterPlayerController* Controller;
	UPROPERTY()
	class ABlasterHUD* HUD;

	UPROPERTY(ReplicatedUsing=OnRep_EquipWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(EditAnywhere)
	class USoundCue* EquipWeaponSound;

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
	int32 StartingARAmmo = 30;
	// because hash algorithm result changed on server and client, can't be replicated!
	TMap<EBlasterWeaponType, int32> CarriedAmmoMap;
	void InitializeCarriedAmmo();

	UPROPERTY(ReplicatedUsing=OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	void HandleReload() const;

	int32 AmountToReload();

	void UpdateAmmoValues();
	
	UFUNCTION(BlueprintCallable)
	void FinishReloading();

};
