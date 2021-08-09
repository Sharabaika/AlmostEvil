// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBase.h"

#include "DrawDebugHelpers.h"
#include "ThirdPersonCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

void AWeaponBase::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME( AWeaponBase, RemainingAmmo );
}

void AWeaponBase::NotifyAll_Shot_Implementation()
{
	PlayShootingEffects();
}

void AWeaponBase::NotifyClient_Shot_Implementation()
{
	OnShot();

	// TODO UPDATE AMMOUI ONLY ON SHOTS
}

void AWeaponBase::Shoot_Implementation(FVector Location, FRotator Direction)
{
	FHitResult OutHit;
	FVector Start = Location +Direction.RotateVector(MuzzleLocation->GetRelativeLocation());
	FVector End =  Start + Direction.Vector()*MaxDistance;
	// DrawDebugLine(GetWorld(), Start, End, FColor::Red, true);
	FCollisionQueryParams TraceParams(TEXT("LineOfSight_Trace"), false, this);
	
	if(GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, TraceParams)) 
	{
		// DrawDebugSphere(GetWorld(),OutHit.Location,50.f,10,FColor(), true, 10);
		if(OutHit.bBlockingHit)
		{			
			// OutHit.Actor->TakeDamage(Damage, FDamageEvent(),  GetInstigatorController(), this);
			// OutHit.Actor->ReceivePointDamage(Damage, UDamageType)
			UGameplayStatics::ApplyPointDamage(
                OutHit.GetActor(),
                Damage,
                Direction.Vector(),
                OutHit,
                GetInstigatorController(),
                this,
                UDamageType::StaticClass());
			
			// GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, OutHit.Actor->GetName());
		}
	}
}

// Sets default values
AWeaponBase::AWeaponBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	MuzzleLocation = CreateDefaultSubobject<UArrowComponent>(TEXT("MuzzleLocationKEK"));
	MuzzleLocation->SetupAttachment(RootComponent);

	IsPrimary = true;
	MaxDistance = 1000.0f;
	Damage = 35.f;
	MagCapacity = 30;
	RemainingAmmo = MagCapacity;

	SetIsActive(true);
}

void AWeaponBase::TryShoot_Implementation(FVector Location, FRotator Direction)
{
	if(!WasteAmmo())
		return;
	
	Shoot(Location, Direction);
	NotifyAll_Shot();
	NotifyClient_Shot();
}

bool AWeaponBase::WasteAmmo()
{
	if(RemainingAmmo>0)
	{
		RemainingAmmo-=1;
		return true;
	}
	return false;
}

void AWeaponBase::Reload_Implementation(int Quantity)
{
	if( Quantity+RemainingAmmo <= MagCapacity)
		RemainingAmmo = Quantity+RemainingAmmo;
}

void AWeaponBase::Server_OnPickup()
{
	Mult_OnPickup();
	BoxComponent->SetSimulatePhysics(false);
	SetIsActive(false);
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, "pick");
}

void AWeaponBase::Server_OnDrop()
{
	Mult_OnDrop();	
	BoxComponent->SetSimulatePhysics(true);	
	SetIsActive(true);

}

void AWeaponBase::Interact(AThirdPersonCharacter* Interactor)
{
	Interactor->PickupWeapon(this);
}

void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	// do physics only on server
	if(HasAuthority() == false)
	{
		BoxComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
		BoxComponent->SetSimulatePhysics(false);
	}
}

