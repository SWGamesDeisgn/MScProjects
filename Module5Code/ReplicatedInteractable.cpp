// Fill out your copyright notice in the Description page of Project Settings.

#include "ReplicatedInteractable.h"

#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Payday/PaydayCharacter.h"

// Sets default values for this component's properties
UReplicatedInteractable::UReplicatedInteractable()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	
	pOwner = nullptr;
	pCharacter = nullptr;
}

// Called when the game starts
void UReplicatedInteractable::BeginPlay()
{
	Super::BeginPlay();

	// Get owner and find the target trigger
	pOwner = GetOwner();
	if( pOwner )
	{
		/* Deprecated
		// Get component by TSubclassOf<UBoxComponent>
		cBoxTrigger = Cast<UBoxComponent>( pOwner->GetComponentByClass( UBoxComponent::StaticClass() ) );
		if( cBoxTrigger )
		{
			// Bind the overlap event to the BeginOverlap function to handle the overlap event
			cBoxTrigger->OnComponentBeginOverlap.AddDynamic( this, &UReplicatedInteractable::BeginOverlap );
			cBoxTrigger->OnComponentEndOverlap.AddDynamic( this, &UReplicatedInteractable::EndOverlap );
		}
		*/
		// Set self to be replicated
		SetIsReplicated( true );

		// Set owner to be replicated
		pOwner->SetReplicates( true );
	}	
}

void UReplicatedInteractable::InteractItem_Implementation()
{
	// Notify all clients and host to update its status
	NotifyStatus();
}

void UReplicatedInteractable::NotifyStatus_Implementation()
{
	// Hide and disable collision for this actor
	GetOwner()->SetActorHiddenInGame( true );
	GetOwner()->SetActorEnableCollision( false );
	GetOwner()->SetActorTickEnabled( false );
}

/* Deprecated
void UReplicatedInteractable::BeginOverlap( UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult )
{
	GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Red, TEXT( "I am colliding with something?!" ) );

	// If the character is interacting with pickup, set the current pickup to the character
	pCharacter = Cast<APaydayCharacter>( OtherActor );
	if( pCharacter )
	{
		// Set this actor component to the player character
		pOwner->SetOwner( pCharacter );
		//pCharacter->SetTargetInteractive( this );
	}
}


void UReplicatedInteractable::EndOverlap( UPrimitiveComponent* OverlappedComp , AActor* OtherActor , UPrimitiveComponent* OtherComp , int32 OtherBodyIndex )
{
	GEngine->AddOnScreenDebugMessage( -1 , 3.0f , FColor::Red , TEXT( "I am no longer colliding with something!" ) );
	// If the character is interacting with pickup, set the current pickup to the character
	pCharacter = Cast<APaydayCharacter>( OtherActor );
	if( pCharacter )
	{
		// Remove the actor from the player
		//pCharacter->RemoveTargetInteractive();
	}
}
*/