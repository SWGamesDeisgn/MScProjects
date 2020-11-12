// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EKeycardTypes.generated.h"

UENUM( BlueprintType )
enum class EKeycardTypes : uint8
{
	KT_Blue UMETA( DisplayName = "Keycard Blue" ) ,
	KT_Green UMETA( DisplayName = "Keycard Green" )

};
