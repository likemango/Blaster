// Fill out your copyright notice in the Description page of Project Settings.


#include "CrouchFeetStepNotify.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

void UCrouchFeetStepNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                   const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if(!CrouchFeetStepSoundWave)
		return;
	
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(MeshComp->GetOwner());
	if(BlasterCharacter && BlasterCharacter->IsLocallyControlled())
	{
		UGameplayStatics::PlaySound2D(GetWorld(), CrouchFeetStepSoundWave, 0.3);
	}
}
