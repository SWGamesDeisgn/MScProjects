// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Payday/EKeycardTypes.h"
#include "Keycard.generated.h"

UCLASS()
class PAYDAY_API AKeycard : public AActor
{
	GENERATED_BODY()
	
public:

	UPROPERTY( EditAnywhere )
	USoundBase* cInteractSound;
	
	// Sets default values for this actor's properties
	AKeycard();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	UPROPERTY( EditAnywhere , BlueprintReadWrite )
	EKeycardTypes						eKeycardColour;
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:

	UPROPERTY( VisibleAnywhere )
	class UStaticMeshComponent*			cStaticMesh;
	UPROPERTY( VisibleAnywhere )
	class UBoxComponent*				cBoxCollider;
	UPROPERTY( VisibleAnywhere )
	class UReplicatedInteractable*		cReplicatedComp;
	class APaydayCharacter*				cCharacter;
	bool								bCharacterDetected;
	class APaydayPlayerState*			cPlayerState;
	
	void InteractionDetected();
	UFUNCTION()
		void OnOverlapBegin( UPrimitiveComponent* OverlappedComp , AActor* OtherActor , UPrimitiveComponent* OtherComp , int32 OtherBodyIndex , bool bFromSweep , const FHitResult& SweepResult );
	UFUNCTION()
		void OnOverlapEnd( UPrimitiveComponent* OverlappedComp , AActor* OtherActor , UPrimitiveComponent* OtherComp , int32 OtherBodyIndex );
};
