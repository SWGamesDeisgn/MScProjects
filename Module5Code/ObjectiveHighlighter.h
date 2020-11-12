// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Payday/Core/GameStates/PaydayGameState.h"


#include "ObjectiveHighlighter.generated.h"

UCLASS()
class PAYDAY_API AObjectiveHighlighter : public AActor
{
	GENERATED_BODY()

	/*
	 * Game State references
	 */
	class APaydayGameState* cPaydayGameState;
	
public:	
	// Sets default values for this actor's properties
	AObjectiveHighlighter();

	/*
	 * Actor Virtual Methods
	 */
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick( float DeltaTime ) override;

	/**
	 * \brief Highlight shape
	 */
	UPROPERTY( VisibleAnywhere, Category = "Static Mesh", meta = ( AllowPrivateAccess = "true" ) )
	class UStaticMeshComponent*			cStaticMeshComponent;
	UPROPERTY( EditAnywhere )
	class AActor* cOwningActor;

	/*
	 * Other settings
	 */
	
	UPROPERTY( EditAnywhere, DisplayName = "Target Objective" )
	EPaydayObjectiveState eTargetState;
	
	UPROPERTY( EditAnywhere, DisplayName = "Highlight Object by Default" )
	bool bIsDefaultHighlighted;

	/*
	 * Event Handlers
	 */
	UFUNCTION()
	void OnPaydayObjectiveChanged( EPaydayObjectiveState eState );
private:
	bool bHasBeenInteracted;
};
