// Fill out your copyright notice in the Description page of Project Settings.


#include "Drill.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "BankVault.h"
#include "Components/WidgetComponent.h"
#include "Payday/UI/Components/DrillProgressPanel.h"
#include "Payday/PaydayHUD.h"
#include "Payday/Core/PlayerControllers/PaydayPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values
ADrill::ADrill()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	cBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	RootComponent = cBoxComponent;

	cStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	cStaticMeshComponent->SetupAttachment(RootComponent);

	// Create audio component
	cAudioComponent = CreateDefaultSubobject<UAudioComponent>( TEXT( "AudioComponent" ) );
	cAudioComponent->SetupAttachment( RootComponent );
	
	cEmitterLocation = CreateDefaultSubobject<UBoxComponent>( TEXT( "VFX Emitter Locator" ) );
	cEmitterLocation->SetupAttachment( RootComponent );
	cEmitterLocation->SetBoxExtent( FVector( 4.0f , 4.0f , 4.0f ) );
	cEmitterLocation->SetHiddenInGame( true );
	cEmitterLocation->SetCollisionEnabled( ECollisionEnabled::NoCollision );

	bIsValutDestroyed = false;

	bIsDrillDeactivated = false;
}

// Called when the game starts or when spawned
void ADrill::BeginPlay()
{
	Super::BeginPlay();
	// Stop drilling sound
	cAudioComponent->SetFloatParameter( "IsDrilling", 0.0f );

	// Find the widget component when the drill is init
	// And assume the first component is the HUD widget
	TArray<UWidgetComponent*> arrWidgetComponents;
	GetComponents( arrWidgetComponents );
	if( arrWidgetComponents.Num() > 0 )
	{
		cWidgetComponent = arrWidgetComponents[ 0 ];
		if( cWidgetComponent )
		{
			cPanelWidget = Cast<UDrillProgressPanel>( cWidgetComponent->GetUserWidgetObject() );
		}
	}

	if (IsHidden())
	{
		cBoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		cStaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

// Called every frame
void ADrill::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Hotfix for updating the drill HUD progress bar
	if( cPanelWidget )
	{
		float fProgress = GetWorldTimerManager().GetTimerElapsed( DestroyDrillTimerHandler ) / fMaxDrillTime;
		cPanelWidget->SetDrillProgress( fProgress );
	}
}

void ADrill::SetBankVault(class ABankVault* cVault)
{
	// Call the server side to activate the drill
	// Server_ActivateDrill( cVault );

	// set the pointer
	cBankVault = cVault;



	// Activate the timer for the drill
	GetWorld()->GetTimerManager().SetTimer( DestroyDrillTimerHandler, this, &ADrill::OnDrillDestroyed, fMaxDrillTime );

	if (GetWorld())
	{
		APaydayGameState* cGameSate = Cast<APaydayGameState>(GetWorld()->GetGameState());
		if (cGameSate)
		{
			cGameSate->SetDrillActivationStatus(true);
		}
	}
	
	// Play drilling sound
	PlayDrillingSFX();
	PlayDrillingVFX();

	// Update the drill HUD
	if( cPanelWidget )
	{
		cPanelWidget->SetProgressText( "Running" );
	}
}

bool ADrill::Server_ActivateDrill_Validate(ABankVault* cVault)
{
	return !bIsValutDestroyed;
}

void ADrill::Server_ActivateDrill_Implementation( ABankVault* cVault )
{
	// set the pointer
	cBankVault = cVault;

	// Activate the timer for the drill
	GetWorld()->GetTimerManager().SetTimer( DestroyDrillTimerHandler, this, &ADrill::OnDrillDestroyed, fMaxDrillTime );
	
}

void ADrill::OnDrillDestroyed()
{
	// Notify the vault has been destroyed by player
	if( cBankVault )
	{
		cBankVault->DestroyVault();
		bIsValutDestroyed = true;

		APlayerController* cController = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
		if (cController)
		{
			APaydayHUD* cHUD = Cast<APaydayHUD>(cController->GetHUD());
			if (cHUD)
			{
				cHUD->SetObjectiveMessage(EPaydayObjectiveState::GRAB_THE_MONEY);
			}
		}
		if (GetWorld())
		{
			APaydayGameState* cGameSate = Cast<APaydayGameState>(GetWorld()->GetGameState());
			if (cGameSate)
			{
				cGameSate->SetDrillActivationStatus(false);

				// Update the game state objective
				cGameSate->SetCurrentObjective( EPaydayObjectiveState::GRAB_THE_MONEY );
			}
		}
		// Stop drilling sound
		StopDrillingSFX();
		// Stop Drill VFX.
		StopDrillingVFX();

		GetWorld()->DestroyActor(this);
	}
}

void ADrill::DeactivateDrill()
{
	if (!bIsDrillDeactivated)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, TEXT("Drill Deactivated"));
		GetWorld()->GetTimerManager().PauseTimer(DestroyDrillTimerHandler);
		bIsDrillDeactivated = true;

		if (GetWorld())
		{
			APaydayGameState* cGameSate = Cast<APaydayGameState>(GetWorld()->GetGameState());
			if (cGameSate)
			{
				cGameSate->SetDrillActivationStatus(false);
			}
		}

		APlayerController* cController = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
		if (cController)
		{
			APaydayHUD* cHUD = Cast<APaydayHUD>(cController->GetHUD());
			if (cHUD)
			{
				cHUD->SetObjectiveMessage(EPaydayObjectiveState::REACTIVATE_THE_DRILL);
			}
		}

		// Stop drilling sound
		StopDrillingSFX();
		// Stop Drill VFX.
		StopDrillingVFX();

		// Update the drill HUD
		if( cPanelWidget )
		{
			cPanelWidget->SetProgressText( "Stopped" );
		}
	}

}

void ADrill::StartActivatingDrill()
{
	if (bIsDrillDeactivated)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, TEXT("Activating Drill"));
		GetWorld()->GetTimerManager().SetTimer(ReactivateDrillTimer, this, &ADrill::ActivateDrill, fMaxDrillReactivationTime);
		
		// Start drilling sound
		PlayDrillingSFX();

		// Update the drill HUD
		if( cPanelWidget )
		{
			cPanelWidget->SetProgressText( "Activating" );
		}
	}
}

void ADrill::StopActivatingDrill()
{
	if (bIsDrillDeactivated)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, TEXT("Stopped Activating Drill"));
		GetWorld()->GetTimerManager().ClearTimer(ReactivateDrillTimer);
		
		// Stop drilling sound
		StopDrillingSFX();

		// Update the drill HUD
		if( cPanelWidget )
		{
			cPanelWidget->SetProgressText( "Stopped" );
		}
	}
}

void ADrill::PlayDrillingSFX()
{
	cAudioComponent->SetFloatParameter( "IsDrilling", 1.0f );
}

void ADrill::StopDrillingSFX()
{
	cAudioComponent->SetFloatParameter( "IsDrilling", 0.0f );
}

void ADrill::ActivateDrill()
{
	if (bIsDrillDeactivated)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, TEXT("Drill Activated"));
		GetWorld()->GetTimerManager().UnPauseTimer(DestroyDrillTimerHandler);
		bIsDrillDeactivated = false;
		
		if (GetWorld())
		{
			APaydayGameState* cGameSate = Cast<APaydayGameState>(GetWorld()->GetGameState());
			if (cGameSate)
			{
				cGameSate->SetDrillActivationStatus(true);
			}
		}

		APlayerController* cController = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
		if (cController)
		{
			APaydayHUD* cHUD = Cast<APaydayHUD>(cController->GetHUD());
			if (cHUD)
			{
				cHUD->SetObjectiveMessage(EPaydayObjectiveState::OPEN_THE_VAULT);
			}
		}

		// Start drilling sound
		PlayDrillingSFX();
		// Start Drill VFX.
		PlayDrillingVFX();
		// Update the drill HUD
		if( cPanelWidget )
		{
			cPanelWidget->SetProgressText( "Running" );
		}
	}

}

void ADrill::PlayDrillingVFX()
{
	if( nullptr != cSparksEmitter )
	{
		if( nullptr == cParticleSystem )
		{
			cParticleSystem = UGameplayStatics::SpawnEmitterAttached( cSparksEmitter , cEmitterLocation );
		}

		else
		{
			cParticleSystem->bSuppressSpawning = false;
		}
	}

}

void ADrill::StopDrillingVFX()
{

	if( nullptr != cParticleSystem && nullptr != cSparksEmitter )
	{
		cParticleSystem->bSuppressSpawning = true;
	}
}