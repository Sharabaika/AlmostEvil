// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickup.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"

// Sets default values
APickup::APickup()
{
	PrimaryActorTick.bCanEverTick = true;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(FName("BoxTest"));

	BoxComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	BoxComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	BoxComponent->SetSimulatePhysics(true);

	
	RootComponent=BoxComponent;

	Mesh = CreateOptionalDefaultSubobject<USkeletalMeshComponent>(TEXT("awdawdwad"));
	Mesh->SetGenerateOverlapEvents(false);
	Mesh->SetSimulatePhysics(false);
	Mesh->SetupAttachment(BoxComponent);

	ComponentRotator = CreateDefaultSubobject<UComponentRotator>(TEXT("Rotator"));
	ComponentRotator->ComponentToRotate = Mesh;
	

	bReplicates = true;
	bReplicateMovement = true;
}

bool APickup::GetIsActive_Implementation()
{
	return IsActive;
}

void APickup::BeginPlay()
{
	Super::BeginPlay();

	
	if(IsRunningDedicatedServer() == false)
	{
		ComponentRotator->Start();
	}
}

void APickup::BP_OnDrop_Implementation()
{
	if(IsRunningDedicatedServer() == false){
		ComponentRotator->Start();
	}
}

void APickup::BP_OnPickup_Implementation()
{
	if(IsRunningDedicatedServer() == false){
		ComponentRotator->Reset();
	}
}

void APickup::Mult_OnDrop_Implementation()
{
	BP_OnDrop();
}

void APickup::Mult_OnPickup_Implementation()
{
	BP_OnPickup();
}

void APickup::Server_OnDrop_Implementation()
{
	Mult_OnDrop();
}

void APickup::Server_OnPickup_Implementation()
{
	Mult_OnPickup();
}


void APickup::Interact_Implementation(AThirdPersonCharacter* Interactor)
{
	BP_Interact(Interactor);
}
