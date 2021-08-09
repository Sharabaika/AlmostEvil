// Fill out your copyright notice in the Description page of Project Settings.


#include "ThirdPersonCharacter.h"

#include "InteractableInterface.h"
#include "MyCharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

FAttachmentTransformRules GenerateAttachmentRules(){
		return FAttachmentTransformRules(
    EAttachmentRule::SnapToTarget,
    EAttachmentRule::SnapToTarget,
    EAttachmentRule::SnapToTarget,
    true);}

void AThirdPersonCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION(AThirdPersonCharacter, LookRotation, COND_SkipOwner);

	// TODO ??
	DOREPLIFETIME(AThirdPersonCharacter, isHoldingRifle);

	DOREPLIFETIME(AThirdPersonCharacter, IsAlive);
}

// Sets default values
AThirdPersonCharacter::AThirdPersonCharacter(const class FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer.SetDefaultSubobjectClass<UMyCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	isHoldingRifle = false;
	IsAlive= true;
	CanAim = true;
	
	GetCharacterMovement()->bOrientRotationToMovement = true;

	Weapons.SetNum(2);
	CurrentWeaponIndex = 0;
	
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &AThirdPersonCharacter::OnOverlapBegin);
	GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &AThirdPersonCharacter::OnOverlapEnd);

	HandSocketName = FName("gun_socket");
	PrimaryWeaponSocketName = FName("PrimaryWeaponSlot");
	SecondaryWeaponSocketName = FName("SecondaryWeaponSlot");
}

void AThirdPersonCharacter::TurnInput(float Rate)
{
	if(bAiming && IsAlive)
	{
		AddControllerYawInput(Rate);
		SetLookRotation(GetControlRotation());
	}
}

void AThirdPersonCharacter::PitchInput(float Rate)
{
	if(bAiming && IsAlive)
	{
		AddControllerPitchInput(Rate);
		SetLookRotation(GetControlRotation());
	}
}

void AThirdPersonCharacter::MoveForward(float Value)
{
	if(bAiming || !IsAlive)
		return;
	
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AThirdPersonCharacter::MoveRight(float Value)
{
	if(bAiming || !IsAlive)
		return;
	
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AThirdPersonCharacter::JumpPressed()
{
	TryJump();
}

void AThirdPersonCharacter::JumpReleased()
{
	if(!IsAlive)
		return;
	
	StopJumping();
	bJumping = false;
}

void AThirdPersonCharacter::CrouchPressed()
{
	if(!IsAlive)
		return;
	
	if(bIsCrouched)
	{
		UnCrouch();
	}else
	{
		Crouch();
	}
}

void AThirdPersonCharacter::SprintPressed()
{
	if(!IsAlive)
		return;
	
	if(bIsCrouched)
	{
		UnCrouch();
	}
	if(bAiming)
	{
		StopAimInput();
	}
	
	GetMyMovementComponent()->SetSprinting(true);
}

void AThirdPersonCharacter::SprintReleased()
{
	if(!IsAlive)
		return;
	
	GetMyMovementComponent()->SetSprinting(false);
}

void AThirdPersonCharacter::ShootPressed()
{
	AWeaponBase* Weapon = GetCurrentWeapon();
	if(Weapon)
	{
		StartShootingServer(Weapon->GetActorLocation(), Weapon->GetActorRotation());
	}
}

void AThirdPersonCharacter::ShootReleased()
{
	if(!IsAlive)
		return;
}

void AThirdPersonCharacter::AimInput()
{
	if(!GetCurrentWeapon())
		return;	
	if(!IsAlive || !CanAim)
		return;
	
	bAiming = true;
	bUseControllerRotationYaw = true;
	Controller->SetControlRotation(GetActorRotation());
	
	StartAimingServer(true);

	GetCurrentWeapon()->OnStartAiming();
}

void AThirdPersonCharacter::StopAimInput()
{
	if(!IsAlive)
		return;
	
	bAiming = false;
	bUseControllerRotationYaw = false;
	SetControllerRotationWithCamera();

	StartAimingServer(false);

	if(GetCurrentWeapon())
	{
		GetCurrentWeapon()->OnStopAiming();
	}
}

void AThirdPersonCharacter::EquipSlot1Input()
{
	EquipSlotInput(0);
}

void AThirdPersonCharacter::EquipSlot2Input()
{
	EquipSlotInput(1);
}

void AThirdPersonCharacter::EquipSlotInput(int slotIndex)
{
	if(GetCurrentWeapon())
		GetCurrentWeapon()->OnStopAiming();
	EquipSlot(slotIndex);
}

void AThirdPersonCharacter::InteractInput()
{
	InteractWithObjectOfInterest();
}

void AThirdPersonCharacter::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	IInteractableInterface* InteractableObject = Cast<IInteractableInterface>(OtherActor);
	if(InteractableObject && InteractableObject->GetIsActive())
	{
		SetObjectOfInterest(InteractableObject);
	}
}

void AThirdPersonCharacter::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	IInteractableInterface* InteractableObject = Cast<IInteractableInterface>(OtherActor);
	if(InteractableObject)
	{
		RemoveObjectOfInterest(InteractableObject);
	}
}

void AThirdPersonCharacter::StartAimingMulticast_Implementation(bool IsAiming)
{
	bAiming = IsAiming;
	bUseControllerRotationYaw = IsAiming;
}

void AThirdPersonCharacter::StartJumpingMulticast_Implementation(bool IsJumping)
{
	bJumping = IsJumping;
}

void AThirdPersonCharacter::TryJump_Implementation()
{
	if(bAiming || !CanJump() || !IsAlive)
		return;
	
	Jump();
	bJumping = true;
}

void AThirdPersonCharacter::StartAimingServer_Implementation(bool IsAiming)
{
	if(!IsAlive)
		return;
	
	StartAimingMulticast(IsAiming);
	bUseControllerRotationYaw = IsAiming;
}

void AThirdPersonCharacter::StartShootingServer_Implementation(FVector Location, FRotator Direction)
{
	if(!IsAlive)
		return;
	
	if(GetCurrentWeapon() && bAiming)
	{
		
		GetCurrentWeapon()->TryShoot(Location, Direction);
	}
}

void AThirdPersonCharacter::DropWeapon_Implementation(int Index)
{
	AWeaponBase* WeaponToDrop = Weapons[Index];
	if(WeaponToDrop)
	{
		WeaponToDrop->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		WeaponToDrop->Server_OnDrop();
	}
}

void AThirdPersonCharacter::PickupWeapon_Implementation(AWeaponBase* Weapon)
{
	if(!Weapon)
		return;
	
	RemoveObjectOfInterest(Weapon);
	Weapon->Server_OnPickup();
	Weapon->SetInstigator(GetInstigator());
	
	int Index = Weapon->GetIsPrimary() ? 1:0;
	
	DropWeapon(Index);
	
	Weapons[Index] = Weapon;
	SetWeaponListClient(Weapons);
	
	FName SocketToAttach = GetSocketName(Weapon->GetIsPrimary());
	Weapon->AttachToComponent(GetMesh(), GenerateAttachmentRules(), SocketToAttach);
	
	EquipWeapon(GetCurrentWeapon());
}

void AThirdPersonCharacter::EquipSlot_Implementation(int Index)
{
	if(Index<0 || Index>=Weapons.Num())
		return;

	if(Index == CurrentWeaponIndex)
		return;
		
	UnequipWeapon(GetCurrentWeapon());
	
	CurrentWeaponIndex = Index;
	SetCurrentWeaponClient(Index);
	
	EquipWeapon(GetCurrentWeapon());
}

void AThirdPersonCharacter::UnequipWeapon_Implementation(AWeaponBase* Weapon)
{
	if(!IsAlive)
		return;
	if(!Weapon)
		return;
	
	Weapon->AttachToComponent(GetMesh(), GenerateAttachmentRules(), GetSocketName(Weapon->GetIsPrimary()));
}

void AThirdPersonCharacter::EquipWeapon_Implementation(AWeaponBase* Weapon)
{
	if(!IsAlive)
    	return;

	if(Weapon){
		Weapon->AttachToComponent(GetMesh(), GenerateAttachmentRules(), HandSocketName);
		isHoldingRifle = true;
	}else
	{
		isHoldingRifle = false;
	}
}

void AThirdPersonCharacter::DeathEvent_Implementation()
{
	IsAlive = false;
	GetMesh()->SetSimulatePhysics(true);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
void AThirdPersonCharacter::SignCamera(AActor* Camera)
{
	if(IsLocallyControlled())
	{
		SignedCameraActor = Camera;
		SignCamera_Server(Camera);
		TrySetViewTargetToSignedCamera();
	}
}

void AThirdPersonCharacter::SetControllerRotationWithCamera()
{
	if(Controller && SignedCameraActor)
		Controller->SetControlRotation(FRotator(0,SignedCameraActor->GetActorRotation().Yaw, 0));
}

void AThirdPersonCharacter::TrySetViewTargetToSignedCamera_Implementation()
{
	if(Controller && IsLocallyControlled())
	{
		APlayerController* PlayerController = Cast<APlayerController>(Controller);
		if(PlayerController && SignedCameraActor)
			PlayerController->SetViewTargetWithBlend(SignedCameraActor);

		SetControllerRotationWithCamera();
	}
}

void AThirdPersonCharacter::SetLookRotation(FRotator Rot)
{
	LookRotation = Rot;
	SetLookRotationSync(Rot);
}

void AThirdPersonCharacter::SignCamera_Server_Implementation(AActor* Camera)
{
	SignedCameraActor = Camera;
	SignCamera_Multicast(Camera);
}

void AThirdPersonCharacter::SignCamera_Multicast_Implementation(AActor* Camera)
{
	if(IsLocallyControlled() && SignedCameraActor != Camera)
	{
		// if player possessed pawn before it replicated camera
		
		SignedCameraActor = Camera;
		TrySetViewTargetToSignedCamera();
	}
	else
	{
		SignedCameraActor = Camera;
	}
}

void AThirdPersonCharacter::SetObjectOfInterest(IInteractableInterface* NewObject)
{
	CurrentObjectOfInterest = NewObject;
	if(NewObject)
		NewObject->OnBeginInterest();
}

void AThirdPersonCharacter::RemoveObjectOfInterest(IInteractableInterface* NewObject)
{
	if(NewObject == CurrentObjectOfInterest)
		CurrentObjectOfInterest = nullptr;
}

void AThirdPersonCharacter::InteractWithObjectOfInterest_Implementation()
{
	if(CurrentObjectOfInterest)
		CurrentObjectOfInterest->Interact(this);
}

void AThirdPersonCharacter::SetLookRotationSync_Implementation(FRotator Rot)
{
	// TODO smooth on other clients
	LookRotation = Rot;
}

AWeaponBase* AThirdPersonCharacter::GetCurrentWeapon()
{
	return Weapons[CurrentWeaponIndex];
}

void AThirdPersonCharacter::Death_Implementation()
{
	IsAlive = false;
	OnDeath_Server();
	DeathEvent();
}

void AThirdPersonCharacter::SetWeaponListClient_Implementation(const TArray<AWeaponBase*> &WeaponList)
{
	Weapons = WeaponList;
}

void AThirdPersonCharacter::SetCurrentWeaponClient_Implementation(int WeaponIndex)
{
	CurrentWeaponIndex = WeaponIndex;
}

void AThirdPersonCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);
	
	PlayerInputComponent->BindAction("Jump", IE_Pressed,this, &AThirdPersonCharacter::JumpPressed);
	PlayerInputComponent->BindAction("Jump", IE_Released,this, &AThirdPersonCharacter::JumpReleased);
	
	PlayerInputComponent->BindAction("Crouch", IE_Pressed,this, &AThirdPersonCharacter::CrouchPressed);
	
	PlayerInputComponent->BindAction("Run", IE_Pressed,this, &AThirdPersonCharacter::SprintPressed);
	PlayerInputComponent->BindAction("Run", IE_Released,this, &AThirdPersonCharacter::SprintReleased);
	
	PlayerInputComponent->BindAction("Shoot", IE_Pressed,this, &AThirdPersonCharacter::ShootPressed);
	PlayerInputComponent->BindAction("Shoot", IE_Released,this, &AThirdPersonCharacter::ShootReleased);
	
	PlayerInputComponent->BindAction("Aim", IE_Pressed,this, &AThirdPersonCharacter::AimInput);
	PlayerInputComponent->BindAction("Aim", IE_Released,this, &AThirdPersonCharacter::StopAimInput);

	PlayerInputComponent->BindAction("Interact", IE_Pressed,this, &AThirdPersonCharacter::InteractInput);


	PlayerInputComponent->BindAxis("MoveForward", this, &AThirdPersonCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AThirdPersonCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &AThirdPersonCharacter::TurnInput);
	PlayerInputComponent->BindAxis("LookUp", this, &AThirdPersonCharacter::PitchInput);

	PlayerInputComponent->BindAction("EquipSlot1", IE_Pressed,this, &AThirdPersonCharacter::EquipSlot1Input);
	PlayerInputComponent->BindAction("EquipSlot2", IE_Pressed,this, &AThirdPersonCharacter::EquipSlot2Input);
}

UMyCharacterMovementComponent* AThirdPersonCharacter::GetMyMovementComponent() const
{
	return static_cast<UMyCharacterMovementComponent*>(GetCharacterMovement());
}

