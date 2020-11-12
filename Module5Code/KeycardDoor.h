// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Payday/EKeycardTypes.h"
#include "KeycardDoor.generated.h"

UCLASS()
class PAYDAY_API AKeycardDoor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AKeycardDoor();



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	


	// Called every frame
	virtual void Tick(float DeltaTime) override;
private:
	UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true") )
	EKeycardTypes						eKeycardColour;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true") )
	bool								bInteractionDestoysKeycards;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true") )
	float								fOpeningAngle;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true") )
	float								fOpeningSpeed;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true") )
	USoundBase*							OpenedSound;
	UPROPERTY( VisibleAnywhere )
	class UStaticMeshComponent*			cStaticMesh;
	UPROPERTY( VisibleAnywhere )
	class UBoxComponent*				cBoxCollider;
	UPROPERTY( VisibleAnywhere )
	class UReplicatedInteractable*		cReplicatedComp;
	class APaydayCharacter*				cCharacter;
	bool								bCharacterDetected;
	class APaydayPlayerState*			cPlayerState;
	bool								bRotate;
	float								fYawValue;
	bool								bInvertRotation;

	void InteractionDetected();
	UFUNCTION()
		void OnOverlapBegin( UPrimitiveComponent* OverlappedComp , AActor* OtherActor , UPrimitiveComponent* OtherComp , int32 OtherBodyIndex , bool bFromSweep , const FHitResult& SweepResult );
	UFUNCTION()
		void OnOverlapEnd( UPrimitiveComponent* OverlappedComp , AActor* OtherActor , UPrimitiveComponent* OtherComp , int32 OtherBodyIndex );
	void RotateMe();

};
