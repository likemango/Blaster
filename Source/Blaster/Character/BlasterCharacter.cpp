// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"

#include "BlasterAnimInstance.h"
#include "IAnimationBudgetAllocator.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "SkeletalMeshComponentBudgeted.h"
#include "Blaster/Blaster.h"
#include "Blaster/Animations/SKGAnimInstance.h"
#include "Blaster/BlasterComponents/BuffComponent.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/BlasterComponents/LagCompensationComponent.h"
#include "Blaster/GameModes/BlasterGameMode.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Blaster/Weapon/Weapon.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Components/BoxComponent.h"
#include "SKGShooterFramework/Public/Components/SKGShooterPawnComponent.h"
#include "Statics/SKGAnimationBudgetAllocatorFunctionsLibrary.h"
#include "AnimationBudgetAllocator/Private/AnimationBudgetAllocatorModule.h"
#include "Components/SKGAttachmentManagerComponent.h"

// Sets default values
ABlasterCharacter::ABlasterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	// CameraBoom->TargetArmLength = 600.f;
	// CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(GetMesh(), FName("S_Camera"));
	// FollowCamera->bUsePawnControlRotation = false;

	// 设置Controller如何影响Character的朝向
	// bUseControllerRotationYaw = false; // 如果是true，那么角色会持续朝向control的Yaw朝向
	// GetCharacterMovement()->bOrientRotationToMovement = true; // 使用rotateRate去转向朝向

	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverHeadWidget"));
	WidgetComponent->SetupAttachment(RootComponent);
	
	// Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	// Combat->SetIsReplicated(true);

	// Buff = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComp"));
	// Buff->SetIsReplicated(true);

	// LagCompensation = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensation"));

	// GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	// TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	// how fast character turn rotation. Should 'bOrientRotationToMovement' to true.
	// GetCharacterMovement()->RotationRate = FRotator(0,850.f,0);
	// always spawn character
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	// DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));

	GrenadeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrenadeMesh"));
	GrenadeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GrenadeMesh->SetupAttachment(GetMesh(), FName(TEXT("GrenadeSocket")));

	/** 
	* Hit boxes for server-side rewind, hidden for now.
	*/
	/*head = CreateDefaultSubobject<UBoxComponent>(TEXT("head"));
	head->SetupAttachment(GetMesh(), FName("head"));
	HitCollisionBoxes.Add(FName("head"), head);

	pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("pelvis"));
	pelvis->SetupAttachment(GetMesh(), FName("pelvis"));
	HitCollisionBoxes.Add(FName("pelvis"), pelvis);

	spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_02"));
	spine_02->SetupAttachment(GetMesh(), FName("spine_02"));
	HitCollisionBoxes.Add(FName("spine_02"), spine_02);

	spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_03"));
	spine_03->SetupAttachment(GetMesh(), FName("spine_03"));
	HitCollisionBoxes.Add(FName("spine_03"), spine_03);

	upperarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_l"));
	upperarm_l->SetupAttachment(GetMesh(), FName("upperarm_l"));
	HitCollisionBoxes.Add(FName("upperarm_l"), upperarm_l);

	upperarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_r"));
	upperarm_r->SetupAttachment(GetMesh(), FName("upperarm_r"));
	HitCollisionBoxes.Add(FName("upperarm_r"), upperarm_r);

	lowerarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_l"));
	lowerarm_l->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	HitCollisionBoxes.Add(FName("lowerarm_l"), lowerarm_l);

	lowerarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_r"));
	lowerarm_r->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	HitCollisionBoxes.Add(FName("lowerarm_r"), lowerarm_r);

	hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_l"));
	hand_l->SetupAttachment(GetMesh(), FName("hand_l"));
	HitCollisionBoxes.Add(FName("hand_l"), hand_l);

	hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_r"));
	hand_r->SetupAttachment(GetMesh(), FName("hand_r"));
	HitCollisionBoxes.Add(FName("hand_r"), hand_r);

	blanket = CreateDefaultSubobject<UBoxComponent>(TEXT("blanket"));
	blanket->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(FName("blanket"), blanket);

	backpack = CreateDefaultSubobject<UBoxComponent>(TEXT("backpack"));
	backpack->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(FName("backpack"), backpack);

	thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_l"));
	thigh_l->SetupAttachment(GetMesh(), FName("thigh_l"));
	HitCollisionBoxes.Add(FName("thigh_l"), thigh_l);

	thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_r"));
	thigh_r->SetupAttachment(GetMesh(), FName("thigh_r"));
	HitCollisionBoxes.Add(FName("thigh_r"), thigh_r);

	calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_l"));
	calf_l->SetupAttachment(GetMesh(), FName("calf_l"));
	HitCollisionBoxes.Add(FName("calf_l"), calf_l);

	calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_r"));
	calf_r->SetupAttachment(GetMesh(), FName("calf_r"));
	HitCollisionBoxes.Add(FName("calf_r"), calf_r);

	foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_l"));
	foot_l->SetupAttachment(GetMesh(), FName("foot_l"));
	HitCollisionBoxes.Add(FName("foot_l"), foot_l);

	foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_r"));
	foot_r->SetupAttachment(GetMesh(), FName("foot_r"));
	HitCollisionBoxes.Add(FName("foot_r"), foot_r);

	for(auto HitBoxPair : HitCollisionBoxes)
	{
		HitBoxPair.Value->SetCollisionObjectType(ECC_HitBox);
		HitBoxPair.Value->SetCollisionResponseToAllChannels(ECR_Ignore);
		HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block);
		HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}*/
	
	AttachmentManagerComponent = CreateDefaultSubobject<USKGAttachmentManagerComponent>("SKGAttachmentManagerComp");
	ShooterPawnComponent = CreateDefaultSubobject<USKGShooterPawnComponent>("ShooterPawnComponent");
	ShooterPawnComponent->OnHeldActorSet.AddDynamic(this, &ABlasterCharacter::OnHeldActorSet);
	ShooterPawnComponent->OnPoseComplete.AddDynamic(this, &ABlasterCharacter::OnPoseComplete);
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ABlasterCharacter, Health);
	DOREPLIFETIME(ABlasterCharacter, Shield);

	DOREPLIFETIME_CONDITION(ABlasterCharacter, FirearmOnBack, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ABlasterCharacter, FirearmToSwitchTo, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ABlasterCharacter, PistolInHolster, COND_OwnerOnly);
}

void ABlasterCharacter::SetIsInCoolDownState(bool NewState)
{
	bInCoolDownTime = NewState;
}

void ABlasterCharacter::ServerPlayerLeftGame_Implementation()
{
	// GameMode只能在服务器端获取到
	if(ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>())
	{
		BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
		if(BlasterPlayerController)
		{
			BlasterPlayerState = BlasterPlayerState == nullptr ? Cast<ABlasterPlayerState>(BlasterPlayerController->PlayerState) : BlasterPlayerState;
			BlasterGameMode->PlayerLeftGame(BlasterPlayerState);
		}
	}
}

void ABlasterCharacter::SetTeamColor(ETeamTypes Team)
{
	if (GetMesh() == nullptr || OriginalMaterial == nullptr) return;
	switch (Team)
	{
	case ETeamTypes::ET_NoTeam:
		GetMesh()->SetMaterial(0, OriginalMaterial);
		DissolveMaterialInst = BlueDissolveMatInst;
		break;
	case ETeamTypes::ET_BlueTeam:
		GetMesh()->SetMaterial(0, BlueMaterial);
		DissolveMaterialInst = BlueDissolveMatInst;
		break;
	case ETeamTypes::ET_RedTeam:
		GetMesh()->SetMaterial(0, RedMaterial);
		DissolveMaterialInst = RedDissolveMatInst;
		break;
	}
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
	// SpawnDefaultWeapon();

	SetOnlyTickPoseWhenRenderedDedicated();
	SetupAnimationBudgetAllocator();
	// SpawnInitialFirearm();
	BindToAnimBPEvent();
	BindToPlayerControllerEvent();
	
	UpdateHUDAmmo();
	UpdateHealthHUD();
	UpdateHUDShield();

	if(HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ThisClass::ReceiveDamage);
	}
	if(GrenadeMesh)
	{
		GrenadeMesh->SetVisibility(false);
	}
}
void ABlasterCharacter::UpdateHealthHUD()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if(BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void ABlasterCharacter::UpdateHUDShield()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDShield(Shield, MaxShield);
	}
}

void ABlasterCharacter::UpdateHUDAmmo()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterPlayerController && Combat && Combat->EquippedWeapon)
	{
		BlasterPlayerController->SetHUDCarriedAmmo(Combat->CarriedAmmo);
		BlasterPlayerController->SetHUDWeaponAmmo(Combat->EquippedWeapon->GetAmmo());
	}
}

void ABlasterCharacter::SpawnDefaultWeapon()
{
	ABlasterGameMode* BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	UWorld* World = GetWorld();
	if (BlasterGameMode && World && !bEliminated && DefaultWeaponClass)
	{
		AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
		StartingWeapon->bDestroyWeapon = true;
		if (Combat)
		{
			Combat->EquipWeapon(StartingWeapon);
		}
	}
}

bool ABlasterCharacter::IsDead()
{
	return false;
}

void ABlasterCharacter::PickUpActor(AActor* Actor)
{
	if(IsHoldingActor())
	{
		FName SocketName;
		if(AttachNonEquippedActor(Actor, SocketName))
		{
			Cast<IInteractInterface>(Actor)->OnPickup(GetMesh(), SocketName);
		}
	}
	else
	{
		AttachEquipActor(Actor);
		Cast<IInteractInterface>(Actor)->OnPickup(GetMesh(), FirearmAttachSockName);
	}
}

void ABlasterCharacter::OnRep_FirearmToSwitchTo()
{
	if(!IsLocallyControlled())
	{
		GetSKGAnimInstance()->StartUnequip();
	}
}

bool ABlasterCharacter::IsHoldingActor() const
{
	return ShooterPawnComponent->GetHeldActor() != nullptr;
}

void ABlasterCharacter::AttachEquipActor(AActor* InFirearmToEquip)
{
	ShooterPawnComponent->SetHeldActor(InFirearmToEquip);
	FAttachmentTransformRules AttachmentTransformRules = CreateFirearmAttachmentRules();
	InFirearmToEquip->AttachToComponent(GetMesh(), AttachmentTransformRules, FirearmAttachSockName);
	InFirearmToEquip->SetOwner(this);
}

bool ABlasterCharacter::AttachNonEquippedActor(AActor* Actor, FName& OutSocketName)
{
	if(IWeaponInterface* FirearmInterface = Cast<IWeaponInterface>(Actor))
	{
		EBlasterWeaponPriorityType PriorityType = FirearmInterface->GetWeaponPriorityType();
		if(PriorityType == EBlasterWeaponPriorityType::Primary)
		{
			if(FirearmOnBack)
				return false;
			FirearmOnBack = Actor;
			OutSocketName = Cast<IInteractInterface>(Actor)->GetAttachSocket();
			FAttachmentTransformRules AttachmentTransformRules = CreateFirearmAttachmentRules();
			Actor->AttachToComponent(GetMesh(), AttachmentTransformRules, OutSocketName);
			return true;
		}
		if(PriorityType == EBlasterWeaponPriorityType::Secondary)
		{
			if(PistolInHolster)
				return false;
			PistolInHolster = Actor;
			OutSocketName = Cast<IInteractInterface>(Actor)->GetAttachSocket();
			FAttachmentTransformRules AttachmentTransformRules = CreateFirearmAttachmentRules();
			Actor->AttachToComponent(GetMesh(), AttachmentTransformRules, OutSocketName);
			return true;
		}
	}
	else
	{
		OutSocketName = Cast<IInteractInterface>(Actor)->GetAttachSocket();
		FAttachmentTransformRules AttachmentTransformRules = CreateFirearmAttachmentRules();
		Actor->AttachToComponent(GetMesh(), AttachmentTransformRules, OutSocketName);
		return true;
	}
	return false;
}

FAttachmentTransformRules ABlasterCharacter::CreateFirearmAttachmentRules() const
{
	FAttachmentTransformRules AttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget,
				EAttachmentRule::KeepWorld, true);
	return AttachmentTransformRules;
}

bool ABlasterCharacter::ShouldSwapWeapons() const
{
	if(!bIsPerformingAction && FirearmOnBack != nullptr)
		return true;
	return false;
}

void ABlasterCharacter::SwapWeapons()
{
	bIsPerformingAction = true;
	StopAiming();
	GetSKGAnimInstance()->StartUnequip();
	FirearmToSwitchTo = FirearmOnBack;
	OnRep_FirearmToSwitchTo();
	if(!HasAuthority())
	{
		Server_SwapWeapons();
	}
}

void ABlasterCharacter::StartAiming()
{
	bWantsToAim = true;
	if(!bIsPerformingAction)
	{
		ShooterPawnComponent->StartAiming();	
	}
}

void ABlasterCharacter::StopAiming()
{
	bWantsToAim = false;
	ShooterPawnComponent->StopAiming();
}

USKGAnimInstance* ABlasterCharacter::GetSKGAnimInstance()
{
	if(!SKGAnimInstance)
	{
		SKGAnimInstance = Cast<USKGAnimInstance>(GetMesh()->GetAnimInstance());
	}
	return SKGAnimInstance;
}

void ABlasterCharacter::Server_SwapWeapons_Implementation()
{
	FirearmToSwitchTo = FirearmOnBack;
}

void ABlasterCharacter::OnHeldActorSet(AActor* NewHeldActor, AActor* OldHeldActor)
{
	if(NewHeldActor)
	{
		ShooterPawnComponent->UnlinkAnimLayerClass();
		ShooterPawnComponent->LinkAnimLayerClass(SKGAnimLayerArmed);
	}
	else
	{
		ShooterPawnComponent->UnlinkAnimLayerClass();
		ShooterPawnComponent->LinkAnimLayerClass(SKGAnimLayerUnarmed);
	}
	BindToFirearmEvents(NewHeldActor);
	UnbindFromFirearmEvents(OldHeldActor);	
}

void ABlasterCharacter::OnPoseComplete(const FSKGProceduralPoseReplicationData& CurrentPoseData)
{
	bIsPerformingAction = false;
}

void ABlasterCharacter::BindToFirearmEvents(AActor* Actor)
{
	if(AWeapon* Firearm = Cast<AWeapon>(Actor))
	{
		Firearm->OnFired.AddDynamic(this, &ABlasterCharacter::OnFired);
		Firearm->OnReload.AddDynamic(this, &ABlasterCharacter::OnReloading);
		Firearm->OnActionCycled.AddDynamic(this, &ABlasterCharacter::OnActionCycled);
		Firearm->OnMagnifierFlipped.AddDynamic(this, &ABlasterCharacter::OnMagnifierFlipped);
	}
}

void ABlasterCharacter::UnbindFromFirearmEvents(AActor* Actor)
{
	if(AWeapon* Firearm = Cast<AWeapon>(Actor))
	{
		Firearm->OnFired.RemoveDynamic(this, &ABlasterCharacter::OnFired);
		Firearm->OnReload.RemoveDynamic(this, &ABlasterCharacter::OnReloading);
		Firearm->OnActionCycled.RemoveDynamic(this, &ABlasterCharacter::OnActionCycled);
		Firearm->OnMagnifierFlipped.RemoveDynamic(this, &ABlasterCharacter::OnMagnifierFlipped);
	}
}

void ABlasterCharacter::OnFired(const FRotator& RecoilControlRotationMultiplier,
	const FVector& RecoilLocationMultiplier, const FRotator& RecoilRotationMultiplier,
	const FAnimationMontageData& AnimationData)
{
	ShooterPawnComponent->PerformProceduralRecoil(RecoilControlRotationMultiplier, RecoilLocationMultiplier,RecoilRotationMultiplier);
	PlayAnimMontage(AnimationData.Montage, 1, AnimationData.Section);
}

void ABlasterCharacter::OnReloading(UAnimMontage* Montage)
{
	PlayAnimMontage(Montage);
}

void ABlasterCharacter::OnActionCycled(UAnimMontage* Montage)
{
	PlayAnimMontage(Montage);
}

void ABlasterCharacter::OnMagnifierFlipped(UAnimMontage* Montage, FName SectionName)
{
	PlayAnimMontage(Montage, 1, SectionName);
}

void ABlasterCharacter::SetOnlyTickPoseWhenRenderedDedicated()
{
	/*
	 * Dont tick pose on dedicated server.
	 * We will capture the pose though for a single frame on dedicated server to evaluate hits, dropping weapon, that sort of thing
	 */
}

void ABlasterCharacter::SetupAnimationBudgetAllocator()
{
	/*
	 * Sets up the animation budget allocator for all meshes except our own
	 */
	if(!IsLocallyControlled())
	{
		if(USkeletalMeshComponentBudgeted* SkeletalMeshComponentBudgeted = Cast<USkeletalMeshComponentBudgeted>(GetMesh()))
		{
			USKGAnimationBudgetAllocatorFunctionsLibrary::RegisterSkeletalMeshComponentBudgeted(SkeletalMeshComponentBudgeted);

			// UAnimationBudgetBlueprintLibrary::EnableAnimationBudget(this, true);
			if(UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull))
			{
				FAnimationBudgetAllocatorModule& AnimationBudgetAllocatorModule = FModuleManager::LoadModuleChecked<FAnimationBudgetAllocatorModule>("AnimationBudgetAllocator");
				IAnimationBudgetAllocator* AnimationBudgetAllocator = AnimationBudgetAllocatorModule.GetBudgetAllocatorForWorld(World);
				if(AnimationBudgetAllocator)
				{
					AnimationBudgetAllocator->SetEnabled(true);
				}
			}
			
		}
	}
}

void ABlasterCharacter::SpawnInitialFirearm()
{
	UWorld* World = GetWorld();
	check(World);
	
	if(HasAuthority())
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = this;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParameters.TransformScaleMethod = ESpawnActorScaleMethod::MultiplyWithRoot;
		FTransform* Transform = new FTransform();
		Transform->SetLocation(FVector::ZeroVector);
		Transform->SetRotation(FQuat::Identity);
		Transform->SetTranslation(FVector::OneVector);
		AActor* newFirearm = World->SpawnActor(InitialFirearm, Transform, SpawnParameters);

		ShooterPawnComponent->SetHeldActor(newFirearm);

		FAttachmentTransformRules AttachmentTransformRules = CreateFirearmAttachmentRules();
		newFirearm->AttachToComponent(GetMesh(), AttachmentTransformRules, FirearmAttachSockName);
	}
}

void ABlasterCharacter::BindToAnimBPEvent()
{
	GetSKGAnimInstance()->OnUnequipComplete.AddDynamic(this, &ABlasterCharacter::UnequipComplete);
	if(IsLocallyControlled())
	{
		GetSKGAnimInstance()->OnEquipComplete.AddDynamic(this, &ABlasterCharacter::EquipComplete);
	}
}

void ABlasterCharacter::BindToPlayerControllerEvent()
{
	// todo
}

void ABlasterCharacter::UnequipComplete()
{
	if(AActor* HeldActor = ShooterPawnComponent->GetHeldActor())
	{
		if(IInteractInterface* InteractInterface = Cast<IInteractInterface>(HeldActor))
		{
			FAttachmentTransformRules AttachmentTransformRules = CreateFirearmAttachmentRules();
			HeldActor->AttachToComponent(GetMesh(), AttachmentTransformRules, InteractInterface->GetAttachSocket());
			FirearmOnBack = HeldActor;
		}
	}
	if(FirearmToSwitchTo)
	{
		AttachEquipActor(FirearmToSwitchTo);
	}
	else
	{
		bIsPerformingAction = false;
	}
}

void ABlasterCharacter::EquipComplete()
{
	bIsPerformingAction = false;
	if(bWantsToAim)
	{
		StartAiming();
	}
}

void ABlasterCharacter::Destroyed()
{
	Super::Destroyed();

	if(ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
	}
	if(Buff)
	{
		Buff->Character = this;
		if(GetCharacterMovement())
		{
			Buff->SetInitialSpeeds(GetCharacterMovement()->MaxWalkSpeed, GetCharacterMovement()->MaxWalkSpeedCrouched);
			Buff->SetInitialJumpVelocity(GetCharacterMovement()->JumpZVelocity);
		}
	}
	if (LagCompensation)
	{
		LagCompensation->Character = this;
		if (Controller)
		{
			LagCompensation->Controller = Cast<ABlasterPlayerController>(Controller);
		}
	}
}

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(GetLocalRole() > ROLE_SimulatedProxy && IsLocallyControlled())
	{
		CalculateAimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if(TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}
	// HideCameraIfCharacterClose();
	PollInit();
}

void ABlasterCharacter::HideCameraIfCharacterClose()
{
	if(!IsLocallyControlled()) return;

	float DistanceToCamera = (FollowCamera->GetComponentLocation() - GetActorLocation()).Size();
	if(DistanceToCamera < CameraHideDistance)
	{
		GetMesh()->SetVisibility(false);
		if(Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			// 这使得该component的Owner无法看到该component
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
		if (Combat && Combat->SecondaryWeapon && Combat->SecondaryWeapon->GetWeaponMesh())
		{
			Combat->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if(Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			// 这使得该component的Owner无法看到该component
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
		if (Combat && Combat->SecondaryWeapon && Combat->SecondaryWeapon->GetWeaponMesh())
		{
			Combat->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}
void ABlasterCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if(AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// Map InRange to OutRange, Because the Rotation package has been packaged from float to uint16.
		// (0, 90) - (0, 90)  (-90, 0) - (270, 360)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}
/*
 * 旋转根骨骼，当Yaw_Offset到90度时进行lerp到0，重新指向瞄准方向 
 */
void ABlasterCharacter::CalculateAimOffset(float DeltaTime)
{
	if(!Combat || Combat->EquippedWeapon == nullptr)
		return;
	
	float Speed = CalculateSpeed();
	bool bInAir = GetCharacterMovement()->IsFalling();

	// this tell us that how AO_Yaw changed during turning.
	// FVector Dir_RootComponent = GetActorForwardVector();
	// FVector Dir_RootBone = GetMesh()->GetBoneAxis("root", EAxis::X).RotateAngleAxis(90, FVector(0,0,1));
	// DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + Dir_RootComponent * 500.f, FColor::Red);
	// DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + Dir_RootBone * 500.f, FColor::Blue);
	
	if(Speed == 0.f && !bInAir) // standing still, not jumping
	{
		bRotateRootBone = true;
		FRotator CurrentRotator = FRotator(0, GetBaseAimRotation().Yaw, 0);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentRotator, LastAimingRotator);
		AO_Yaw = DeltaAimRotation.Yaw;
		if(TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		// UE_LOG(LogTemp, Warning, TEXT("ServerAO_Yaw: %f"), AO_Yaw);
		bUseControllerRotationYaw = true;
		SetTurningInPlaceType(DeltaTime);
	}
	if(Speed > 0 || bInAir) // moving or jumping
	{
		bRotateRootBone = false;
		LastAimingRotator = FRotator(0, GetBaseAimRotation().Yaw, 0);
		AO_Yaw = 0;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}
	CalculateAO_Pitch();
}
/*
 * 不旋转根骨骼，只是当存在旋转变化时播放转向动画，角色骨骼模型始终指向目标
 */
void ABlasterCharacter::ProxyAimOffset()
{
	if(!Combat || !Combat->EquippedWeapon) return;
	bRotateRootBone = false;
	float Speed = CalculateSpeed();
	if(Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	
	ProxyRotationLastFrame = ProxyRotation;
	// 由于bUseControllerRotationYaw, 三行应该结果相同 
	// ProxyRotation = GetBaseAimRotation();
	// ProxyRotation = GetActorRotation();
	ProxyRotation = GetActorRotation();
	ProxyAO_Yaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;
	if (FMath::Abs(ProxyAO_Yaw) > ProxyTurnYawThreshold)
	{
		if (ProxyAO_Yaw > ProxyTurnYawThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (ProxyAO_Yaw < -ProxyTurnYawThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}
void ABlasterCharacter::SetTurningInPlaceType(float DeltaTime)
{
	if(AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if(AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if(TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		// let Yaw offset turn to zero, so match the rootComponent rotation.
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = InterpAO_Yaw;
		// UE_LOG(LogTemp, Log, TEXT("AO_YA_Interp: %s"), *FString::SanitizeFloat(AO_Yaw))
		if(FMath::Abs(AO_Yaw) < 15.f)
		{
			// UE_LOG(LogTemp, Log, TEXT("FinishTurning: %s"), *FString::SanitizeFloat(AO_Yaw))
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			// this make AO_Yaw to be zero,so match the rootComponent rotation
			LastAimingRotator = FRotator(0, GetBaseAimRotation().Yaw, 0);
		}
	}
	// UE_LOG(LogTemp, Warning, TEXT("AimOffset Yaw: %s"), *FString::SanitizeFloat(AO_Yaw));
}

void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	ProxyAimOffset();
	TimeSinceLastMovementReplication = 0;
}

void ABlasterCharacter::Eliminate(bool bLeftGame)
{
	DropOrDestroyWeapons();
	
	MulticastEliminate(bLeftGame);
}
void ABlasterCharacter::MulticastEliminate_Implementation(bool bLeftGame)
{
	bPlayerLeft = bLeftGame;
	if(BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDWeaponAmmo(0);
	}
	bEliminated = true;
	PlayElimMontage();

	// Eliminate effect
	if (DissolveMaterialInst)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInst, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 200.f);
	}
	StartDissolve();

	if(Combat)
	{
		Combat->FireButtonPressed(false);
	}
	
	// Disable character movement
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	// Disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GrenadeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// Elim bot particles
	if(ElimBotEffect)
	{
		FVector ElimBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200.f);
		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(this, ElimBotEffect,
			ElimBotSpawnPoint, GetActorRotation());
	}
	if(ElimBotSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ElimBotSound, GetActorLocation());
	}
	bool bHideSniperScope = IsLocallyControlled() &&
		Combat &&
			Combat->bIsAiming &&
				Combat->EquippedWeapon &&
					Combat->EquippedWeapon->GetWeaponType() == EBlasterWeaponType::EWT_SniperRifle;
	if(bHideSniperScope)
	{
		ShowSniperScopeWidget(false);
	}
	if(CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
	GetWorldTimerManager().SetTimer(
		RespawnTimer,
		this,
		&ThisClass::OnRespawnTimerFinished,
		RespawnTime
	);
}

void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// 这些bing是给到Controller的，至于controller如何影响Character，还需要另外设置
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ABlasterCharacter::Jump);
	PlayerInputComponent->BindAxis("MoveForward", this, &ThisClass::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ThisClass::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ThisClass::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ThisClass::LookUp);

	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ABlasterCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ThisClass::CrouchPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ThisClass::CrouchReleased);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ThisClass::AimPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ThisClass::AimReleased);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ThisClass::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ThisClass::StopFire);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ThisClass::ReloadButtonPressed);
	PlayerInputComponent->BindAction("ThrowGrenade", IE_Pressed, this, &ThisClass::ThrowGrenadePressed);
}
void ABlasterCharacter::MoveForward(float Value)
{
	if(Controller != nullptr && Value != 0.f)
	{
		FRotator ControllerRotator = Controller->GetControlRotation();
		// Y -- Z -- X for Pitch Yaw Roll
		const FRotator YawRotator = FRotator(0, ControllerRotator.Yaw, 0);
		// 以世界标准坐标系，应用该旋转，然后 获取旋转后的X方向的单位向量
		const FVector Direction = FRotationMatrix(YawRotator).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}
void ABlasterCharacter::MoveRight(float Value)
{
	if(Controller != nullptr && Value != 0.f)
	{
		FRotator ControllerRotator = Controller->GetControlRotation();
		// Y -- Z -- X for Pitch Yaw Roll
		const FRotator YawRotator = FRotator(0, ControllerRotator.Yaw, 0);
		// 以世界标准坐标系，应用该旋转，然后 获取旋转后的Y方向的单位向量
		const FVector Direction = FRotationMatrix(YawRotator).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}
void ABlasterCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}
void ABlasterCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}
void ABlasterCharacter::EquipButtonPressed()
{
	// if (Combat)
	// {
		ServerEquipButtonPressed();
	// }
}
void ABlasterCharacter::CrouchPressed()
{
	if(!bIsCrouched)
	{
		Crouch();
	}
}
void ABlasterCharacter::CrouchReleased()
{
	if(bIsCrouched)
	{
		UnCrouch();
	}
}

void ABlasterCharacter::ReloadButtonPressed()
{
	// if(Combat)
	// {
	// 	Combat->Reload();
	// }
	if(ShooterPawnComponent->GetHeldActor() && ShooterPawnComponent->GetHeldActor()->Implements<UWeaponInterface>())
	{
		Cast<IWeaponInterface>(ShooterPawnComponent->GetHeldActor())->Reload();
	}
}

void ABlasterCharacter::ThrowGrenadePressed()
{
	if(Combat)
	{
		Combat->ThrowGrenade();
	}
}

void ABlasterCharacter::AimPressed()
{
	if(Combat)
	{
		Combat->SetIsAiming(true);
	}
}
void ABlasterCharacter::AimReleased()
{
	if(Combat)
	{
		Combat->SetIsAiming(false);
	}
}
void ABlasterCharacter::Jump()
{
	if(bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}
void ABlasterCharacter::StartFire()
{
	// if(Combat)
	// {
	// 	Combat->FireButtonPressed(true);
	// }
	if(bIsPerformingAction) return;

	AActor* HeldActor = ShooterPawnComponent->GetHeldActor();
	if(HeldActor && HeldActor->Implements<UWeaponInterface>())
	{
		Cast<IWeaponInterface>(HeldActor)->Fire();
	}
}
void ABlasterCharacter::StopFire()
{
	// if(Combat)
	// {
	// 	Combat->FireButtonPressed(false);
	// }
	AActor* HeldActor = ShooterPawnComponent->GetHeldActor();
	if(HeldActor && HeldActor->Implements<UWeaponInterface>())
	{
		Cast<IWeaponInterface>(HeldActor)->StopFire();
	}
}

void ABlasterCharacter::PlayFireMontage(bool bAiming)
{
	if(!Combat || !Combat->EquippedWeapon) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayReloadMontage()
{
	if(!Combat || !Combat->EquippedWeapon) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;
		switch (Combat->EquippedWeapon->GetWeaponType())
		{
		case EBlasterWeaponType::EWT_AssaultRifle:
			SectionName = FName("Rifle");
			break;
		case EBlasterWeaponType::EWT_RocketLauncher:
			SectionName = FName("RocketLauncher");
			break;
		case EBlasterWeaponType::EWT_Pistol:
			SectionName = FName("Pistol");
			break;
		case EBlasterWeaponType::EWT_SubmachineGun:
			SectionName = FName("SubmachineGun");
			break;
		case EBlasterWeaponType::EWT_Shotgun:
			SectionName = FName("Shotgun");
			break;
		case EBlasterWeaponType::EWT_SniperRifle:
			SectionName = FName("SniperRifle");
			break;
		case EBlasterWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("GrenadeLauncher");
			break;
		}
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayElimMontage() const
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && EliminateMontage)
	{
		AnimInstance->Montage_Play(EliminateMontage);
	}
}

void ABlasterCharacter::PlayThrowGrenadeMontage() const
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && ThrowGrenadeMontage)
	{
		AnimInstance->Montage_Play(ThrowGrenadeMontage);
	}
}

void ABlasterCharacter::PlayHitReactMontage() const
{
	if(!Combat || !Combat->EquippedWeapon) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	// sample stop play hit when reloading,because it will block it. Need more fix
	if(AnimInstance && HitReactMontage && !AnimInstance->Montage_IsPlaying(ReloadMontage))
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PollInit()
{
	if(BlasterPlayerState == nullptr)
	{
		BlasterPlayerState = Cast<ABlasterPlayerState>(GetPlayerState());
		if(BlasterPlayerState)
		{
			BlasterPlayerState->AddToScore(0.f);
			BlasterPlayerState->AddToDefeats(0);
			SetTeamColor(BlasterPlayerState->GetTeamType());
			if(ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this)))
			{
				if(BlasterGameState->TopScoringPlayers.Contains(BlasterPlayerState))
				{
					OnLeadTheCrown();
				}
			}
		}
	}
}

FVector ABlasterCharacter::GetHitTarget() const
{
	if(!Combat)
	{
		return FVector();
	}
	return Combat->LocallyHitTarget;
}
void ABlasterCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if(bEliminated)
	{
		return;
	}
	if(bInCoolDownTime)
	{
		return;
	}
	ABlasterGameMode* BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	if(!BlasterGameMode) return;
	Damage = BlasterGameMode->CalculateDamage(InstigatedBy, Controller, Damage);
	if(Damage <= 0) return;

	float DamageToHealth = Damage;
	if (Shield > 0.f)
	{
		if (Shield >= Damage)
		{
			Shield = FMath::Clamp(Shield - Damage, 0.f, MaxShield);
			DamageToHealth = 0.f;
		}
		else
		{
			Shield = 0.f;
			DamageToHealth = FMath::Clamp(DamageToHealth - Shield, 0.f, Damage);
		}
	}

	Health = FMath::Clamp(Health - DamageToHealth, 0.f, MaxHealth);
	
	UpdateHealthHUD();
	UpdateHUDShield();
	PlayHitReactMontage();
	if(Health == 0.f)
	{
		BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
		ABlasterPlayerController* AttackerController = Cast<ABlasterPlayerController>(InstigatedBy);
		BlasterGameMode->PlayerEliminated(this, BlasterPlayerController, AttackerController);
	}
}
void ABlasterCharacter::OnRep_Health(float LastHealth)
{
	UpdateHealthHUD();
	if(Health < LastHealth)
	{
		PlayHitReactMontage();
	}
}

void ABlasterCharacter::OnRep_Shield(float LastShield)
{
	UpdateHUDShield();
	if (Shield < LastShield)
	{
		PlayHitReactMontage();
	}
}

void ABlasterCharacter::OnRespawnTimerFinished()
{
	ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	if(BlasterGameMode && !bPlayerLeft)
	{
		if(BlasterGameMode && BlasterGameMode->GetMatchState() != MatchState::CoolDown)
		{
			BlasterGameMode->RespawnCharacter(this, Controller);
		}
	}
	if(bPlayerLeft && IsLocallyControlled())
	{
		OnPlayerLeft.Broadcast();
	}
}


void ABlasterCharacter::OnLeadTheCrown_Implementation()
{
	if(!CrownComponent)
	{
		CrownComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(CrownCompSystem,
			GetMesh(),
			FName(),
			GetActorLocation() + FVector(0,0,110.f),
			FRotator::ZeroRotator,
			EAttachLocation::KeepWorldPosition,
			false
		);
	}
	if(CrownComponent)
	{
		CrownComponent->Activate();
	}
}

void ABlasterCharacter::OnLoseTheCrown_Implementation()
{
	if(CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
}


void ABlasterCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &ThisClass::ABlasterCharacter::UpdateDissolveMaterial);
	if(DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}
void ABlasterCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void ABlasterCharacter::DropOrDestroyWeapon(AWeapon* Weapon)
{
	if (Weapon == nullptr) return;
	if (Weapon->bDestroyWeapon)
	{
		Weapon->Destroy();
	}
	else
	{
		Weapon->Dropped();
	}
}

void ABlasterCharacter::DropOrDestroyWeapons()
{
	if (Combat)
	{
		if (Combat->EquippedWeapon)
		{
			DropOrDestroyWeapon(Combat->EquippedWeapon);
		}
		if (Combat->SecondaryWeapon)
		{
			DropOrDestroyWeapon(Combat->SecondaryWeapon);
		}
	}
}

void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if(OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = Weapon;
	if(IsLocallyControlled())
	{
		if(Weapon)
		{
			Weapon->ShowPickupWidget(true);
		}
	}
}

void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	// if (Combat)
	// {
		if (IInteractInterface* InteractWeapon = Cast<IInteractInterface>(OverlappingWeapon))
		{
			InteractWeapon->Interact(this);
			// Combat->EquipWeapon(OverlappingWeapon);
		}
		else if (ShouldSwapWeapons())
		{
			SwapWeapons();
		}
	// }
}

float ABlasterCharacter::CalculateSpeed() const
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}
bool ABlasterCharacter::IsAiming()
{
	return Combat && Combat->bIsAiming;
}
AWeapon* ABlasterCharacter::GetEquippedWeapon() const
{
	if(Combat)
	{
		return Combat->EquippedWeapon;
	}
	return nullptr;
}

ECombatState ABlasterCharacter::GetCombatState() const
{
	if(Combat)
	{
		return Combat->CombatState;
	}
	return ECombatState::ECS_MAX;
}

bool ABlasterCharacter::IsLocallyReloading()
{
	if (Combat == nullptr) return false;
	return Combat->bLocallyReloading;
}

bool ABlasterCharacter::IsEquippedWeapon()
{
	return Combat && Combat->EquippedWeapon;
}



