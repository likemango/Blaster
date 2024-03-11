// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"


void AShotgun::Fire(const FVector& HitTarget)
{
	AWeapon::Fire(HitTarget);
	
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if(OwnerPawn == nullptr)
		return;
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if(MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		TMap<ABlasterCharacter*, uint32> HitMap;

		FVector Start = SocketTransform.GetLocation();
		for(uint32 i = 0; i < ScatterPellets; ++i)
		{
#pragma region GeneralTraceEffect
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);
			if (ImpactParticle)
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					ImpactParticle,
					FireHit.ImpactPoint,
					FireHit.ImpactNormal.Rotation()
				);
			}
			if (HitSound)
			{
				// lower sound
				UGameplayStatics::PlaySoundAtLocation(
					this,
					HitSound,
					FireHit.ImpactPoint,
					.5f,
					FMath::FRandRange(-.5f, .5f)
				);
			}
#pragma endregion

			if(HasAuthority())
			{
				ABlasterCharacter* VictimCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
				AController* InstigatorController = OwnerPawn->GetController();
				if (VictimCharacter && InstigatorController)
				{
					if (HitMap.Contains(VictimCharacter))
					{
						HitMap[VictimCharacter]++;
					}
					else
					{
						HitMap.Emplace(VictimCharacter, 1);
					}
				}
			}
		}

		if(HasAuthority())
		{
			AController* InstigatorController = OwnerPawn->GetController();
			for (auto HitPair : HitMap)
			{
				if (HitPair.Key && InstigatorController)
				{
					UGameplayStatics::ApplyDamage(
						HitPair.Key,
						DamageValue * HitPair.Value,
						InstigatorController,
						this,
						UDamageType::StaticClass()
					);
				}
			}
		}
	}
}
