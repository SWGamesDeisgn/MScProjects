// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MoneyPickup.generated.h"

UCLASS()
class PAYDAY_API AMoneyPickup : public AActor
{
	GENERATED_BODY()
	
public:

	UPROPERTY( EditAnywhere )
	USoundBase* cInteractSound;
	UPROPERTY( EditAnywhere )
	UParticleSystem* cParticles;
	// Sets default values for this actor's properties
	AMoneyPickup();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY( VisibleAnywhere )
		class UStaticMeshComponent*			cStaticMesh;
	UPROPERTY( VisibleAnywhere )
		class UBoxComponent*				cBoxCollider;
	UPROPERTY( VisibleAnywhere )
		class UReplicatedInteractable*		cReplicatedComp;
	class APaydayCharacter*					cCharacter;
	bool									bCharacterDetected;
	UPROPERTY(EditAnywhere, category = "Design Stat")
		float								fMoney;
	class APaydayPlayerState*				cPlayerState;
	FTimerHandle							cParticleTimer;
	UParticleSystemComponent*				cParticleSystem;
	void InteractionDetected();
	UFUNCTION()
		void OnOverlapBegin( UPrimitiveComponent* OverlappedComp , AActor* OtherActor , UPrimitiveComponent* OtherComp , int32 OtherBodyIndex , bool bFromSweep , const FHitResult& SweepResult );
	UFUNCTION()
		void OnOverlapEnd( UPrimitiveComponent* OverlappedComp , AActor* OtherActor , UPrimitiveComponent* OtherComp , int32 OtherBodyIndex );
	void StopEmitter();
};
