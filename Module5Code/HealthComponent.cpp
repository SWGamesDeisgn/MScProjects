// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerController.h"
#include "Payday/PaydayCharacter.h"
#include "Payday/PaydayHUD.h"
#include "Payday/Core/PlayerControllers/PaydayPlayerController.h"
#include "Payday/Core/PlayerStates/PaydayPlayerState.h"
#include "GameFramework/Actor.h"
#include "Payday/AI/BaseAICharacter.h"

//Armour functionality should go in here too.

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	Max_Health = 100.0;
	Max_Lives = 1;
	bPlayerDead = false;
	Max_Armour = 50.0f;
	Armour_Regen_Speed = 1.0f;
	Armour_Regen_Delay = 5.0f;
	fArmourRegenTimer = 0.0f;
	bActorHasBeenHit = false;
	bArmourRegenerate = false;
	ReviveDelay = 5.0f;
	fReviveDelay = 0.0f;
	bIsReviving = false;
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	// Setup the replication
	SetIsReplicated( false );
	//Health Variables
	fMaxHealth = Max_Health;
	fCurrentHealth = fMaxHealth;
	Current_Health = fCurrentHealth;
	iMaxLives = Max_Lives;
	iCurrentLives = iMaxLives;
	Current_Lives = iCurrentLives;
	bPlayerDead = false;
	DamagePlayer = false;
	HealPlayer = false;
	FullHealPlayer = false;
	GivePlayerLives = false;
	//Armour Variables
	fCurrentArmourValue = Max_Armour;
	fMaxArmourValue = Max_Armour;
	fArmourRegenRate = Armour_Regen_Speed;
	fArmourRegenDelay = Armour_Regen_Delay;
	Current_Armour = fCurrentArmourValue;
	bArmourRegeneration = Armour_Regenerates;
	// ...
	GEngine->AddOnScreenDebugMessage( -1, 2.0f, FColor::Green, FString::Printf( TEXT( "Begin Play; bPlayerDead: %s, iCurrentLives: %i" ), bPlayerDead ? TEXT( "true" ) : TEXT( "false" ), iCurrentLives ) );
}


// Called every frame
void UHealthComponent::TickComponent( float DeltaTime , ELevelTick TickType , FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime , TickType , ThisTickFunction );
	
	//These statements are only for testing our functionality.
	if( DamagePlayer )
	{
		ReduceHealth( 32.7f );
		DamagePlayer = false;
	}

	if( HealPlayer )
	{
		ReplenishHealth( 25.0f );
		HealPlayer = false;
	}

	if( FullHealPlayer )
	{
		FullHeal();
		FullHealPlayer = false;
	}

	if( ResurrectPlayer )
	{
		RevivePlayer();
		ResurrectPlayer = false;
	}

	if( GivePlayerLives )
	{
		AddPlayerLives( 1 );
		GivePlayerLives = false;
	}
	// Actual Tick functionality
	if( bActorHasBeenHit || bArmourRegenerate && fCurrentHealth >= 1 )
	{
		RegenerateArmour( DeltaTime );
	}
	if( iCurrentLives > 0 && bPlayerDead && bIsReviving )
	{
		fReviveDelay += DeltaTime;
		if( fReviveDelay > ReviveDelay )
		{		
			fReviveDelay -= fReviveDelay;
			RevivePlayer();
		}
	}
}

//Remove fReduceAmount from our fCurrentHealth, activates once, if fReduceAmount > fCurrentHealth then our fCurrentHealth = 0.
void UHealthComponent::ReduceHealth( float fReduceAmount )
{
	// Request the server to reduce health
	Server_ReduceHealth( fReduceAmount );
}

void UHealthComponent::Server_ReduceHealth_Implementation( float fReduceAmount )
{
	if( !bPlayerDead && !bIsReviving )
	{
		// We cast the variables here because in editor the world doesn't exist at begin play.
		// Then we add fReduceAmount to the PlayerState to use later in the ShowResults UI Panel.
		APaydayCharacter* cPlayerCharacter = Cast<APaydayCharacter>( GetOwner()->GetInstigator() );
		APaydayPlayerState* cPlayerState = Cast<APaydayPlayerState>( GetWorld()->GetFirstPlayerController()->PlayerState );
		if( cPlayerState && cPlayerCharacter )
		{
			if( fReduceAmount > fCurrentHealth )
			{
				cPlayerState->RecordPlayerDamageTaken( fCurrentHealth );
			}
			else
			{
				cPlayerState->RecordPlayerDamageTaken( fReduceAmount );
			}
		}
	}
	// in some instances it was possible for the value of fReviveDelay to be greater than 0 after the revive was complete.
	// This ensures that fReviveDelay is always 0 unless the player is actually reviving.
	if( !bIsReviving && fReviveDelay != 0 )
	{
		fReviveDelay -= fReviveDelay;
	}
	if( fCurrentArmourValue <= 0 )
	{
		GEngine->AddOnScreenDebugMessage( -1 , 2.0f , FColor::Green ,FString::Printf( TEXT( "%s - Triggered Server Reduce Health; ActorIsDead: %s, ActorLivesRemaining: %i" ) , *GetOwner()->GetOwner()->GetName() , bPlayerDead ? TEXT( "true" ) : TEXT( "false" ) , iCurrentLives ) );
		// Handle the reduce health event on the server-side
		if( !bPlayerDead && iCurrentLives > 0 )
		{
			GEngine->AddOnScreenDebugMessage( -1 , 2.0f , FColor::Green , TEXT( "Something got hurt" ) );
			if( fCurrentHealth > 0 )
			{
				if( fReduceAmount >= fCurrentHealth )
				{
					fCurrentHealth = 0;
					bPlayerDead = true;
					bIsReviving = true;
					Client_UpdateReviveStatus( bIsReviving );
					iCurrentLives--;
					Current_Lives = iCurrentLives;
					GEngine->AddOnScreenDebugMessage( -1 , 2.0f , FColor::Yellow , FString::Printf( TEXT( "%s has died a most tragic death. Oh well... Life goes on. Lives Remaining: %d" ) , *GetOwner()->GetName() , iCurrentLives ) );
					// Notify the client to update the lives count and health
					Client_UpdateLives( iCurrentLives );

					// Notify the client to update the flag of dead
					Client_UpdatePlayerIsDead( bPlayerDead );

				}
				else
				{
					fCurrentHealth -= fReduceAmount;
					GEngine->AddOnScreenDebugMessage( -1 , 2.0f , FColor::Yellow , FString::Printf( TEXT( "%s has taken damage!" ) , *GetOwner()->GetName() ) );
				}

				// Notify the client to update the health
				Client_UpdateHealth( fCurrentHealth );

			}
			Current_Health = fCurrentHealth;
		}
	}
	else
	{
		if( fReduceAmount >= fCurrentArmourValue && !bPlayerDead)
		{
			fCurrentHealth -= (fReduceAmount - fCurrentArmourValue);
			fCurrentArmourValue -= fCurrentArmourValue;
			Current_Health = fCurrentHealth;
			Client_UpdateHealth( fCurrentHealth );
			if( fCurrentHealth <= fReduceAmount - fCurrentArmourValue )
			{
				bPlayerDead = true;
				if( iCurrentLives > 0 )
				{
					bIsReviving = true;
					Client_UpdateReviveStatus( bIsReviving );
				}
				Client_UpdatePlayerIsDead( bPlayerDead );
				iCurrentLives--;
				Current_Lives = iCurrentLives;
			}
		}
		else
		{
			fCurrentArmourValue -= fReduceAmount;
		}
		Current_Armour = fCurrentArmourValue;
		Client_UpdateArmour( fCurrentArmourValue );
	}

	// If we have armour regenerating and are not currently dead i.e health is not zero then
	// we disable regeneration and reset the regeneration timer to zero.
	if( bArmourRegeneration && fCurrentHealth > 0.0f )
	{
		bActorHasBeenHit = true;
		fArmourRegenTimer -= fArmourRegenTimer;
		bArmourRegenerate = false;
	}
}

//Add fReplenishAmount to our fCurrentHealth, if the sum of fCurrentHealth + fReplenishAmount then our fCurrentHealth is capped to fMaxHealth.
void UHealthComponent::ReplenishHealth( float fReplenishAmount )
{
	// Request the server to replenish health
	Server_ReplenishHealth( fReplenishAmount );
}

void UHealthComponent::Server_ReplenishHealth_Implementation( float fReplenishAmount )
{
	if( !bPlayerDead && iCurrentLives > 0 )
	{
		if( fCurrentHealth < fMaxHealth )
		{
			if( ( fCurrentHealth + fReplenishAmount ) > fMaxHealth )
			{
				fCurrentHealth = fMaxHealth;
			}
			else
			{
				fCurrentHealth += fReplenishAmount;
			}
		}
		Current_Health = fCurrentHealth;
		GEngine->AddOnScreenDebugMessage( -1, 2.0f, FColor::Yellow, FString::Printf( TEXT( "%s has been healed!" ), *GetOwner()->GetName() ) );

		// Notify the client to update the health
		Client_UpdateHealth( fCurrentHealth );
	}
}

//Sets the players fCurrentHealth back to fMaxHealth.
void UHealthComponent::FullHeal()
{
	// Request the server to heal fully
	Server_FullHeal();
}

void UHealthComponent::Server_FullHeal_Implementation()
{
	if( !bPlayerDead && iCurrentLives > 0 )
	{
		if( fCurrentHealth > 0 )
		{
			fCurrentHealth = fMaxHealth;
			GEngine->AddOnScreenDebugMessage( -1, 2.0f, FColor::Yellow, FString::Printf( TEXT( "%s has been fully healed!" ), *GetOwner()->GetName() ) );
		}
		Current_Health = fCurrentHealth;

		// Notify the client to update the health
		Client_UpdateHealth( fCurrentHealth );
	}
}

//Revive a dead player to max health, might add a partial resurrect later, which specifies how much health the player comes back to life with.
void UHealthComponent::RevivePlayer()
{
	// Request the server to handle the revive player event
	Server_RevivePlayer();
}

void UHealthComponent::Server_RevivePlayer_Implementation()
{
	if( bPlayerDead && iCurrentLives > 0 )
	{
		bPlayerDead = false;
		fReviveDelay -= fReviveDelay;
		fCurrentHealth = fMaxHealth;
		Current_Health = fCurrentHealth;
		fCurrentArmourValue = fMaxArmourValue;
		Current_Armour = fCurrentArmourValue;
		bIsReviving = false;
		GEngine->AddOnScreenDebugMessage( -1, 2.0f, FColor::Yellow, FString::Printf( TEXT( "%s has been resurrected. Drat..." ), *GetOwner()->GetName() ) );

		// Notify the client to update health and the flag of death
		Client_UpdateHealth( fCurrentHealth );
		Client_UpdatePlayerIsDead( bPlayerDead );
		Client_UpdateArmour( fCurrentArmourValue );
		Client_UpdateReviveStatus( bIsReviving );
	}
}

//Add X amount to iCurrentLives, can not go above IMaxLives.
void UHealthComponent::AddPlayerLives( int iAmountToAdd )
{
	// Request the server to handle the add player lives event
	Server_AddPlayerLives( iAmountToAdd );
}

void UHealthComponent::Server_AddPlayerLives_Implementation( int iAmountToAdd )
{
	if( ( iAmountToAdd + iCurrentLives ) > iMaxLives )
	{
		iCurrentLives = iMaxLives;
		Current_Lives = iCurrentLives;
		GEngine->AddOnScreenDebugMessage( -1, 2.0f, FColor::Yellow, FString::Printf( TEXT( "%s has gained a life" ), *GetOwner()->GetName() ) );
	}
	else
	{
		iCurrentLives += iAmountToAdd;
		Current_Lives = iCurrentLives;
		GEngine->AddOnScreenDebugMessage( -1, 2.0f, FColor::Yellow, FString::Printf( TEXT( "%s has gained a life" ), *GetOwner()->GetName() ) );
	}

	// Notify the client to update the max lives
	Client_UpdateLives( iCurrentLives );
}

void UHealthComponent::Client_UpdateHealth_Implementation( float fNewHealth )
{

	GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Blue, FString::Printf( TEXT( "New Health: %.2f" ), fNewHealth ) );
	fCurrentHealth = fNewHealth;
	Current_Health = fCurrentHealth;
	// Update the player state
	APaydayPlayerState* cPlayerState = Cast<APaydayPlayerState>( GetWorld()->GetFirstPlayerController()->PlayerState );
	if( cPlayerState )
	{
		cPlayerState->SetPlayerHealth( fCurrentHealth / fMaxHealth );
	}

	// Only Update the UI when this health component belongs to the player character
	APaydayCharacter* cPlayer = Cast<APaydayCharacter>( GetOwner() );
	if( cPlayer )
	{
		// Get Controller from this player and cast it to PlayerController
		APaydayPlayerController* cPlayerController = Cast<APaydayPlayerController>( cPlayer->GetController() );
		if( cPlayerController )
		{
			// Get HUD from PlayerController
			APaydayHUD* cHUD = Cast<APaydayHUD>( cPlayerController->GetHUD() );
			if( cHUD )
			{
				cHUD->UpdateHealth( fCurrentHealth / fMaxHealth );
			}
		}
	}
}

void UHealthComponent::Client_UpdateLives_Implementation( int iNewLivesCount )
{
	iCurrentLives = iNewLivesCount;
	APaydayPlayerState* cPlayerState = Cast<APaydayPlayerState>( GetWorld()->GetFirstPlayerController()->PlayerState );
	if( cPlayerState )
	{
		cPlayerState->SetPlayerLives( iCurrentLives );
	}
}

void UHealthComponent::Client_UpdatePlayerIsDead_Implementation( bool bNewPlayerDead )
{
	bPlayerDead = bNewPlayerDead;
	APaydayCharacter* cPlayerCharacter = Cast<APaydayCharacter>( GetOwner()->GetInstigator() );
	ABaseAICharacter* cAICharacter = Cast<ABaseAICharacter>( GetOwner()->GetInstigator() );
	if( bPlayerDead )
	{
		APaydayPlayerState* cPlayerState = Cast<APaydayPlayerState>( GetWorld()->GetFirstPlayerController()->PlayerState );
		if( cPlayerState )
		{
			if( cPlayerCharacter )
			{
				cPlayerState->RecordPlayerDied();
			}		

			if( cAICharacter )
			{
				cPlayerState->AddKillCountToRecord();
			}
			
		}

		if( cPlayerCharacter )
		{
			cPlayerCharacter->PlayerHasDied();
			if( iCurrentLives <= 0 )
			{
				APaydayGameState* cGameState = Cast<APaydayGameState>( GetWorld()->GetGameState() );
				if( cGameState )
				{
					cGameState->SetCurrentGameState( GAME_END );
				}
			}
		}
	}
}

void UHealthComponent::Client_UpdateArmour_Implementation( float fNewArmour )
{
	GEngine->AddOnScreenDebugMessage( -1 , 3.0f , FColor::Blue , FString::Printf( TEXT( "New Armour: %.2f" ) , fNewArmour ) );
	fCurrentArmourValue = fNewArmour;
	Current_Armour = fCurrentArmourValue;
	// Update the player state
	APaydayPlayerState* cPlayerState = Cast<APaydayPlayerState>( GetWorld()->GetFirstPlayerController()->PlayerState );
	if( cPlayerState )
	{
		cPlayerState->SetPlayerArmour( fCurrentArmourValue / fMaxArmourValue );
	}

	// Only Update the UI when this health component belongs to the player character
	APaydayCharacter* cPlayer = Cast<APaydayCharacter>( GetOwner() );
	if( cPlayer )
	{
		// Get Controller from this player and cast it to PlayerController
		APaydayPlayerController* cPlayerController = Cast<APaydayPlayerController>( cPlayer->GetController() );
		if( cPlayerController )
		{
			// Get HUD from PlayerController
			APaydayHUD* cHUD = Cast<APaydayHUD>( cPlayerController->GetHUD() );
			if( cHUD )
			{
				cHUD->UpdateArmour( fCurrentArmourValue / fMaxArmourValue );
			}
		}
	}
}

//Getters for various values.
//Is the player dead yet? returns true if player is dead.
bool UHealthComponent::IsPlayerDead()
{
	return bPlayerDead;
}

float UHealthComponent::GetPlayerHealth()
{
	return fCurrentHealth;
}

float UHealthComponent::GetPlayerMaxHealth()
{
	return fMaxHealth;
}

int UHealthComponent::GetPlayerLives()
{
	return iCurrentLives;
}

int UHealthComponent::GetPlayerMaxLives()
{
	return iMaxLives;
}

void UHealthComponent::SetMaxHealth( float fSetMaxHealth, bool bSetCurrentHealth )
{
	Max_Health = fSetMaxHealth;
	fMaxHealth = Max_Health;
	if( bSetCurrentHealth )
	{
		fCurrentHealth = fMaxHealth;
		Current_Health = fCurrentHealth;
	}

}

void UHealthComponent::SetMaxLives( int fSetMaxLives, bool bSetCurrentLives )
{
	Max_Lives = fSetMaxLives;
	iMaxLives = Max_Lives;
	if( bSetCurrentLives )
	{
		iCurrentLives = iMaxLives;
		Current_Lives = iCurrentLives;		
	}	
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps( OutLifetimeProps );
}

float UHealthComponent::GetCurrentArmourValue()
{
	return fCurrentArmourValue;
}

void UHealthComponent::RegenerateArmour( float fDeltaTime )
{
	if( bActorHasBeenHit  )
	{
		fArmourRegenTimer += fDeltaTime;
		if( fArmourRegenTimer >= fArmourRegenDelay )
		{
			bActorHasBeenHit = false;
			fArmourRegenTimer -= fArmourRegenTimer;
			bArmourRegenerate = true;
		}
	}
	if( bArmourRegenerate )
	{
		if( fCurrentArmourValue < fMaxArmourValue )
		{
			fArmourRegenTimer += fDeltaTime;
			if( fArmourRegenTimer >= fArmourRegenRate )
			{
				fCurrentArmourValue += (fMaxArmourValue / 10);
				fArmourRegenTimer -= fArmourRegenTimer;
				Current_Armour = fCurrentArmourValue;
				if( fCurrentArmourValue > fMaxArmourValue )
				{
					fCurrentArmourValue = fMaxArmourValue;
				}
				Client_UpdateArmour( fCurrentArmourValue );
			}
		}
	}
}

bool UHealthComponent::GetPlayerReviveStatus()
{
	return bIsReviving;
}

void UHealthComponent::Client_UpdateReviveStatus_Implementation( bool bNewRevive)
{
	bIsReviving = bNewRevive;
	fReviveDelay = 0.0f;
}