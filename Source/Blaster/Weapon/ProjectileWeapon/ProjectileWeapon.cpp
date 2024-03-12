// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"

#include "Blaster/Weapon/ProjectileWeapon/Projectile/Projectile.h"
#include "Engine/SkeletalMeshSocket.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	// Only server spawn the bullet, the bullet is replicated, so show in all clients!
	
	// ENetRole LocalRole = GetLocalRole();
	// FString LocalRoleText = StaticEnum<ENetRole>()->GetNameStringByValue((int64)LocalRole);
	// UE_LOG(LogTemp, Warning, TEXT("LocalRole: %s"), *LocalRoleText);
	
	if(!HasAuthority())
		return;
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if(MuzzleFlashSocket && OwnerPawn)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FActorSpawnParameters ActorSpawnParameters;
		ActorSpawnParameters.Owner = GetOwner();
		ActorSpawnParameters.Instigator = OwnerPawn;
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator ToTargetRotator = ToTarget.Rotation();
		UWorld* World = GetWorld();
		if(ProjectileClass && World)
		{
			AProjectile* NewProjectile = World->SpawnActor<AProjectile>(
				ProjectileClass,
				SocketTransform.GetLocation(),
				ToTargetRotator,
				ActorSpawnParameters
			);
		}
	}
}

