// Fill out your copyright notice in the Description page of Project Settings.


#include "KeycardDoor.h"
#include "Payday/PaydayCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Payday/Core/PlayerStates/PaydayPlayerState.h"

// Sets default values
AKeycardDoor::AKeycardDoor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	bInteractionDestoysKeycards = true;
	fOpeningAngle = -90.0f;
	fOpeningSpeed = 1.0f;

	cStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>( "Static Mesh" );
	cStaticMesh->SetCollisionProfileName( "BlockAll" );

	cBoxCollider = CreateDefaultSubobject<UBoxComponent>( "Box Collider" );
	cBoxCollider->SetCollisionProfileName( "Trigger" );

	RootComponent = cStaticMesh;
	cBoxCollider->SetupAttachment( RootComponent );

	cReplicatedComp = CreateDefaultSubobject<UReplicatedInteractable>( "ReplicatedInteractable" );
	cCharacter = nullptr;
	bCharacterDetected = false;

	cBoxCollider->OnComponentBeginOverlap.AddDynamic( this , &AKeycardDoor::OnOverlapBegin );
	cBoxCollider->OnComponentEndOverlap.AddDynamic( this , &AKeycardDoor::OnOverlapEnd );	
	
}

// Called when the game starts or when spawned
void AKeycardDoor::BeginPlay()
{
	Super::BeginPlay();
	if( fOpeningAngle < 0.0f )
	{
		bInvertRotation = true;
	}
}

// Called every frame
void AKeycardDoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if( bCharacterDetected )
	{
		if( cCharacter->GetPlayerInteracting() == true )
		{
			InteractionDetected();
		}
	}
	if( bRotate )
	{
		RotateMe();
	}

}

void AKeycardDoor::OnOverlapBegin( UPrimitiveComponent* OverlappedComp , AActor* OtherActor , UPrimitiveComponent* OtherComp , int32 OtherBodyIndex , bool bFromSweep , const FHitResult& SweepResult )
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

void AKeycardDoor::OnOverlapEnd( UPrimitiveComponent* OverlappedComp , AActor* OtherActor , UPrimitiveComponent* OtherComp , int32 OtherBodyIndex )
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

void AKeycardDoor::InteractionDetected()
{
	if( bCharacterDetected && cReplicatedComp )
	{
		if( cPlayerState )
		{
			if( cPlayerState->GetKeycard().Contains( eKeycardColour ) )
			{
				GEngine->AddOnScreenDebugMessage( -1 , 3.0f , FColor::Green , TEXT( "Match Found!!!!!" ) );
				GEngine->AddOnScreenDebugMessage( -1 , 3.0f , FColor::Green , TEXT( "Keycard Used" ) );
				bRotate = true;
				//cReplicatedComp->InteractItem();
				bCharacterDetected = false;

				// Play open door sfx
				if( OpenedSound )
				{
					UGameplayStatics::PlaySoundAtLocation( GetWorld() , OpenedSound , GetActorLocation() );
				}
				if( bInteractionDestoysKeycards )
				{
					cPlayerState->RemoveKeycard( eKeycardColour );
				}
			}
		}

		else
		{
			GEngine->AddOnScreenDebugMessage( -1 , 3.0f , FColor::Green , TEXT( "Access Denied! Keycard Required!" ) );
			bCharacterDetected = false;
		}
	}
}

void AKeycardDoor::RotateMe()
{

	if( bInvertRotation )
	{
		fYawValue--;
		if( fYawValue <= fOpeningAngle )
		{
			bRotate = false;
		}
	}
	else
	{
		fYawValue++;
		if( fYawValue >= fOpeningAngle )
		{
			bRotate = false;
		}
	}
	AddActorLocalRotation( FRotator( 0.0f , fOpeningSpeed , 0.0f ) );
}