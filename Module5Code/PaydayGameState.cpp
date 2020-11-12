// Fill out your copyright notice in the Description page of Project Settings.


#include "PaydayGameState.h"

void APaydayGameState::MultiCast_SetDetectionValue_Implementation( float fNewDetectionValue )
{
	fDetectionValue = fNewDetectionValue;
}

void APaydayGameState::MultiCast_SetCurrentGameState_Implementation( EPaydayGameState eGameState )
{
	ePaydayGameState = eGameState;
}

void APaydayGameState::MultiCast_SetCurrentObjectiveState_Implementation(EPaydayObjectiveState eNewState )
{
	ePaydayCurrentObjective = eNewState;
}

void APaydayGameState::SetDetectionValue(float fNewDetectionValue)
{
	// Fire network values for sync detection value
	MultiCast_SetDetectionValue( fNewDetectionValue );
}

float APaydayGameState::GetDetectionValue()
{
	return fDetectionValue;
}

void APaydayGameState::SetCurrentGameState( EPaydayGameState eGameState )
{
	// Fire the network values for sync the current game state value
	MultiCast_SetCurrentGameState( eGameState );
	if (eGameState == GAME_END)
	{
		OnGameEndedDelegate.Broadcast();
	}
}

EPaydayGameState APaydayGameState::GetCurrentGameState()
{
	return ePaydayGameState;
}

void APaydayGameState::SetPlayersAreHostile( bool bIsPlayerHostile )
{
	bPlayersAreHostile = bIsPlayerHostile;
	if (bIsPlayerHostile)
	{
		OnPlayerDetectedDelegate.Broadcast();
	}
}

bool APaydayGameState::GetArePlayersHostile()
{
	return bPlayersAreHostile;
}

void APaydayGameState::SetCurrentObjective(EPaydayObjectiveState eCurrentObjectiveState)
{
	// Fire the multicast function for replicating the current objective state
	MultiCast_SetCurrentObjectiveState( eCurrentObjectiveState );
	
	// Fire an event when the objective is changed
	OnPaydayObjectiveChanged.Broadcast( ePaydayCurrentObjective );
}

EPaydayObjectiveState APaydayGameState::GetCurrentObjectiveState()
{
	return ePaydayCurrentObjective;
}

void APaydayGameState::SetDrillActivationStatus(bool bDrillActivationStatus)
{
	bIsDrillActivated = bDrillActivationStatus;
	
	if(bDrillActivationStatus)
	{
		OnDrillActivatedDelegate.Broadcast();
	}
}

bool APaydayGameState::GetDrillActivationStatus()
{
	return bIsDrillActivated;
}
