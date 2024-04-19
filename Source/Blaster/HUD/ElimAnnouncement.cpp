// Fill out your copyright notice in the Description page of Project Settings.


#include "ElimAnnouncement.h"

#include "Components/TextBlock.h"

void UElimAnnouncement::SetElimAnnouncementText(FString AttackerName, FString VictimName) const
{
	FString String = AttackerName.Append(TEXT(" killed ")).Append(VictimName);
	AnnouncementText->SetText(FText::FromString(String));
}
