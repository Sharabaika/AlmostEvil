// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


#include "IDamageable.h"
#include "MyCharacterMovementComponent.h"
#include "WeaponBase.h"
#include "GameFramework/Character.h"
#include "ThirdPersonCharacter.generated.h"

class  IInteractableInterface;

UCLASS()
class ALMOSTEVIL_API AThirdPersonCharacter : public ACharacter, public IIDamageable
{	
GENERATED_BODY()

public:
	
	AThirdPersonCharacter(const class FObjectInitializer& ObjectInitializer);

#pragma region Flags
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Mesh sockets")
	FName HandSocketName;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Mesh sockets")
	FName PrimaryWeaponSocketName;
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Mesh sockets")
	FName SecondaryWeaponSocketName;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Movement flags")
	bool CanAim;
	
	UPROPERTY(BlueprintReadOnly, Replicated, Category="Movement flags")
	bool bAiming;
	
	UPROPERTY(BlueprintReadOnly, Replicated, Category="Movement flags")
	bool bJumping;
	
	UPROPERTY(BlueprintReadOnly, Replicated, Category="Movement flags")
	FRotator LookRotation;

	UPROPERTY(BlueprintReadOnly, Replicated, Category="Movement flags")
	bool isHoldingRifle;

	UPROPERTY(BlueprintReadOnly, Replicated)
	bool IsAlive;
#pragma endregion

#pragma region Weapons
	
	UPROPERTY(BlueprintReadOnly, Category="Inventory")
	TArray<AWeaponBase*> Weapons;
	
	UPROPERTY(BlueprintReadOnly)
	int CurrentWeaponIndex;
	
#pragma endregion	

#pragma region WeaponHandling
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void EquipSlot(int Index);
	
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void DropWeapon(int Index);
	
	UFUNCTION(Server, Reliable)
	void PickupWeapon(AWeaponBase* Weapon);	
	
	UFUNCTION(Server, Reliable)
	void EquipWeapon(AWeaponBase* Weapon);

	UFUNCTION(Server, Reliable)
	void UnequipWeapon(AWeaponBase* Weapon);

	UFUNCTION(BlueprintCallable)
	AWeaponBase* GetCurrentWeapon();

	UFUNCTION(BlueprintCallable)
	FName GetSocketName(bool IsPrimaryWeapon){ return IsPrimaryWeapon? PrimaryWeaponSocketName:SecondaryWeaponSocketName;};

#pragma endregion

#pragma region Death
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void DeathEvent();

#pragma endregion
	
#pragma region CameraManagment

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SignCamera(AActor* Camera);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Camera")
	void TrySetViewTargetToSignedCamera();
	
#pragma endregion 

protected:
	
#pragma region Input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void TurnInput(float Rate);
	void PitchInput(float Rate);
	void MoveForward(float Value);
	void MoveRight(float Value);

	void JumpPressed();
	void JumpReleased();

	void CrouchPressed();

	void SprintPressed();
	void SprintReleased();

	void ShootPressed();
	void ShootReleased();

	UFUNCTION(BlueprintCallable, Category="Input")
	void StopAimInput();
	void AimInput();

	void EquipSlot1Input();
	void EquipSlot2Input();
	void EquipSlotInput(int slotIndex);
	
	void InteractInput();
#pragma endregion 

#pragma region Interactions

	IInteractableInterface* CurrentObjectOfInterest;
	
	UFUNCTION()
    void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd( UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void SetObjectOfInterest(IInteractableInterface* NewObject);
	void RemoveObjectOfInterest(IInteractableInterface* NewObject);

	UFUNCTION(Server, Reliable)
	void InteractWithObjectOfInterest();
#pragma endregion

#pragma region SyncMovement
	
	UFUNCTION(Server, Reliable)
    void TryJump();

	UFUNCTION(NetMulticast, Unreliable)
    void StartJumpingMulticast(bool Jumping);

	UFUNCTION(Server, Reliable)
    void SetLookRotationSync(FRotator Rot);

	// Gets the character's MyCustomMovementComponent
	UFUNCTION(BlueprintCallable, Category = "Movement")
    UMyCharacterMovementComponent* GetMyMovementComponent() const;
#pragma endregion 

#pragma region GunPlay
	UFUNCTION(Server, Reliable)
	void StartAimingServer(bool IsAiming);

	UFUNCTION(NetMulticast, Unreliable)
	void StartAimingMulticast(bool Aiming);
	
	UFUNCTION(server, Reliable)
	void StartShootingServer(FVector Location, FRotator Direction);

#pragma endregion
	
#pragma region Death
	virtual void Death_Implementation() override;

	UFUNCTION(BlueprintImplementableEvent)
	void OnDeath_Server();
	
#pragma endregion 

private:
	
#pragma region CameraManagment
	UPROPERTY()
	AActor* SignedCameraActor;

	UFUNCTION(Server, Reliable)
    void SignCamera_Server(AActor* Camera);

	UFUNCTION(NetMulticast, Reliable)
    void SignCamera_Multicast(AActor* Camera);

	UFUNCTION(Category = "Movement")
    void SetLookRotation(FRotator Rot);

	UFUNCTION(Category = "Camera")
    void SetControllerRotationWithCamera();
	    

#pragma endregion
	
#pragma region WeaponHandling

	UFUNCTION(Client,Reliable)
    void SetCurrentWeaponClient(int WeaponIndex);

	UFUNCTION(Client, Reliable)
    void SetWeaponListClient(const TArray<AWeaponBase*> &WeaponList);
#pragma endregion 
};
