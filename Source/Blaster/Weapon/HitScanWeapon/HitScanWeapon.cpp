// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

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
			
			// only cause damage on server
			if(HasAuthority() && FireHit.GetActor())
			{
				AController* FireInstigator = OwnerPawn->GetController();
				ABlasterCharacter* DamageCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
				if(DamageCharacter && FireInstigator)
				{
					UGameplayStatics::ApplyDamage(
				DamageCharacter,
					DamageValue,
					FireInstigator,
					this,
					UDamageType::StaticClass());
				}
			}
		}
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
	}
}

FVector AHitScanWeapon::TraceEndWithScatter(const FVector& FireStart, const FVector& HitTarget)
{
	FVector HitDirection = (HitTarget - FireStart).GetSafeNormal();
	FVector ScatterSphereLocation = FireStart + HitDirection * DistanceScatterSphere;

	FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::RandRange(0.f, ScatterSphereRadius);
	FVector RandLoc = ScatterSphereLocation + RandVec;
	FVector RandDir = (RandLoc - FireStart).GetSafeNormal();

	FVector ScatterLoc = FireStart + RandDir * TRACE_LINE_LENGTH;
	
	/*DrawDebugSphere(GetWorld(), ScatterSphereLocation, ScatterSphereRadius, 16, FColor::Red, true);
	DrawDebugSphere(GetWorld(), ScatterSphereLocation, 4, 12, FColor::Orange, true);
	DrawDebugLine(GetWorld(), FireStart, ScatterLoc, FColor::Cyan, true);*/
	return ScatterLoc;
}

void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	UWorld* World = GetWorld();
	if (World)
	{
		FVector End = bUseScatter ? TraceEndWithScatter(TraceStart, HitTarget) : TraceStart + (HitTarget - TraceStart) * 1.25f;

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
