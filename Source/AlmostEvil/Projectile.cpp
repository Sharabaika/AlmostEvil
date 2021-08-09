// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"



#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(FName("Collision"));
	RootComponent = BoxComponent;

	Mesh = CreateDefaultSubobject<UStaticMesh>(FName("Mesh"));
	
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(FName("ProjectileMovement"));
}