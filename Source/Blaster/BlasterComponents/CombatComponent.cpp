// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/Weapon/ProjectileWeapon/Projectile/Projectile.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundCue.h"


UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 400.f;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
		if(Character->GetCamera())
		{
			DefaultFOV = Character->GetCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
		// 只在服务器端初始化弹药数量
		if(Character->HasAuthority())
		{
			InitializeCarriedAmmo();
			
			ThrowGrenadeAmmo = MaxThrowGrenadeAmmo;
			UpdateHUDThrowGrenadeAmmo();
		}
	}
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bIsAiming);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME(UCombatComponent, ThrowGrenadeAmmo);
	DOREPLIFETIME(UCombatComponent, SecondaryWeapon);
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType,FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		LocallyHitTarget = HitResult.ImpactPoint;

		SetHUDCrosshairs(DeltaTime);
		InterpFOV(DeltaTime);
	}
}
void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if(Character == nullptr || Character->Controller == nullptr) return;
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if(Controller)
	{
		HUD = HUD == nullptr ? Cast<ABlasterHUD>(Controller->GetHUD()) : HUD;
		if(HUD)
		{
			if (EquippedWeapon)
			{
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
			}
			else
			{
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
			}
			
			// set CrosshairSpread by some conditions
			// By velocity
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0;
			FVector2D VelocityRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D MapVelocityRange(0.f,1.f);
			CrosshairVelocityFactory = FMath::GetMappedRangeValueClamped(VelocityRange, MapVelocityRange, Velocity.Size());
			// By If in Air
			if(Character->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactory = FMath::FInterpTo(CrosshairInAirFactory, 2.25f, DeltaTime, 2.25f);
			}
			else
			{
				CrosshairInAirFactory = FMath::FInterpTo(CrosshairInAirFactory, 0.f, DeltaTime, 30.f);
			}
			// By Aiming
			if(bIsAiming)
			{
				CrosshairAimFactory = FMath::FInterpTo(CrosshairAimFactory, 0.6f, DeltaTime, 30.f);
			}
			else
			{
				CrosshairAimFactory = FMath::FInterpTo(CrosshairAimFactory, 0.f, DeltaTime, 30.f);
			}
			// By shooting
			CrosshairShootingFactory = FMath::FInterpTo(CrosshairShootingFactory, 0, DeltaTime, 30.f);
			
			HUDPackage.CrosshairSpread = 0.5f + CrosshairVelocityFactory + CrosshairInAirFactory - CrosshairAimFactory + CrosshairShootingFactory;
			
			HUD->SetHUDPackage(HUDPackage);
		}
	}
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if(!EquippedWeapon || !Character) return;
	
	if(bIsAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetAimFOV(), DeltaTime, EquippedWeapon->GetAimChangeSpeed());	
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomOffInterpSpeed);
	}
	Character->GetCamera()->SetFieldOfView(CurrentFOV);
}

bool UCombatComponent::CanFire() const
{
	if(!EquippedWeapon)
		return false;
	if(!EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Reloading && EquippedWeapon->GetWeaponType() == EBlasterWeaponType::EWT_Shotgun)
	{
		return true;
	}
	return !EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied;
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	if(GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
	FVector2D CrosshairLocation(ViewportSize.X / 2.0f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	// line start from camera toward outside
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection);
	if(bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;
		if(Character)
		{
			// move it forward to let trace start from character's forward.
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 100.f);
			// DrawDebugSphere(GetWorld(), Start, 12,16,FColor::Red);
		}
		FVector End = Start + CrosshairWorldDirection * TRACE_LINE_LENGTH;
		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECC_Visibility);
		if(!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End;
		}
		if(TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>())
		{
			HUDPackage.CrosshairColor = FLinearColor::Red;
		}
		else
		{
			HUDPackage.CrosshairColor = FLinearColor::White;
		}
	}
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	if(bFireButtonPressed)
	{
		Fire();
	}
}
void UCombatComponent::Fire()
{
	if(CanFire())
	{
		ServerFire(LocallyHitTarget);
		StartFireTimer();
		if(EquippedWeapon)
		{
			CrosshairShootingFactory = 0.75f;
		}
	}
}
void UCombatComponent::StartFireTimer()
{
	if(!Character || !EquippedWeapon) return;

	Character->GetWorldTimerManager().SetTimer(
		FireTimerHandle,
		this,
		&ThisClass::OnFireTimerFinished,
		EquippedWeapon->GetFireInterval()
	);
}
void UCombatComponent::OnFireTimerFinished()
{
	if(!EquippedWeapon) return;
	
	bCanFire = true;
	if(bFireButtonPressed && EquippedWeapon->IsAutomatic())
	{
		Fire();
	}
	ReloadEmptyWeapon();
}
void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& HitTarget)
{
	if(EquippedWeapon)
	{
		// server called this!
		EquippedWeapon->SpendRound();
	}
	MulticastFire(HitTarget);
}
void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& HitTarget)
{
	if(!EquippedWeapon)
		return;
	if(Character && CombatState == ECombatState::ECS_Reloading && EquippedWeapon->GetWeaponType() == EBlasterWeaponType::EWT_Shotgun)
	{
		// character montage
		Character->PlayFireMontage(bIsAiming);
		// weapon fire animation
		EquippedWeapon->Fire(HitTarget);
		CombatState = ECombatState::ECS_Unoccupied;
		bCanFire = false;
		return;
	}
	bCanFire = false;
	if(Character && CombatState == ECombatState::ECS_Unoccupied)
	{
		// character montage
		Character->PlayFireMontage(bIsAiming);
		// weapon fire animation
		EquippedWeapon->Fire(HitTarget);
	}
}

void UCombatComponent::SetIsAiming(bool bIsAim)
{
	if(Character == nullptr || EquippedWeapon == nullptr)
		return;
	// authorized client do aiming intermediately
	bIsAiming = bIsAim;
	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
	// then tell server to do the result, and tell other simulated client
	Server_SetIsAiming(bIsAim);

	if(Character->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EBlasterWeaponType::EWT_SniperRifle)
	{
		Character->ShowSniperScopeWidget(bIsAiming);
	}
}
void UCombatComponent::Server_SetIsAiming_Implementation(bool bIsAim)
{
	bIsAiming = bIsAim;
	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if(Character == nullptr || WeaponToEquip == nullptr) return;
	if(CombatState != ECombatState::ECS_Unoccupied) return;

	if (EquippedWeapon != nullptr && SecondaryWeapon == nullptr)
	{
		EquipSecondaryWeapon(WeaponToEquip);
	}
	else
	{
		EquipPrimaryWeapon(WeaponToEquip);
	}
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::SwapWeapons()
{
	AWeapon* TempWeapon = EquippedWeapon;
	EquippedWeapon = SecondaryWeapon;
	SecondaryWeapon = TempWeapon;

	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon);
	EquippedWeapon->SetHUDAmmo();
	UpdateCarriedAmmo();
	PlayEquipWeaponSound(EquippedWeapon);

	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttachActorToBackpack(SecondaryWeapon);
}

void UCombatComponent::EquipPrimaryWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;
	DropEquippedWeapon();
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon);
	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->SetHUDAmmo();
	UpdateCarriedAmmo();
	PlayEquipWeaponSound(WeaponToEquip);
	ReloadEmptyWeapon();
}

void UCombatComponent::EquipSecondaryWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;
	SecondaryWeapon = WeaponToEquip;
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttachActorToBackpack(WeaponToEquip);
	PlayEquipWeaponSound(WeaponToEquip);
	SecondaryWeapon->SetOwner(Character);

	if (EquippedWeapon == nullptr) return;
	EquippedWeapon->SetOwner(Character);
}

void UCombatComponent::DropEquippedWeapon()
{
	if(EquippedWeapon != nullptr)
	{
		EquippedWeapon->Dropped();
	}
}

void UCombatComponent::AttachActorToRightHand(AActor* ActorToAttach) const
{
	if(Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr)
		return;
	
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		//因为EquippedWeapon是可复制的，当它被attach到任何对象上时，都会同步复制AActor::OnRep_AttachmentReplication()
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::AttachActorToLeftHand(AActor* ActorToAttach) const
{
	if(Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr)
		return;
	bool bUsePistolSocket = EquippedWeapon->GetWeaponType() == EBlasterWeaponType::EWT_Pistol ||
		EquippedWeapon->GetWeaponType() == EBlasterWeaponType::EWT_SubmachineGun;
	FName SocketName = bUsePistolSocket ? FName(TEXT("PistolSocket")) : FName(TEXT("LeftHandSocket"));
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(SocketName);
	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::AttachActorToBackpack(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr) return;
	const USkeletalMeshSocket* BackpackSocket = Character->GetMesh()->GetSocketByName(FName("BackpackSocket"));
	if (BackpackSocket)
	{
		BackpackSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::UpdateCarriedAmmo()
{
	if(EquippedWeapon == nullptr)return;
	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if(Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

void UCombatComponent::PlayEquipWeaponSound(AWeapon* WeaponToEquip) const
{
	if(Character && WeaponToEquip && WeaponToEquip->GetEquipSound())
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			WeaponToEquip->GetEquipSound(),
			Character->GetActorLocation());
	}
}

void UCombatComponent::ReloadEmptyWeapon()
{
	if(EquippedWeapon && EquippedWeapon->IsEmpty())
	{
		Reload();
	}
}

void UCombatComponent::SetGrenadeVisibility(bool bVisible) const
{
	if(Character && Character->GetGrenadeMesh())
	{
		Character->GetGrenadeMesh()->SetVisibility(bVisible);
	}
}

void UCombatComponent::Reload()
{
	if(CarriedAmmo > 0 && CombatState == ECombatState::ECS_Unoccupied && EquippedWeapon && !EquippedWeapon->IsFull())
	{
		ServerReload();
	}
}

void UCombatComponent::ServerReload_Implementation()
{
	if(!Character || !EquippedWeapon) return;
	
	CombatState = ECombatState::ECS_Reloading;
	HandleReload();
}

void UCombatComponent::HandleReload() const
{
	Character->PlayReloadMontage();
}

int32 UCombatComponent::AmountToReload()
{
	if (EquippedWeapon == nullptr) return 0;
	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();
	
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		return FMath::Clamp(RoomInMag, 0, AmountCarried);
	}
	return 0;
}

void UCombatComponent::UpdateAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	int32 ReloadAmount = AmountToReload();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
	EquippedWeapon->AddAmmo(ReloadAmount);
}

void UCombatComponent::UpdateShotgunAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= 1;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
	EquippedWeapon->AddAmmo(1);
	bCanFire = true;
	if(EquippedWeapon->IsFull() || CarriedAmmo == 0)
	{
		ShotgunReloadJumpToEnd();
	}
}

void UCombatComponent::ShotgunShellReload()
{
	if(Character && Character->HasAuthority())
	{
		UpdateShotgunAmmoValues();
	}
}

void UCombatComponent::ThrowGrenadeFinished()
{
	CombatState = ECombatState::ECS_Unoccupied;
	AttachActorToRightHand(EquippedWeapon);
}

void UCombatComponent::SpawnGrenade()
{
	SetGrenadeVisibility(false);
	if(Character && Character->IsLocallyControlled())
	{
		ServerSpawnGrenade(LocallyHitTarget);
	}
}

bool UCombatComponent::ShouldSwapWeapons()
{
	return (EquippedWeapon != nullptr && SecondaryWeapon != nullptr);
}

void UCombatComponent::ServerSpawnGrenade_Implementation(const FVector_NetQuantize& Target)
{
	if(Character && Character->GetGrenadeMesh() && GrenadeClass && GetWorld())
	{
		FActorSpawnParameters ActorSpawnParameters;
		ActorSpawnParameters.Owner = Character;
		ActorSpawnParameters.Instigator = Character;
		const FVector SpawnLocation = Character->GetGrenadeMesh()->GetComponentLocation();
		const FVector ToTarget = Target - SpawnLocation;
		GetWorld()->SpawnActor<AProjectile>(GrenadeClass, SpawnLocation, ToTarget.Rotation(), ActorSpawnParameters);
	}
}

void UCombatComponent::ShotgunReloadJumpToEnd()
{
	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if(AnimInstance && Character->GetReloadMontage())
	{
		AnimInstance->Montage_JumpToSection("ShotgunEnd");
	}
}

void UCombatComponent::ThrowGrenade()
{
	if(ThrowGrenadeAmmo<= 0) return;
	if(CombatState != ECombatState::ECS_Unoccupied || EquippedWeapon == nullptr )
		return;

	// authorized client do first
	CombatState = ECombatState::ECS_ThrowGrenade;
	if(Character)
	{
		Character->PlayThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		SetGrenadeVisibility(true);
	}
	if(Character && !Character->HasAuthority())
	{
		ServerThrowGrenade();
	}
	ThrowGrenadeAmmo = FMath::Clamp(ThrowGrenadeAmmo - 1, 0, MaxThrowGrenadeAmmo);
	UpdateHUDThrowGrenadeAmmo();
}

void UCombatComponent::PickupAmmo(int32 AddAmmoNum, EBlasterWeaponType AddWeaponType)
{
	if(CarriedAmmoMap.Contains(AddWeaponType))
	{
		CarriedAmmoMap[AddWeaponType] = FMath::Clamp(CarriedAmmoMap[AddWeaponType] + AddAmmoNum, 0, MaxCarriedAmmo);
		UpdateCarriedAmmo();
	}
	if(EquippedWeapon && EquippedWeapon->GetWeaponType() == AddWeaponType && EquippedWeapon->IsEmpty())
	{
		Reload();
	}
}

void UCombatComponent::OnRep_ThrowGrenadeAmmo()
{
	UpdateHUDThrowGrenadeAmmo();
}

void UCombatComponent::UpdateHUDThrowGrenadeAmmo()
{
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if(Controller)
	{
		Controller->SetHUDGrenadeAmmo(ThrowGrenadeAmmo);
	}
}

void UCombatComponent::ServerThrowGrenade_Implementation()
{
	if(ThrowGrenadeAmmo <= 0) return;
	CombatState = ECombatState::ECS_ThrowGrenade;
	if(Character)
	{
		Character->PlayThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		SetGrenadeVisibility(true);
	}
	ThrowGrenadeAmmo = FMath::Clamp(ThrowGrenadeAmmo - 1, 0, MaxThrowGrenadeAmmo);
	UpdateHUDThrowGrenadeAmmo();
}

void UCombatComponent::ReloadFinished()
{
	if(Character && Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValues();
	}
	if(bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::OnRep_EquipWeapon() const
{
	if(EquippedWeapon && Character)
	{
		// it can be called twice
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);  
		AttachActorToRightHand(EquippedWeapon);
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
		PlayEquipWeaponSound(EquippedWeapon);
		EquippedWeapon->EnableCustomDepth(false);
		EquippedWeapon->SetHUDAmmo();
	}
}

void UCombatComponent::OnRep_SecondaryWeapon()
{
	if (SecondaryWeapon && Character)
	{
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
		AttachActorToBackpack(SecondaryWeapon);
		PlayEquipWeaponSound(EquippedWeapon);
	}
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	//该复制仅发生在服务器向当前component的持有者，当更新时，同步更新owner的HUD
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if(Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
	bool bJumpToShotgunEnd = CombatState == ECombatState::ECS_Reloading &&
		EquippedWeapon != nullptr && EquippedWeapon->GetWeaponType() == EBlasterWeaponType::EWT_Shotgun && CarriedAmmo == 0;
	if(bJumpToShotgunEnd)
	{
		ShotgunReloadJumpToEnd();
	}
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EBlasterWeaponType::EWT_AssaultRifle, StartingAssaultRifleAmmo);
	CarriedAmmoMap.Emplace(EBlasterWeaponType::EWT_RocketLauncher, StartingRocketAmmo);
	CarriedAmmoMap.Emplace(EBlasterWeaponType::EWT_Pistol, StartingPistolAmmo);
	CarriedAmmoMap.Emplace(EBlasterWeaponType::EWT_SubmachineGun, StartingSMGAmmo);
	CarriedAmmoMap.Emplace(EBlasterWeaponType::EWT_Shotgun, StartingShotgunAmmo);
	CarriedAmmoMap.Emplace(EBlasterWeaponType::EWT_SniperRifle, StartingSniperAmmo);
	CarriedAmmoMap.Emplace(EBlasterWeaponType::EWT_GrenadeLauncher, StartingGrenadeLauncherAmmo);
}

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Reloading:
		HandleReload();
		break;
	case ECombatState::ECS_Unoccupied:
		if(bFireButtonPressed)
		{
			Fire();
		}
		break;
	case ECombatState::ECS_ThrowGrenade:
		if(Character)
		{
			Character->PlayThrowGrenadeMontage();
		}
		SetGrenadeVisibility(true);
		break;
	}
}