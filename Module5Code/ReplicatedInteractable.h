// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/BoxComponent.h"
#include "ReplicatedInteractable.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PAYDAY_API UReplicatedInteractable : public UActorComponent
{
	GENERATED_BODY()

public:
	
	// Sets default values for this component's properties
	UReplicatedInteractable();

	/*
	 * Replicated Component Settings
	 */
	UBoxComponent* cBoxTrigger;

protected:
	
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	
	// A multicast function of handling interaction from player
	// NetMulticast - client and server sides will receive this call
	// Client and server sides need to update the money pickup such as the visibility and collision.
	// As a result, we need to handle this on both side

	/**
	 * \brief Interact the item, only run on the server-side
	 */
	UFUNCTION( Server, Reliable )
	void InteractItem();
	void InteractItem_Implementation();
	
	/**
	 * \brief Notify all clients and the host to update this actor component has changed the status
	 */
	UFUNCTION( NetMulticast, Reliable )
	void NotifyStatus();
	void NotifyStatus_Implementation();


private:
	class AActor* pOwner;
	class APaydayCharacter* pCharacter;
	/**
	 * \brief Handle the overlap event for this interactable
	* \param OverlappedComponent
	 * \param OtherActor Another actor that overlapped to this actor
	 * \param OtherComp
	 * \param OtherBodyIndex
	 * \param bFromSweep
	 * \param SweepResult
	 */

	/* Deprecated
	UFUNCTION()
		void BeginOverlap(
			UPrimitiveComponent* OverlappedComponent ,
			AActor* OtherActor ,
			UPrimitiveComponent* OtherComp ,
			int32 OtherBodyIndex ,
			bool bFromSweep ,
			const FHitResult &SweepResult );

	UFUNCTION()
		void EndOverlap( UPrimitiveComponent* OverlappedComp ,
			AActor* OtherActor ,
			UPrimitiveComponent* OtherComp ,
			int32 OtherBodyIndex );
	*/
};