// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/Weapon/Weapon.h"
#include "Camera/CameraComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


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
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType,FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(Character->IsLocallyControlled())
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
	if(!EquippedWeapon) return false;
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
	bIsAiming = bIsAim;
	Server_SetIsAiming(bIsAim);
	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
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
	if (Character == nullptr || WeaponToEquip == nullptr) return;

	if(EquippedWeapon != nullptr)
	{
		EquippedWeapon->Dropped();
	}
	EquippedWeapon = WeaponToEquip;

	// 1. send to client(set weapon state)
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	// 2. send to client(attach actor), no sure which get to client first!
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		//因为EquippedWeapon是可复制的，当它被attach到任何对象上时，都会同步复制AActor::OnRep_AttachmentReplication()
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}
	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->SetHUDAmmo();

	//该方法仅在服务器端发起，因此从服务器端获取当前信息
	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if(Controller)
	{
		Controller->SetHUDWeaponCarriedAmmo(CarriedAmmo);
	}
	//
	
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::Reload()
{
	if(CarriedAmmo > 0 && CombatState != ECombatState::ECS_Reloading)
	{
		ServerReload();
	}
}

void UCombatComponent::ServerReload_Implementation()
{
	if(!Character) return;
	
	CombatState = ECombatState::ECS_Reloading;
	HandleReload();
}

void UCombatComponent::HandleReload() const
{
	Character->PlayReloadMontage();
}

void UCombatComponent::OnReloadingFinished()
{
	if(Character && Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
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
		const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if (HandSocket)
		{
			HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
		}
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	//该复制仅发生在服务器向当前component的持有者，当更新时，同步更新owner的HUD
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if(Controller)
	{
		Controller->SetHUDWeaponCarriedAmmo(CarriedAmmo);
	}
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EBlasterWeaponType::EWT_AssaultRifle, StartingARAmmo);
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
	}
}