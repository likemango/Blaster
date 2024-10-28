// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"

#include "Blaster/Weapon/ProjectileWeapon/Projectile/Projectile.h"
#include "Engine/SkeletalMeshSocket.h"

PRAGMA_DISABLE_OPTIMIZATION

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	// Only server spawn the bullet, the bullet is replicated, so show in all clients!
	
	// ENetRole LocalRole = GetLocalRole();
	// FString LocalRoleText = StaticEnum<ENetRole>()->GetNameStringByValue((int64)LocalRole);
	// UE_LOG(LogTemp, Warning, TEXT("LocalRole: %s"), *LocalRoleText);
	
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	UWorld* World = GetWorld();
	if(!MuzzleFlashSocket || !World || !InstigatorPawn || !ProjectileClass)
		return;
	
	FActorSpawnParameters ActorSpawnParameters;
	ActorSpawnParameters.Owner = GetOwner();
	ActorSpawnParameters.Instigator = InstigatorPawn;
	
	FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	FVector ToTarget = HitTarget - SocketTransform.GetLocation();
	FRotator ToTargetRotator = ToTarget.Rotation();

	AProjectile* SpawnedProjectile = nullptr;
	if(bUseServerSideRewind)
	{
		if(InstigatorPawn->HasAuthority())
		{
			if(InstigatorPawn->IsLocallyControlled()) //Server host player, just use Replicated projectile
			{
				SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass,SocketTransform.GetLocation(),ToTargetRotator,ActorSpawnParameters);
				SpawnedProjectile->bUseServerSideRewind = false;
				SpawnedProjectile->DamageValue = Damage;
			}
			else //server, not locally controlled, spawn non-replicate projectile, ssr
			{
				SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass,SocketTransform.GetLocation(),ToTargetRotator,ActorSpawnParameters);
				SpawnedProjectile->bUseServerSideRewind = true;
			}
		}
		else
		{
			if(InstigatorPawn->IsLocallyControlled()) //Client player, use ssr, no replicated
			{
				SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass,SocketTransform.GetLocation(),ToTargetRotator,ActorSpawnParameters);
				SpawnedProjectile->bUseServerSideRewind = true;
				SpawnedProjectile->TraceStart = SocketTransform.GetLocation();
				SpawnedProjectile->InitialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->InitialSpeed;
				SpawnedProjectile->DamageValue = Damage;
			}
			else
			{
				SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass,SocketTransform.GetLocation(),ToTargetRotator,ActorSpawnParameters);
				SpawnedProjectile->bUseServerSideRewind = false;
			}
		}
	}
	else
	{
		if(InstigatorPawn->HasAuthority())
		{
			SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass,SocketTransform.GetLocation(),ToTargetRotator,ActorSpawnParameters);
			SpawnedProjectile->bUseServerSideRewind = false;
			SpawnedProjectile->DamageValue = Damage;
		}
	}
}

PRAGMA_ENABLE_OPTIMIZATION