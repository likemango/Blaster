// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "DrawDebugHelpers.h"
#include "Blaster/BlasterComponents/LagCompensationComponent.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if(OwnerPawn == nullptr)
		return;
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if(MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);
		UWorld* World = GetWorld();
		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				World,
				MuzzleFlash,
				SocketTransform
			);
		}
		if (FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				FireSound,
				GetActorLocation()
			);
		}
		if(World && FireHit.bBlockingHit)
		{
			if (HitSound)
			{
				UGameplayStatics::PlaySoundAtLocation(
					this,
					HitSound,
					FireHit.ImpactPoint
				);
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

			if(!FireHit.GetActor())
				return;
			AController* FireInstigator = OwnerPawn->GetController();
			ABlasterCharacter* DamageCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
			// only cause damage on server
			if(FireInstigator && DamageCharacter)
			{
				if(HasAuthority() && !bUseServerSideRewind)
				{
					if(DamageCharacter && FireInstigator)
					{
						UGameplayStatics::ApplyDamage(
					DamageCharacter,
						Damage,
						FireInstigator,
						this,
						UDamageType::StaticClass());
					}
				}
				else if(!HasAuthority() && bUseServerSideRewind)
				{
					BlasterCharacter = BlasterCharacter == nullptr ? Cast<ABlasterCharacter>(OwnerPawn) : BlasterCharacter;
					BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(FireInstigator) : BlasterPlayerController;
					if (BlasterPlayerController && BlasterCharacter && BlasterCharacter->GetLagCompensation())
					{
						BlasterCharacter->GetLagCompensation()->ServerScoreRequest(
							BlasterCharacter,
							Start,
							HitTarget,
							BlasterPlayerController->GetServerTime() - BlasterPlayerController->SingleTripTime,
							this
						);
					}
				}
			}
		}
	}
}

void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	UWorld* World = GetWorld();
	if (World)
	{
		FVector End = TraceStart + (HitTarget - TraceStart) * 1.25f;

		World->LineTraceSingleByChannel(
			OutHit,
			TraceStart,
			End,
			ECollisionChannel::ECC_Visibility
		);
		FVector BeamEnd = End;
		if (OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}
		DrawDebugSphere(GetWorld(), BeamEnd, 16.f, 12, FColor::Orange, true);
		if (BeamParticle)
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
				World,
				BeamParticle,
				TraceStart,
				FRotator::ZeroRotator,
				true
			);
			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}
