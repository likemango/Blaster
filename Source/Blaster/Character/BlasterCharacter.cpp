// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"

#include "BlasterAnimInstance.h"
#include "Blaster/Blaster.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/GameModes/BlasterGameMode.h"
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

// Sets default values
ABlasterCharacter::ABlasterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// 设置Controller如何影响Character的朝向
	bUseControllerRotationYaw = false; // 如果是true，那么角色会持续朝向control的Yaw朝向
	GetCharacterMovement()->bOrientRotationToMovement = true; // 使用rotateRate去转向朝向

	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverHeadWidget"));
	WidgetComponent->SetupAttachment(RootComponent);
	
	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	// how fast character turn rotation. Should 'bOrientRotationToMovement' to true.
	GetCharacterMovement()->RotationRate = FRotator(0,850.f,0);
	// always spawn character
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ABlasterCharacter, Health);
}

void ABlasterCharacter::SetIsInCoolDownState(bool NewState)
{
	bInCoolDownTime = NewState;
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();

	UpdateHealthHUD();

	if(HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ThisClass::ReceiveDamage);
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
	HideCameraIfCharacterClose();
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
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if(Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			// 这使得该component的Owner无法看到该component
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
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

void ABlasterCharacter::Eliminate()
{
	// todo: drop the weapon
	if(Combat && Combat->EquippedWeapon)
	{
		Combat->EquippedWeapon->Dropped();
	}
	
	MulticastEliminate();
	GetWorldTimerManager().SetTimer(
		RespawnTimer,
		this,
		&ThisClass::OnRespawnTimerFinished,
		RespawnTime
	);
}
void ABlasterCharacter::MulticastEliminate_Implementation()
{
	if(BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDWeaponAmmo(0);
	}
	bEliminated = true;
	PlayElimMontage();

	// Eliminate effect
	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
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
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ThisClass::FirePressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ThisClass::FireReleased);
	PlayerInputComponent->BindAction("Reload", IE_Released, this, &ThisClass::ReloadButtonPressed);
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
	if (Combat)
	{
		if(HasAuthority())
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			ServerEquipButtonPressed();
		}
	}
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
	if(Combat)
	{
		Combat->Reload();
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
void ABlasterCharacter::FirePressed()
{
	if(Combat)
	{
		Combat->FireButtonPressed(true);
	}
}
void ABlasterCharacter::FireReleased()
{
	if(Combat)
	{
		Combat->FireButtonPressed(false);
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
		}
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && EliminateMontage)
	{
		AnimInstance->Montage_Play(EliminateMontage);
	}
}

void ABlasterCharacter::PlayHitReactMontage()
{
	if(!Combat || !Combat->EquippedWeapon) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && HitReactMontage)
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
	if(bInCoolDownTime)
	{
		return;
	}
	Health = FMath::Clamp(Health-Damage, 0, MaxHealth);
	UpdateHealthHUD();
	PlayHitReactMontage();
	if(Health == 0.f)
	{
		// GameMode只能在服务器端获取到
		ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
		if(BlasterGameMode)
		{
			BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
			ABlasterPlayerController* AttackerController = Cast<ABlasterPlayerController>(InstigatedBy);
			BlasterGameMode->PlayerEliminated(this, BlasterPlayerController, AttackerController);
		}
	}
}
void ABlasterCharacter::OnRep_Health()
{
	UpdateHealthHUD();
	PlayHitReactMontage();
}

void ABlasterCharacter::OnRespawnTimerFinished()
{
	ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	if(BlasterGameMode && BlasterGameMode->GetMatchState() != MatchState::CoolDown)
	{
		BlasterGameMode->RespawnCharacter(this, Controller);
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

void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if(OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = Weapon;
	if(IsLocallyControlled())
	{
		// if it is control on the server
		if(OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}	
}
void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if(LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
	if(OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
}
void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
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

bool ABlasterCharacter::IsEquippedWeapon()
{
	return Combat && Combat->EquippedWeapon;
}



