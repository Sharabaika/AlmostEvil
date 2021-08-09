// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "WeaponBase.h"

#include "ProjectileWeaponBase.generated.h"

class AProjectile;
/**
 * 
 */
UCLASS()
class ALMOSTEVIL_API AProjectileWeaponBase : public AWeaponBase
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats")
	TSubclassOf<AProjectile> ProjectileClass;
public:
	
	virtual void Shoot_Implementation(FVector Location, FRotator Direction) override;
};
