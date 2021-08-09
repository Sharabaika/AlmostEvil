// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


#include "ComponentRotator.h"
#include "InteractableInterface.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

UCLASS()
class ALMOSTEVIL_API APickup : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

	bool IsActive;	
public:
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	USkeletalMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UBoxComponent* BoxComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UComponentRotator* ComponentRotator;

	void SetIsActive(bool Active){IsActive = Active;}	
	APickup();

	UFUNCTION(Server, Reliable)
	virtual void Interact(AThirdPersonCharacter* Interactor) override;

	UFUNCTION(BlueprintImplementableEvent)
	void BP_Interact(AThirdPersonCharacter* Interactor);

	UFUNCTION(BlueprintNativeEvent)
	bool GetIsActive() override;

	virtual void BeginPlay() override;

	UFUNCTION(Server,Reliable)
	virtual void Server_OnPickup();
	
	UFUNCTION(Server,Reliable)
    virtual void Server_OnDrop();
	
protected:
	UFUNCTION(BlueprintNativeEvent)
	void BP_OnPickup();

	UFUNCTION(BlueprintNativeEvent)
	void BP_OnDrop();
	
	UFUNCTION(NetMulticast, Reliable)
	void Mult_OnPickup();

	UFUNCTION(NetMulticast, Reliable)
	void Mult_OnDrop();
};
