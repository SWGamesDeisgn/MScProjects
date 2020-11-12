// Fill out your copyright notice in the Description page of Project Settings.


#include "MoneyPickup.h"
#include "Payday/Gameplay/ActorComponents/ReplicatedInteractable.h"
#include "Payday/PaydayCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Payday/PaydayHUD.h"
#include "Payday/Core/PlayerControllers/PaydayPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Payday/Core/PlayerStates/PaydayPlayerState.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleEmitter.h"

// Sets default values
AMoneyPickup::AMoneyPickup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	cStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>( "Static Mesh" );
	cStaticMesh->SetCollisionProfileName( "NoCollision" );
	
	cBoxCollider = CreateDefaultSubobject<UBoxComponent>( "Box Collider" );
	cBoxCollider->SetCollisionProfileName( "Trigger" );

	RootComponent = cBoxCollider;
	cStaticMesh->SetupAttachment( RootComponent );

	cReplicatedComp = CreateDefaultSubobject<UReplicatedInteractable>( "ReplicatedInteractable" );
	cCharacter = nullptr;
	bCharacterDetected = false;

	cBoxCollider->OnComponentBeginOverlap.AddDynamic( this , &AMoneyPickup::OnOverlapBegin );
	cBoxCollider->OnComponentEndOverlap.AddDynamic( this , &AMoneyPickup::OnOverlapEnd );
}

// Called when the game starts or when spawned
void AMoneyPickup::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AMoneyPickup::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
	if( bCharacterDetected )
	{
		if( cCharacter->GetPlayerInteracting() == true )
		{
			InteractionDetected();
		}
	}
}

void AMoneyPickup::OnOverlapBegin( UPrimitiveComponent* OverlappedComp , AActor* OtherActor , UPrimitiveComponent* OtherComp , int32 OtherBodyIndex , bool bFromSweep , const FHitResult& SweepResult )
{
	if( OtherActor != nullptr && OtherActor != this && OtherComp != nullptr )
	{
		if( OtherActor->GetName().Contains( "Character" ) )
		{
			cCharacter = Cast<APaydayCharacter>( OtherActor->GetInstigator() );
			if( cCharacter )
			{
				SetOwner( cCharacter );
				bCharacterDetected = true;
			}
		}
	}

}

void AMoneyPickup::OnOverlapEnd( UPrimitiveComponent* OverlappedComp , AActor* OtherActor , UPrimitiveComponent* OtherComp , int32 OtherBodyIndex )
{
	if( OtherActor != nullptr && OtherActor != this && OtherComp != nullptr )
	{
		if( OtherActor->GetName().Contains( "Character" ) )
		{
			cCharacter = Cast<APaydayCharacter>( OtherActor->GetInstigator() );
			if( cCharacter )
			{
				bCharacterDetected = false;
			}
		}
	}
}

// Check to see if bCharacterDetected is true, then get the PlayerState.
// Only if the PlayerState has zero money on them can they pickup another MoneyPickup.
// Sets bCharacterDetected to false, add the value of fMoney to the PlayerState of the interacting player.
// Also set this object to hidden and update the mission objective.
void AMoneyPickup::InteractionDetected()
{
	if( bCharacterDetected && cReplicatedComp )
	{
		// Update the player state
		cPlayerState = Cast<APaydayPlayerState>( GetWorld()->GetFirstPlayerController()->PlayerState );
		if( cPlayerState->GetCurrentMoney() == 0 )
		{
			GEngine->AddOnScreenDebugMessage( -1 , 3.0f , FColor::Green , TEXT( "Money Get +1" ) );
			//cCharacter->SetMoney( fMoney );
			cPlayerState->SetCurrentMoney( fMoney );
			cReplicatedComp->InteractItem();
			bCharacterDetected = false;
			// Play the particle effect and then call a timer to suppress spawning of the particles.
			if( cParticles )
			{
				cParticleSystem = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), cParticles , FTransform( FRotator( 0.0f , 0.0f , 0.0f ) , GetActorLocation() , { 1.0f,1.0f,1.0f } ) );
				GetWorldTimerManager().SetTimer( cParticleTimer , this , &AMoneyPickup::StopEmitter , 1.0f , true , 1.0f );
			}
			APlayerController* cController = Cast<APlayerController>( GetWorld()->GetFirstPlayerController() );
			if( cController )
			{
				APaydayHUD* cHUD = Cast<APaydayHUD>( cController->GetHUD() );
				if( cHUD )
				{
					cHUD->SetObjectiveMessage(EPaydayObjectiveState::TAKE_THE_MONEY_TO_THE_VAN );
				}
			}

			// Play interact sound effect
			if( cInteractSound )
			{
				UGameplayStatics::PlaySoundAtLocation( GetWorld() , cInteractSound , GetActorLocation() );
			}

			// Update the game state objective state
			APaydayGameState* cPaydayGameState = GetWorld()->GetGameState<APaydayGameState>();
			if( cPaydayGameState )
			{
				cPaydayGameState->SetCurrentObjective( EPaydayObjectiveState::TAKE_THE_MONEY_TO_THE_VAN );
			}
		}
	}
}

void AMoneyPickup::StopEmitter()
{
	if( cParticles )
	{
		cParticleSystem->bSuppressSpawning = true;
		GetWorldTimerManager().ClearTimer( cParticleTimer );
	}
}