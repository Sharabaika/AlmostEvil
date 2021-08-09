// Fill out your copyright notice in the Description page of Project Settings.


#include "ComponentRotator.h"

#include "Components/SceneComponent.h"

// Sets default values for this component's properties
UComponentRotator::UComponentRotator()
{
	PrimaryComponentTick.bCanEverTick = true;

	YawSpeed = 90.0f;
}

void UComponentRotator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(isEnabled && !isPaused && ComponentToRotate)
	{
		ComponentToRotate->AddRelativeRotation(FRotator(0,DeltaTime*YawSpeed, 0));
	}
}

void UComponentRotator::Start()
{
	if(isEnabled)
	{
		isPaused = false;
	}
	else
	{
		isEnabled = true;
		if(ComponentToRotate)
		{
			initialRotator = ComponentToRotate->GetRelativeRotation();
		}
	}
}

void UComponentRotator::Pause()
{
		isPaused = true;
}

void UComponentRotator::Reset()
{
	isEnabled = false;
	isPaused = false;
	if(ComponentToRotate)
	{
		ComponentToRotate->SetRelativeRotation(initialRotator);
	}
}

