// Fill out your copyright notice in the Description page of Project Settings.


#include "SurveillanceCamera.h"
#include "Components/StaticMeshComponent.h"
#include "Engine.h"
#include "AI/WaveAISpawner.h"
#include "Core/GameStates/PaydayGameState.h"
#include "Payday/PaydayCharacter.h"
#include "Payday/PaydayGameMode.h"
#include "Payday/PaydayHUD.h"
#include "Payday/Gameplay/ActorComponents/DetectionComponent.h"


// Sets default values
ASurveillanceCamera::ASurveillanceCamera()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Pc_SurveillanceCameraMesh = CreateDefaultSubobject<UStaticMeshComponent>( TEXT( "Surveillance Camera Mesh" ) );
	RootComponent = Pc_SurveillanceCameraMesh;
	Pc_FieldOfVisionMesh = CreateDefaultSubobject<UStaticMeshComponent>( TEXT( "Field Of Vision Mesh" ) );
	Pc_FieldOfVisionMesh->SetupAttachment( RootComponent );
	Pc_FieldOfVisionMesh->SetCollisionProfileName( "Trigger" );

	Pc_FieldOfVisionMesh->OnComponentBeginOverlap.AddDynamic( this, &ASurveillanceCamera::OnOverlapBegin );
	Pc_FieldOfVisionMesh->OnComponentEndOverlap.AddDynamic( this, &ASurveillanceCamera::OnOverlapEnd );

	Pf_SurveillancePollingRate = 0.5f;
	bCameraIsActive = true;
	bPlayerDetected = false;
	cRayTarget = nullptr;
	cRayHitResult = new FHitResult();
	v3RayStart = FVector( 0.0f, 0.0f, 0.0f );
	v3RayEnd = FVector( 0.0f, 0.0f, 0.0f );
	bTimerPause = false;
	cPlayerController = nullptr;
}

// Called when the game starts or when spawned
void ASurveillanceCamera::BeginPlay()
{
	Super::BeginPlay();
	GetWorldTimerManager().SetTimer( cSurveillanceTimer, this, &ASurveillanceCamera::RepeatingRayCast, Pf_SurveillancePollingRate, true, Pf_SurveillancePollingRate );
	GetWorldTimerManager().PauseTimer( cSurveillanceTimer );
	cGameMode = Cast<APaydayGameMode>( GetWorld()->GetAuthGameMode() );

	// If there is no player in the level, it will return nullptr, that's why it breaks the game
	// Don't get HUD here
	// PcGameHUD = Cast<APaydayHUD>( GetWorld()->GetFirstPlayerController()->GetHUD() );
	
	bTimerPause = true;

	//// Get all AI character spawning points from level
	//UGameplayStatics::GetAllActorsOfClass( GetWorld(), Pc_WaveAISpawnerClass, aAISpawningPointArray );

}

// Called every frame
// Mostly handling the timer logic here
void ASurveillanceCamera::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
	if( bCameraIsActive )
	{
		if( bPlayerDetected && aPlayerArray.Num() == 0 )
		{
			bPlayerDetected = false;
			GetWorldTimerManager().PauseTimer( cSurveillanceTimer );
			bTimerPause = true;
		}
		if( bPlayerDetected && aPlayerArray.Num() > 0 && bTimerPause )
		{
			GetWorldTimerManager().UnPauseTimer( cSurveillanceTimer );
			bTimerPause = false;
		}
	}
}

// Setting the camera to active or inactive, can be used by a camera controller to disable all cameras active at once.
void ASurveillanceCamera::SetSurveillanceCameraActiveState( bool bState )
{
	bCameraIsActive = bState;
}

// Old code used for testing. fires a single raycast at the target player's location then draws a debug line using ECC_Camera collision channel.
void ASurveillanceCamera::RayCast( FVector PlayerLocation )
{
	v3RayStart = GetActorLocation();
	v3RayEnd = PlayerLocation;

	if (GetWorld()->LineTraceSingleByChannel(*cRayHitResult, v3RayStart, v3RayEnd, ECC_Camera))
	{
		cRayTarget = cRayHitResult->GetActor();
		if (cRayHitResult->GetActor()->GetName().Contains("Character"))
		{
			DrawDebugLine(GetWorld(), v3RayStart, v3RayEnd, FColor(255, 0, 0), true);
		}
	}
}

// Repeating because its called by a timer, for each player in the array fire a raycast at their position, if they are hit draw a line & do something,
void ASurveillanceCamera::RepeatingRayCast()
{
	if( aPlayerArray.Num() > 0 && bCameraIsActive )
	{
		for( int i = 0; i < aPlayerArray.Num(); i++ )
		{
			v3RayStart = GetActorLocation();
			v3RayEnd = aPlayerArray[ i ]->GetActorLocation();

			if( GetWorld()->LineTraceSingleByChannel( *cRayHitResult, v3RayStart, v3RayEnd, ECC_Camera ) )
			{
				cRayTarget = cRayHitResult->GetActor();
				if( !cRayTarget )
				{
					return;
				}
				if( cRayHitResult->GetActor()->GetName().Contains( "Character" ) )
				{
					// Player is Detected!!
					// if player does something bad do something to tell something about the player.
					DrawDebugLine( GetWorld(), v3RayStart, v3RayEnd, FColor( 255, 0, 0 ), false, 1.0f );
					// Only the host can access the game mode function, otherwise it will return null pointer exception when the other player is detected
					
					//Deprecated
					//if (fTime >= fMaxTime)
					//{
					//	Server_PlayerIsDetected();
					//}
					//float temp = fTime / fMaxTime;			
					//// PcGameHUD->UpdateDetection(temp);
					//APaydayGameState* gameState = Cast<APaydayGameState>( GetWorld()->GetGameState() );
					//gameState->SetDetectionValue( temp );
					//GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, FString::SanitizeFloat(temp));
				}
			}
		}
	}
}
// When a character enters the vision cone of the camera cast their instigator to APaydayCharacter, then add it to an array of players, and setting bPlayerDetected true which is used in Tick to handle pausing of the timer
void ASurveillanceCamera::OnOverlapBegin( UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult )
{
	if( OtherActor != nullptr && OtherActor != this && OtherComp != nullptr )
	{
		if( OtherActor->GetName().Contains( "Character" ) )
		{
			cPlayerController = Cast<APaydayCharacter>( OtherActor->GetInstigator() );
			if( cPlayerController )
			{
				ASurveillanceCamera::AddToPlayerArray( cPlayerController );
				bPlayerDetected = true;
				UpdatePlayerDetection(cPlayerController, true);
			}
		}
	}
}

// If a player leaves the vision cone. remove that player from the player array.
void ASurveillanceCamera::OnOverlapEnd( UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex )
{
	if( OtherActor != nullptr && OtherActor != this && OtherComp != nullptr )
	{
		if( aPlayerArray.Num() > 0 )
		{
			if( OtherActor->GetName().Contains( "Character" ) )
			{
				cPlayerController = Cast<APaydayCharacter>( OtherActor->GetInstigator() );
				if( cPlayerController )
				{
					ASurveillanceCamera::RemoveFromPlayerArray( cPlayerController );
					UpdatePlayerDetection(cPlayerController, false);
				}
			}
		}
	}
}

// Add Player if they don't exist in the array already
void  ASurveillanceCamera::AddToPlayerArray( APaydayCharacter* Player )
{
	if( !aPlayerArray.Contains( Player ) )
	{
		aPlayerArray.Add( Player );
	}
}

// Remove the Player if they Do exist in the array.
void ASurveillanceCamera::RemoveFromPlayerArray( APaydayCharacter* Player )
{
	if( aPlayerArray.Contains( Player ) )
	{
		aPlayerArray.Remove( Player );
	}
}

void ASurveillanceCamera::UpdatePlayerDetection(APaydayCharacter* player, bool bIsDetected)
{
	TArray<UDetectionComponent*> cDetectionCompArray;
	player->GetComponents<UDetectionComponent>(cDetectionCompArray);
	if (cDetectionCompArray.Num() >= 1)
	{
		UDetectionComponent* cDetectionComp = cDetectionCompArray[0];
		if (cDetectionComp)
		{
			cDetectionComp->UpdateDetectionCamera(bIsDetected);
		}
	}
}

//Deprecated moved the logic to detection component

//void ASurveillanceCamera::Server_PlayerIsDetected_Implementation()
//{
//	if( cGameMode )
//	{
//		cGameMode->DetectPlayer();
//
//		// Request all AI character spawning points to spawn AI character
//		for( AActor* cActor : aAISpawningPointArray )
//		{
//			AWaveAISpawner* cAISpawningPoint = Cast<AWaveAISpawner>( cActor );
//			if( cAISpawningPoint )
//			{
//				cAISpawningPoint->Server_StartSpawningAICharacter();
//			}
//		}
//	}
//}
