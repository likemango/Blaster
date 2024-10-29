// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponInterface.h"


// Add default functionality here for any IWeaponInterface functions that are not pure virtual.
void IWeaponInterface::Fire()
{
}

void IWeaponInterface::StopFire()
{
}

bool IWeaponInterface::CanFire()
{
	return true;
}

void IWeaponInterface::Reload()
{
}

void IWeaponInterface::ReloadComplete()
{
}

void IWeaponInterface::CycleFireMode()
{
}

EBlasterWeaponPriorityType IWeaponInterface::GetWeaponPriorityType()
{
	return EBlasterWeaponPriorityType::Primary;
}

void IWeaponInterface::ActionFinishedCycling()
{
}
