// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"

#include "Blaster/Weapon/Weapon.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"


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
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// 这些bing是给到Controller的，至于controller如何影响Character，还需要另外设置
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAxis("MoveForward", this, &ThisClass::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ThisClass::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ThisClass::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ThisClass::LookUp);
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlapWeapon, COND_OwnerOnly);
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

void ABlasterCharacter::OnRep_OverlapWeapon(AWeapon* LastWeapon)
{
	if(LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
	if(OverlapWeapon)
	{
		OverlapWeapon->ShowPickupWidget(true);
	}
}

void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if(OverlapWeapon)
	{
		OverlapWeapon->ShowPickupWidget(false);
	}
	OverlapWeapon = Weapon;
	if(IsLocallyControlled())
	{
		if(OverlapWeapon)
		{
			OverlapWeapon->ShowPickupWidget(true);
		}
	}	
}




