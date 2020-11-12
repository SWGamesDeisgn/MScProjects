// Fill out your copyright notice in the Description page of Project Settings.


#include "Keycard.h"
#include "Payday/Gameplay/ActorComponents/ReplicatedInteractable.h"
#include "Payday/PaydayCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Payday/Core/PlayerStates/PaydayPlayerState.h"

// Sets default values
AKeycard::AKeycard()
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

	cBoxCollider->OnComponentBeginOverlap.AddDynamic( this , &AKeycard::OnOverlapBegin );
	cBoxCollider->OnComponentEndOverlap.AddDynamic( this , &AKeycard::OnOverlapEnd );
}

// Called when the game starts or when spawned
void AKeycard::BeginPlay()
{
	Super::BeginPlay();
	if( IsHidden() )
	{
		cBoxCollider->SetCollisionEnabled( ECollisionEnabled::NoCollision );
	}
}

// Called every frame
void AKeycard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if( bCharacterDetected)
	{
		if( cCharacter->GetPlayerInteracting() == true )
		{
			InteractionDetected();
		}
	}
}

void AKeycard::OnOverlapBegin( UPrimitiveComponent* OverlappedComp , AActor* OtherActor , UPrimitiveComponent* OtherComp , int32 OtherBodyIndex , bool bFromSweep , const FHitResult& SweepResult )
{
	if( OtherActor != nullptr && OtherActor != this && OtherComp != nullptr )
	{
		if( OtherActor->GetName().Contains( "Character" ) )
		{
			cCharacter = Cast<APaydayCharacter>( OtherActor->GetInstigator() );
			if( cCharacter )
			{
				cPlayerState = Cast<APaydayPlayerState>( GetWorld()->GetFirstPlayerController()->PlayerState );
				SetOwner( cCharacter );
				bCharacterDetected = true;
			}
		}
	}

}

void AKeycard::OnOverlapEnd( UPrimitiveComponent* OverlappedComp , AActor* OtherActor , UPrimitiveComponent* OtherComp , int32 OtherBodyIndex )
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

void AKeycard::InteractionDetected()
{
	if( bCharacterDetected && cReplicatedComp )
	{
		if( cPlayerState )
		{
			if( !cPlayerState->GetKeycard().Contains( eKeycardColour ) )
			{
				cPlayerState->SetKeycard( eKeycardColour );
				GEngine->AddOnScreenDebugMessage( -1 , 3.0f , FColor::Green , TEXT( "Keycard Get" ) );
				cReplicatedComp->InteractItem();
				bCharacterDetected = false;

				// Play interact sound effect
				if( cInteractSound )
				{
					UGameplayStatics::PlaySoundAtLocation( GetWorld() , cInteractSound , GetActorLocation() );
				}
			}
		}
	}
}