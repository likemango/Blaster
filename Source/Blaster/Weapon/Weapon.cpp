// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

#include "NiagaraFunctionLibrary.h"
#include "Blaster/Weapon/Casing/Casing.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Interface/AmmoInterface.h"
#include "Blaster/Interface/HitInterface.h"
#include "Blaster/Interface/PawnInterface.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "DataTypes/SKGAttachmentDataTypes.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "PhysicalMaterial/SKGPhysicalMaterial.h"
#include "SKGAttachment/Public/Components/SKGAttachmentManagerComponent.h"
#include "SKGProceduralAnim/Public/Components/SKGOffHandIKComponent.h"
#include "SKGProceduralAnim/Public/Components/SKGProceduralAnimComponent.h"
#include "SKGShooterFramework/Public/Components/SKGFirearmComponent.h"
#include "Sound/SoundCue.h"
#include "Statics/SKGAttachmentHelpers.h"
#include "Statics/SKGShooterFrameworkCoreEffectStatics.h"
#include "Subsystems/SKGProjectileWorldSubsystem.h"


AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	//开启,则说明只有服务器才有authority,客户端则没有,需要通过replicate来实现同步
	//未开启,则所有客户端都是独立生成的一份,独立于服务器在客户端上生成,各自都有authority
	bReplicates = true;
	Super::SetReplicateMovement(true);

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponMesh->CustomDepthStencilValue = CUSTOM_DEPTH_BLUE;
	WeaponMesh->MarkRenderStateDirty();
	WeaponMesh->SetRenderCustomDepth(true);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	}

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);

	// SKG
	FirearmComponent = CreateDefaultSubobject<USKGFirearmComponent>("FirearmComp");
	ProceduralAnimComponent = CreateDefaultSubobject<USKGProceduralAnimComponent>("ProceduralAnimCompo");
	OffHandIKComponent = CreateDefaultSubobject<USKGOffHandIKComponent>("OffHandIKComp");
	AttachmentManagerComponent = CreateDefaultSubobject<USKGAttachmentManagerComponent>("AttachmentManagerComp");

	FireModes.Emplace(FGameplayTag::RequestGameplayTag(FName("FireModes.Safe")));
	FireModes.Emplace(FGameplayTag::RequestGameplayTag(FName("FireModes.Semi")));
	FireModes.Emplace(FGameplayTag::RequestGameplayTag(FName("FireModes.FullAuto")));
	CurrentFireModeTag = FGameplayTag::RequestGameplayTag(FName("FireModes.Semi"));
	
	FireModeIndex = FireModes.Find(CurrentFireModeTag);
	SetupFireRate(FireRate);

	AttachmentManagerComponent->OnAttachmentComponentAttachmentAdded.AddDynamic(this, &AWeapon::OnAttachmentComponentAttachmentAdded);
	AttachmentManagerComponent->OnAttachmentComponentAttachmentRemoved.AddDynamic(this, &AWeapon::OnAttachmentComponentAttachmentRemoved);
	AttachmentManagerComponent->OnAttachmentsChanged.AddDynamic(this, &AWeapon::OnAttachmentChanged);
	FirearmComponent->OnProceduralAnimComponentsUpdated.AddDynamic(this, &AWeapon::OnProceduralAnimComponentsUpdated);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AreaSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnSphereOverlapEnd);
	
	if(PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}

	ConstructFromPreset();
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	InterpolateActorToTargetTransform(DeltaTime);
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
	DOREPLIFETIME_CONDITION(AWeapon, bUseServerSideRewind, COND_OwnerOnly);
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverLappedComponent, AActor* OtherActor,
                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABlasterCharacter* OverlapBlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if(OverlapBlasterCharacter)
	{
		OverlapBlasterCharacter->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereOverlapEnd(UPrimitiveComponent* OverLappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ABlasterCharacter* OverlapBlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if(OverlapBlasterCharacter)
	{
		OverlapBlasterCharacter->SetOverlappingWeapon(nullptr);
	}
}
// 实现Server reconciliation
void AWeapon::SpendRound()
{
	Ammo = FMath::Clamp(Ammo-1, 0, MagCapacity);
	SetHUDAmmo();
	if(HasAuthority())
	{
		//服务器端的校验程序在这里执行
		ClientUpdateAmmo(Ammo);
	}
	else
	{
		++Sequence;
	}
}

void AWeapon::ClientUpdateAmmo_Implementation(int32 ServerAmmo)
{
	if(HasAuthority())
	{
		return;
	}
	Ammo = ServerAmmo;
	--Sequence;
	Ammo -= Sequence;
	SetHUDAmmo();
}

//它只在服务器执行，所以这里并没有服务器和解，只是手动实现replicate
void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	SetHUDAmmo();
	ClientAddAmmo(Ammo);
}

void AWeapon::ClientAddAmmo_Implementation(int32 AmmoToAdd)
{
	if(HasAuthority())
	{
		return;
	}
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	
	BlasterCharacter = BlasterCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterCharacter;
	bool bJumpToShotgunEnd = BlasterCharacter &&
		BlasterCharacter->GetCombat() &&
			BlasterCharacter->GetCombatState() == ECombatState::ECS_Reloading &&
				WeaponType == EBlasterWeaponType::EWT_Shotgun &&
					IsFull();
	if(bJumpToShotgunEnd)
	{
		BlasterCharacter->GetCombat()->ShotgunReloadJumpToEnd();
	}
	SetHUDAmmo();
}

void AWeapon::SetHUDAmmo()
{
	if(GetOwner())
	{
		BlasterCharacter = BlasterCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterCharacter;
		if(BlasterCharacter && BlasterCharacter->IsLocallyControlled())
		{
			BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(BlasterCharacter->Controller) : BlasterPlayerController;
			if(BlasterPlayerController)
			{
				BlasterPlayerController->SetHUDWeaponAmmo(Ammo);
			}
		}
	}
}

void AWeapon::EnableCustomDepth(bool bEnable) const
{
	if(WeaponMesh)
	{
		WeaponMesh->SetRenderCustomDepth(bEnable);
	}
}

void AWeapon::ConstructFromPreset()
{
	if(HasAuthority())
	{
		if(!PresetString.IsEmpty())
		{
			// GEngine->AddOnScreenDebugMessage(1, 6, FColor::Red, PresetString);
			FSKGAttachmentActor Data;
			if(USKGAttachmentHelpers::DeserializeAttachmentManagerJson(PresetString, Data))
			{
				USKGAttachmentHelpers::ConstructExistingActorFromAttachmentManagerData(this, Data);
			}
		}
	}
}

void AWeapon::InterpolateActorToTargetTransform(float DeltaTime)
{
	float InterpSpeed = 75.f;
	FVector InterpLocation = FMath::VInterpConstantTo(GetActorLocation(), DroppedActorTargetLocation, DeltaTime, InterpSpeed);
	FRotator InterpRotation = FMath::RInterpConstantTo(GetActorRotation(), DroppedActorTargetRotator, DeltaTime, InterpSpeed);
	SetActorLocationAndRotation(InterpLocation, InterpRotation, false, nullptr, ETeleportType::TeleportPhysics);
	if(GetActorLocation().Equals(DroppedActorTargetLocation, 0.1f))
	{
		if(GetActorRotation().Equals(DroppedActorTargetRotator, 1.f))
		{
			StopTickAndPhysics();
		}
		else
		{
			SetActorRotation(InterpRotation, ETeleportType::TeleportPhysics);
		}
	}
}

void AWeapon::SetupFireRate(float InFireRate)
{
	FireRateDelay = 1.f / (InFireRate / 60.f);
}

void AWeapon::OnAttachmentComponentAttachmentAdded(AActor* Attachment)
{
	if(Attachment->Implements<UAmmoInterface>())
	{
		AmmoSource = Attachment;
		AmmoRemaining = IAmmoInterface::Execute_Demo_GetMagazineCapacity(Attachment);
		FirearmProjectile = IAmmoInterface::Execute_Demo_GetProjectileType(Attachment);
	}
}

void AWeapon::OnAttachmentComponentAttachmentRemoved(AActor* Attachment)
{
	if(IAmmoInterface* DemoAmmoSourceInterface = Cast<IAmmoInterface>(Attachment))
	{
		AmmoSource = nullptr;
		FirearmProjectile = nullptr;
	}
}

void AWeapon::OnProceduralAnimComponentsUpdated()
{
	if(!FirearmComponent)
		return;
	
	TArray<USKGProceduralAnimComponent*> ProceduralAnimComponents = FirearmComponent->GetProceduralAnimComponents();
	for (USKGProceduralAnimComponent* Component : ProceduralAnimComponents)
	{
		if(!Component)
			continue;

		// todo
		// if(ASKGDemoMagnifier* Magnifier = Cast<ASKGDemoMagnifier>(Component->GetOwner()))
		// {
		// 	Magnifier->LinkOptic();;
		// }
	}
}

void AWeapon::OnAttachmentChanged()
{
	// todo
}

void AWeapon::OnMagnifierFlippedCallback(UAnimMontage* Montage, FName SectionName)
{
	// todo
}

void AWeapon::Fire()
{
	if(CanFire())
	{
		FSKGMuzzleTransform MuzzleTransform = GetMuzzleTransform();
		if(HasAuthority())
		{
			ReplicateFireData(MuzzleTransform, true);
		}
		else
		{
			Server_Fire(MuzzleTransform);
		}
		FireLocal();
	}
}

void AWeapon::StopFire()
{
	if(GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(TFullAutoHandle);
	}
	if(HasAuthority())
	{
		ReplicateFireData(FSKGMuzzleTransform(), false);
	}
	else
	{
		Server_StopFire();
	}
}

bool AWeapon::CanFire()
{
	if(FirearmProjectile && CurrentFireModeTag != FGameplayTag::RequestGameplayTag(FName("FireModes.Safe"))
		&& AmmoRemaining > 0 && !bIsReloading)
	{
		if(CurrentFireModeTag != FGameplayTag::RequestGameplayTag(FName("FireModes.Semi"))
			|| CurrentFireModeTag != FGameplayTag::RequestGameplayTag(FName("FireModes.FullAuto")))
		{
			if(UGameplayStatics::GetTimeSeconds(this) - LastShotTime >= FireRateDelay)
			{
				return true;
			}
			return false;
		}
	}
	return false;
}

void AWeapon::Reload_Implementation()
{
	if(FirearmProjectile && !bIsReloading)
	{
		if(HasAuthority())
		{
			bIsReloading = true;
			OnRep_bIsReloading();
		}
		else
		{
			Server_Reload();
			PlayReloadMontage();
			bIsReloading = true;
		}
	}
}

void AWeapon::ReloadComplete_Implementation()
{
	bIsReloading = false;
	if(HasAuthority() && AmmoSource)
	{
		if(AmmoSource->Implements<UAmmoInterface>())
		{
			AmmoRemaining = IAmmoInterface::Execute_Demo_GetMagazineCapacity(AmmoSource);
			FirearmProjectile = IAmmoInterface::Execute_Demo_GetProjectileType(AmmoSource);
		}
	}
}

void AWeapon::CycleFireMode()
{
	if(HasAuthority())
	{
		if(FireModeIndex + 1 >= FireModes.Num())
		{
			FireModeIndex = 0;
		}
		else
		{
			FireModeIndex = FireModeIndex + 1;
		}
		CurrentFireModeTag = FireModes[FireModeIndex];
	}
	else
	{
		Server_CycleFireMode();
	}
}

EBlasterWeaponPriorityType AWeapon::GetWeaponPriorityType()
{
	return PriorityType;
}

void AWeapon::ActionFinishedCycling()
{
	// todo
}

FName AWeapon::GetAttachSocket_Implementation()
{
	return AttachSocket;
}

void AWeapon::Interact_Implementation(APawn* Pawn)
{
	if(!GetOwner())
	{
		Cast<IPawnInterface>(Pawn)->PickUpActor(this);
	}
}

bool AWeapon::IsPickup_Implementation()
{
	// todo
	return false;
}

void AWeapon::OnPickup_Implementation(USceneComponent* Component, FName Socket)
{
	if(GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(TActorDroppedPhysicsHandle);
	}
	
	OnPickedup(Component, Socket);
}

void AWeapon::OnDrop_Implementation()
{
	SkeletalMeshComponent->SetSimulatePhysics(true);
	for (AActor* Actor: AttachmentManagerComponent->GetAttachments())
	{
		Actor->SetActorEnableCollision(false);
	}
	GetWorld()->GetTimerManager().SetTimer(TActorDroppedPhysicsHandle, this, &AWeapon::ReplicateDropPhysics, 0.1f, true);
}

bool AWeapon::CanBePickedUp_Implementation()
{
	return bCanGetPickedup;
}

FSKGMuzzleTransform AWeapon::GetMuzzleTransform() const
{
	return FirearmComponent->GetMuzzleProjectileTransform();
}

void AWeapon::ReplicateFireData(const FSKGMuzzleTransform& MuzzleTransform, bool bIsFiring)
{
	if(HasAuthority())
	{
		ProjectileReplicationData.bIsFiring = bIsFiring;
		ProjectileReplicationData.MuzzleTransform = MuzzleTransform;
		OnRep_ProjectileReplicationData();
	}
}

void AWeapon::OnRep_ProjectileReplicationData()
{
	if(!IsLocallyControlled())
	{
		if(ProjectileReplicationData.bIsFiring)
		{
			FireRemote();
		}
		else
		{
			GetWorld()->GetTimerManager().ClearTimer(TFullAutoHandle);
		}
	}
}

bool AWeapon::IsLocallyControlled()
{
	if(APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		return Pawn->IsLocallyControlled();
	}
	return false;
}

void AWeapon::FireRemote()
{
	TFunction<void()> Func = [this](){FireRemote();};
	FireShot(ProjectileReplicationData.MuzzleTransform, Func);
}

void AWeapon::FireShot(FTransform MuzzleTransform, const TFunction<void()>& Func)
{
	if(MuzzleTransform.IsValid())
	{
		LaunchProjectile(FirearmProjectile, MuzzleTransform);
		PlayFireAnimation();
		PlayShotEffects();
		LastShotTime = UGameplayStatics::GetTimeSeconds(this);
		if(CurrentFireModeTag == FGameplayTag::RequestGameplayTag(FName("FireModes.FullAuto")))
		{
			GetWorld()->GetTimerManager().SetTimer(TFullAutoHandle, std::cref(Func) ,FireRateDelay, true);
		}
	}
}

void AWeapon::LaunchProjectile(USKGPDAProjectile* Projectile, FTransform MuzzleTransform)
{
	USKGProjectileWorldSubsystem* ProjectileWorldSubsystem = GetWorld()->GetSubsystem<USKGProjectileWorldSubsystem>();
	FSKGOnProjectileImpact OnProjectileImpact;
	FSKGOnProjetilePositionUpdate OnProjetilePositionUpdate;
	OnProjectileImpact.BindDynamic(this, &AWeapon::OnImpact);
	const TArray<AActor*> Attachments = AttachmentManagerComponent->GetAttachments();
	ProjectileWorldSubsystem->FireProjectile(0, Projectile, Attachments, MuzzleTransform, nullptr, OnProjectileImpact, OnProjetilePositionUpdate);
}

void AWeapon::OnImpact(const FHitResult& HitResult, const FVector& Direction, const int32 ProjectileID)
{
	PerformImpactEffect(HitResult);
	if(IHitInterface * HitInterface = Cast<IHitInterface>(HitResult.GetActor()))
	{
		float DamageAmount = CalculateDamageFromProjectile(ProjectileID);
		HitInterface->Hit(DamageAmount, HitResult);
	}
}

float AWeapon::CalculateDamageFromProjectile(int32 ProjectileID)
{
	USKGProjectileWorldSubsystem* ProjectileWorldSubsystem = GetWorld()->GetSubsystem<USKGProjectileWorldSubsystem>();
	FSKGProjectileData ProjectileData;
	if(ProjectileWorldSubsystem->GetProjectileByID(ProjectileID, ProjectileData))
	{
		// 伤害与速度相关
		// a lot calculations.
	}
	return 12.f;
}

void AWeapon::PerformImpactEffect(const FHitResult& HitResult)
{
	if(USKGPhysicalMaterial* SKGDemoPhysicalMaterial = Cast<USKGPhysicalMaterial>(HitResult.PhysMaterial))
	{
		SKGDemoPhysicalMaterial->PlayEffect(HitResult, true, FGameplayTag::RequestGameplayTag(FName("Impact.Bullet")));
	}
}

void AWeapon::PlayFireAnimation()
{
	FAnimationMontageData AnimationMontageData;
	AnimationMontageData.Montage = CharacterShootMontage;
	AnimationMontageData.Section = TEXT("None");
	OnFired.Broadcast(ControlRotationMultiplier, RecoilLocationMultiplier, RecoilRotationMultiplier, AnimationMontageData);
	SkeletalMeshComponent->GetAnimInstance()->Montage_Play(FirearmShootMontage);
}

void AWeapon::PlayShotEffects()
{
	if(USoundCue* SoundCue = GetShotSound())
	{
		USKGShooterFrameworkCoreEffectStatics::PlaySoundEffect(this, GetMuzzleTransform().Location, true, SoundCue, 0.2f, 1.0f, 1.0f);
	}
	if(UNiagaraSystem* NiagaraSystem = GetShotMuzzleParticle())
	{
		FTransform MuzzleTransform = FirearmComponent->GetMuzzleTransform();
		UNiagaraFunctionLibrary::SpawnSystemAttached(NiagaraSystem, SkeletalMeshComponent, TEXT("None"), MuzzleTransform.GetLocation(), MuzzleTransform.GetRotation().Rotator(), EAttachLocation::KeepWorldPosition, false, true, ENCPoolMethod::AutoRelease, true);
	}
	FirearmComponent->ShotPerformed();
}

USoundCue* AWeapon::GetShotSound() const
{
	if(IsSuppressed())
	{
		return SuppressedShotSound;
	}
	return ShotSound;
}

bool AWeapon::IsSuppressed() const
{
	if(USKGMuzzleComponent* MuzzleComponent = FirearmComponent->GetCurrentMuzzleComponent())
	{
		FGameplayTag MuzzleTag = MuzzleComponent->GetMuzzleTag();
		if(MuzzleTag == FGameplayTag::RequestGameplayTag(FName("MuzzleComponentType.Barrel")) || 
			MuzzleTag == FGameplayTag::RequestGameplayTag(FName("MuzzleComponentType.MuzzleDevice")))
		{
			return false;
		}
		if(MuzzleTag == FGameplayTag::RequestGameplayTag(FName("MuzzleComponentType.Suppressor")))
		{
			return true;
		}
	}
	return false;
}

UNiagaraSystem* AWeapon::GetShotMuzzleParticle() const
{
	if(IsSuppressed())
	{
		return SuppressedShotMuzzleParticle;
	}
	return ShotMuzzleParticle;
}

void AWeapon::PlayReloadMontage()
{
	OnReload.Broadcast(CharacterReloadMontage);
	SkeletalMeshComponent->GetAnimInstance()->Montage_Play(FirearmReloadMontage);
}

void AWeapon::OnRep_bIsReloading()
{
	if(bIsReloading)
	{
		PlayReloadMontage();
	}
}

void AWeapon::OnWeaponStateSet()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		OnEquipped();
		break;
	case EWeaponState::EWS_EquippedSecondary:
		OnEquippedSecondary();
		break;
	case EWeaponState::EWS_Dropped:
		OnDropped();
		break;
	}
}

void AWeapon::OnEquipped()
{
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if(WeaponType == EBlasterWeaponType::EWT_SubmachineGun)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	}
	EnableCustomDepth(false);

	BlasterCharacter = BlasterCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterCharacter;
	if (BlasterCharacter && bUseServerSideRewind)
	{
		BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(BlasterCharacter->Controller) : BlasterPlayerController;
		if (BlasterPlayerController && HasAuthority() && !BlasterPlayerController->HighPingDelegate.IsBound())
		{
			BlasterPlayerController->HighPingDelegate.AddDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}

void AWeapon::OnDropped()
{
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetEnableGravity(true);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		
	WeaponMesh->CustomDepthStencilValue = CUSTOM_DEPTH_BLUE;
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);

	BlasterCharacter = BlasterCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterCharacter;
	if (BlasterCharacter)
	{
		BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(BlasterCharacter->Controller) : BlasterPlayerController;
		if (BlasterPlayerController && HasAuthority() && !BlasterPlayerController->HighPingDelegate.IsBound())
		{
			BlasterPlayerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}

void AWeapon::OnEquippedSecondary()
{
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (WeaponType == EBlasterWeaponType::EWT_SubmachineGun)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}
	EnableCustomDepth(true);
	if (WeaponMesh)
	{
		WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_TAN);
		WeaponMesh->MarkRenderStateDirty();
	}

	BlasterCharacter = BlasterCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterCharacter;
	if (BlasterCharacter)
	{
		BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(BlasterCharacter->Controller) : BlasterPlayerController;
		if (BlasterPlayerController && HasAuthority() && !BlasterPlayerController->HighPingDelegate.IsBound())
		{
			BlasterPlayerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}

void AWeapon::OnPingTooHigh(bool bHighPing)
{
	bUseServerSideRewind = !bHighPing;
}

void AWeapon::SetWeaponState(EWeaponState NewState)
{
	WeaponState = NewState;
	OnWeaponStateSet();
}

void AWeapon::OnRep_WeaponState()
{
	OnWeaponStateSet();
}


void AWeapon::ShowPickupWidget(bool bShow)
{
	if(PickupWidget)
	{
		PickupWidget->SetVisibility(bShow);
	}
}

void AWeapon::Fire(const FVector& HitTarget)
{
	if(WeaponMesh)
	{
		WeaponMesh->PlayAnimation(FireAnimation,false);
	}
	if(CasingClass)
	{
		const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject"));
		if(AmmoEjectSocket)
		{
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);
			UWorld* World = GetWorld();
			if(World)
			{
				World->SpawnActor<ACasing>(
					CasingClass,
					SocketTransform.GetLocation(),
					SocketTransform.GetRotation().Rotator()
				);
			}
		}
	}
	SpendRound();
}

void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
	// Detach this component from whatever it is attached to.
	// WeaponMesh是根组件，因此移除根组件等同于移除Actor，attachActor的本质就是attach根组件
	// 在这个情况下，等同于DetachFromActor(DetachmentTransformRules);
	WeaponMesh->DetachFromComponent(DetachmentTransformRules);
	SetOwner(nullptr);
	BlasterCharacter = nullptr;
	BlasterPlayerController = nullptr;
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	
	if(Owner == nullptr)
	{
		BlasterCharacter = nullptr;
		BlasterPlayerController = nullptr;
	}
	else
	{
		BlasterCharacter = BlasterCharacter == nullptr ? Cast<ABlasterCharacter>(Owner) : BlasterCharacter;
		if (BlasterCharacter && BlasterCharacter->GetEquippedWeapon() && BlasterCharacter->GetEquippedWeapon() == this)
		{
			SetHUDAmmo();
		}
	}
}

bool AWeapon::IsEmpty() const
{
	return Ammo <= 0;
}

bool AWeapon::IsFull() const
{
	return Ammo == MagCapacity;
}

void AWeapon::Server_Fire_Implementation(const FSKGMuzzleTransform& MuzzleTransform)
{
	AmmoRemaining -= 1;
	if(AmmoRemaining <= 0)
	{
		StopFire();
	}
	else
	{
		ReplicateFireData(MuzzleTransform, true);
	}
}

void AWeapon::FireLocal()
{
	TFunction<void()> Func = [this](){Fire();};
	FireShot(GetMuzzleTransform(), Func);
}

void AWeapon::OnPickedup(USceneComponent* Component, FName Socket)
{
	bCanGetPickedup = false;
	StopTickAndPhysics();

	if(PickedUpData.Count > 254)
	{
		PickedUpData.SceneComponent = Component;
		PickedUpData.Socket = Socket;
		OnRep_PickedUpData();
	}
	else
	{
		FPickedUpRepData NewPickedUpData;
		NewPickedUpData.Socket = Socket;
		NewPickedUpData.SceneComponent = Component;
		NewPickedUpData.Count = PickedUpData.Count + 1;
		PickedUpData = NewPickedUpData;
		OnRep_PickedUpData();
	}
}

void AWeapon::StopTickAndPhysics()
{
	SkeletalMeshComponent->SetSimulatePhysics(false);
	if(IsActorTickEnabled())
	{
		SetActorTickEnabled(false);
	}
}

void AWeapon::StartTickAndPhysics()
{
	SkeletalMeshComponent->SetSimulatePhysics(true);
	if(!IsActorTickEnabled())
	{
		SetActorTickEnabled(true);
	}
}

void AWeapon::ReplicateDropPhysics()
{
	FVector_NetQuantize Location_NetQuantize(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z);
	FVector_NetQuantize Rotation_NetQuantize(GetActorRotation().Roll, GetActorRotation().Pitch, GetActorRotation().Yaw);
	DropPhysicsData.Location = Location_NetQuantize;
	DropPhysicsData.Rotation = Rotation_NetQuantize;
	OnRep_DropPhysicsData();
	
	if(GetVelocity().Equals(FVector::ZeroVector, 0.1f))
	{
		GetWorld()->GetTimerManager().ClearTimer(TActorDroppedPhysicsHandle);
		bCanGetPickedup = true;
	}
}

void AWeapon::SetActorAtTargetTransform()
{
	StopTickAndPhysics();
	SetActorLocationAndRotation(DroppedActorTargetLocation, DroppedActorTargetRotator, false);
}

void AWeapon::Server_CycleFireMode_Implementation()
{
	CycleFireMode();
}

void AWeapon::Server_Reload_Implementation()
{
	if(FirearmProjectile)
	{
		bIsReloading = true;
		OnRep_bIsReloading();
	}
}

void AWeapon::Server_StopFire_Implementation()
{
	ReplicateFireData(FSKGMuzzleTransform(), false);
}

void AWeapon::OnRep_PickedUpData()
{
	StopTickAndPhysics();
	GetWorld()->GetTimerManager().ClearTimer(TActorDroppedReplicationTimeLimitHandle);
	if(PickedUpData.SceneComponent)
	{
		FAttachmentTransformRules AttachmentTransformRules = FAttachmentTransformRules(EAttachmentRule::SnapToTarget,
			EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true);
		if(AttachToComponent(PickedUpData.SceneComponent, AttachmentTransformRules, PickedUpData.Socket))
		{
			GEngine->AddOnScreenDebugMessage(1, 3.f, FColor::Green, TEXT("Pickup Firearm successfully."));
		}
	}
}

void AWeapon::OnRep_DropPhysicsData()
{
	if(!HasAuthority())
	{
		GetWorld()->GetTimerManager().SetTimer(TActorDroppedReplicationTimeLimitHandle, this, &AWeapon::SetActorAtTargetTransform, 5.f, false);

		DroppedActorTargetLocation = DropPhysicsData.Location;
		DroppedActorTargetRotator = FRotator(DropPhysicsData.Rotation.Y, DropPhysicsData.Rotation.Z,DropPhysicsData.Rotation.X);
		if(!IsActorTickEnabled())
		{
			StartTickAndPhysics();
			for (AActor* Actor : AttachmentManagerComponent->GetAttachments())
			{
				Actor->SetActorEnableCollision(false);
			}
		}
	}
}

FVector AWeapon::TraceEndWithScatter(const FVector& HitTarget)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return FVector();

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();

	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	const FVector EndLoc = SphereCenter + RandVec;
	const FVector ToEndLoc = EndLoc - TraceStart;

	/*
	DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);
	DrawDebugSphere(GetWorld(), EndLoc, 4.f, 12, FColor::Orange, true);
	DrawDebugLine(
		GetWorld(),
		TraceStart,
		FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size()),
		FColor::Cyan,
		true);*/

	return FVector(TraceStart + ToEndLoc * TRACE_LINE_LENGTH / ToEndLoc.Size());
}
