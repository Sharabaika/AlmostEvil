// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ComponentRotator.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ALMOSTEVIL_API UComponentRotator : public UActorComponent
{
	GENERATED_BODY()

	bool isEnabled;
	bool isPaused;

	UPROPERTY(EditAnywhere)
	float YawSpeed;


	FRotator initialRotator;
public:	
	UComponentRotator();

	UPROPERTY(BlueprintReadWrite)
	USceneComponent* ComponentToRotate;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void Start();

	UFUNCTION(BlueprintCallable)
	void Pause();
	
	UFUNCTION(BlueprintCallable)
	void Reset();
};
