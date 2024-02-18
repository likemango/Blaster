// Fill out your copyright notice in the Description page of Project Settings.


#include "Casing.h"

ACasing::ACasing()
{
	PrimaryActorTick.bCanEverTick = false;

	CasingStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("CasingMesh");
	SetRootComponent(CasingStaticMeshComponent);
}

void ACasing::BeginPlay()
{
	Super::BeginPlay();
	
}

