// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeaponBase.h"


#include "Projectile.h"
#include "Engine/Engine.h"

void AProjectileWeaponBase::Shoot_Implementation(FVector Location, FRotator Direction)
{
	FVector Start = Location +Direction.RotateVector(MuzzleLocation->GetRelativeLocation());
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = GetInstigator();
	AProjectile* Projectile =GetWorld()->SpawnActor<AProjectile>(ProjectileClass,Start, Direction, SpawnInfo);
}
