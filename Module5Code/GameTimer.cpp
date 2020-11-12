// Fill out your copyright notice in the Description page of Project Settings.


#include "GameTimer.h"
#include "Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Payday/Core/PlayerControllers/PaydayPlayerController.h"
#include "Payday/PaydayHUD.h"
#include "Payday/Core/GameStates/PaydayGameState.h"

// Sets default values
AGameTimer::AGameTimer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	iSeconds = 0;
	bPause = false;
	bResume = false;
}

// Called when the game starts or when spawned
void AGameTimer::BeginPlay()
{
	Super::BeginPlay();
	GetWorldTimerManager().SetTimer( cGameTimerHandle , this , &AGameTimer::UpdateTimer , 1.0f , true , 0.0f );
	if( GetWorld() )
	{
		cPlayerController = Cast<APaydayPlayerController>( GetWorld()->GetFirstPlayerController() );
	}
}

// Tick is used to debug the timer.
// Called every frame
void AGameTimer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if( bPause )
	{
		GEngine->AddOnScreenDebugMessage( -1 , 2.0f , FColor::Green , FString::Printf( TEXT( "GameTimer Paused." ) ) );
		PauseTimer();
		bPause = false;
	}
	if( bResume )
	{
		GEngine->AddOnScreenDebugMessage( -1 , 2.0f , FColor::Green , FString::Printf( TEXT( "GameTimer Resumed." ) ) );
		ResumeTimer();
		bResume = false;
	}
}

void AGameTimer::UpdateTimer()
{
	// To prevent int overflow if seconds is greater than one hour (3600 or 60 * 60) then reset seconds.
	// it would take years to overflow though :)
	if( iSeconds >= 60 * 60 )
	{
		iSeconds = 0;
	}
	iSeconds++;	
	if( cPlayerController )
	{
		// Get HUD from PlayerController
		cHUD = Cast<APaydayHUD>( cPlayerController->GetHUD() );
		if( cHUD )
		{
			cHUD->UpdateTimer(iSeconds );
		}
	}
}

void AGameTimer::PauseTimer()
{
	if( false == GetWorldTimerManager().IsTimerPaused( cGameTimerHandle ) )
	{
		GetWorldTimerManager().PauseTimer( cGameTimerHandle );
	}
}

void AGameTimer::ResumeTimer()
{
	if( true == GetWorldTimerManager().IsTimerPaused( cGameTimerHandle ) )
	{
		GetWorldTimerManager().UnPauseTimer( cGameTimerHandle );
	}
}