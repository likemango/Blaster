// Fill out your copyright notice in the Description page of Project Settings.


#include "Casing.h"

#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

ACasing::ACasing()
{
	PrimaryActorTick.bCanEverTick = false;

	CasingMeshComp = CreateDefaultSubobject<UStaticMeshComponent>("CasingMesh");
	SetRootComponent(CasingMeshComp);
	CasingMeshComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	CasingMeshComp->SetSimulatePhysics(true);
	CasingMeshComp->SetEnableGravity(true);
	CasingMeshComp->SetNotifyRigidBodyCollision(true);
	ShellEjectionImpulse = 10.f;
}

void ACasing::BeginPlay()
{
	Super::BeginPlay();
	
	CasingMeshComp->OnComponentHit.AddDynamic(this, &ThisClass::ACasing::OnHit);
	
	CasingMeshComp->AddImpulse(ShellEjectionImpulse * GetActorForwardVector());
}

void ACasing::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if(ShellSoundCue && !bShellHitGround)
	{
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), ShellSoundCue, GetActorLocation());
		bShellHitGround = true;

		// 调用定时器函数，在2秒后执行销毁Actor的函数
		GetWorldTimerManager().SetTimer(DestroyTimerHandle, this, &ThisClass::DestroyActor, 2.0f, false);
	}
}

void ACasing::DestroyActor()
{
	Destroy();
}

