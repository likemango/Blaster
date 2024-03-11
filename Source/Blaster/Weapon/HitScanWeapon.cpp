// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if(OwnerPawn == nullptr)
		return;
	AController* FireInstigator = OwnerPawn->GetController();
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if(MuzzleFlashSocket && FireInstigator)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());

		FVector Start = SocketTransform.GetLocation();
		FVector End = Start + (HitTarget -  Start) * 1.25f;
		UWorld* World = GetWorld();
		FHitResult FireHit;
		if(World)
		{
			World->LineTraceSingleByChannel(
				FireHit,
				Start,
				End,
				ECC_Visibility
			);
			if(FireHit.bBlockingHit && FireHit.GetActor())
			{
				ABlasterCharacter* DamageCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
				if(DamageCharacter)
				{
					// only cause damage on server
					if(HasAuthority())
					{
						UGameplayStatics::ApplyDamage(
					DamageCharacter,
						DamageValue,
						FireInstigator,
						this,
						UDamageType::StaticClass());
					}
				}
				if(ImpactParticle)
				{
					UGameplayStatics::SpawnEmitterAtLocation(
						this,
						ImpactParticle,
						FireHit.Location,
						FireHit.ImpactNormal.Rotation()
					);
				}
			}
		}
	}
}
