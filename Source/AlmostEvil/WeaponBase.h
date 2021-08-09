// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


#include "Pickup.h"
#include "Components/ArrowComponent.h"
#include "Engine/DataTable.h"

#include "WeaponBase.generated.h"


UCLASS()
class ALMOSTEVIL_API AWeaponBase : public APickup
{
	GENERATED_BODY()
public:	
	AWeaponBase();

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Weapon")
	UArrowComponent* MuzzleLocation;

	UFUNCTION(Server, Reliable, BlueprintCallable)
    void TryShoot(FVector Location, FRotator Direction);
	
	UFUNCTION()
    bool WasteAmmo();
	
	UFUNCTION(Server, Reliable, BlueprintCallable)
    void Reload(int Quantity);

	virtual void Server_OnPickup() override;
	
	virtual void Server_OnDrop() override;
		
	bool GetIsPrimary() const { return IsPrimary;};

	virtual void Interact(AThirdPersonCharacter* Interactor) override;
	
	UFUNCTION(BlueprintImplementableEvent)
    void OnStartAiming();

	UFUNCTION(BlueprintImplementableEvent)
    void OnStopAiming();

	void BeginPlay() override;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stats")
	FDataTableRowHandle AmmoClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats")
	bool IsPrimary;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats")
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats")
	float MaxDistance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats")
	int MagCapacity;

	UPROPERTY(Replicated,EditAnywhere, BlueprintReadOnly, Category="Stats")
	int RemainingAmmo;

	UFUNCTION(BlueprintImplementableEvent, Category="Effects")
	void PlayShootingEffects();

	UFUNCTION(BlueprintImplementableEvent)
	void OnShot();

	UFUNCTION(NetMulticast, Reliable)
	void NotifyAll_Shot();

	UFUNCTION(Client, Reliable)
	void NotifyClient_Shot();

	UFUNCTION(BlueprintNativeEvent)
	void Shoot(FVector Location, FRotator Direction);
	

};
