// Fill out your copyright notice in the Description page of Project Settings.


#include "PaydayPlayerState.h"

void APaydayPlayerState::SetPlayerHealth(float fNewHealth)
{
	fPlayerHealth = fNewHealth;
}

float APaydayPlayerState::GetPlayerHealth()
{
	return fPlayerHealth;
}

void APaydayPlayerState::SetPlayerLives( int iNewLives )
{
	iPlayerLives = iNewLives;
}

int APaydayPlayerState::GetPlayerLives()
{
	return iPlayerLives;
}

void APaydayPlayerState::SetCurrentAmmo( int iNewCurrentAmmo )
{
	iCurrentAmmo = iNewCurrentAmmo;
}

int APaydayPlayerState::GetCurrentAmmo()
{
	return iCurrentAmmo;
}

void APaydayPlayerState::SetMaxAmmo( int iNewMaxAmmo )
{
	iMaxAmmo = iNewMaxAmmo;
}

int APaydayPlayerState::GetMaxAmmo()
{
	return iMaxAmmo;
}

void APaydayPlayerState::SetAmmoReserve( int iNewAmmoReserve )
{
	iAmmoReserve = iNewAmmoReserve;
}

int APaydayPlayerState::GetAmmoReserve() 
{
	return iAmmoReserve;
}

void APaydayPlayerState::SetMaxAmmoReserve( int iNewAmmoReserveMax )
{
	iAmmoReserveMax = iNewAmmoReserveMax;
}

int APaydayPlayerState::GetMaxAmmoReserve() 
{
	return iAmmoReserveMax;
}

void APaydayPlayerState::SetCurrentMoney( int iNewMoney )
{
	iPlayerCarriedMoney = iNewMoney;
}

int APaydayPlayerState::GetCurrentMoney()
{
	return iPlayerCarriedMoney;
}

void APaydayPlayerState::SetPlayerArmour( float fNewArmour )
{
	fPlayerArmour = fNewArmour;
}

float APaydayPlayerState::GetPlayerArmour()
{
	return fPlayerArmour;
}

void APaydayPlayerState::RecordPlayerDamageTaken( float fDamage )
{
	fPlayerDamageTaken += fDamage;
}

float APaydayPlayerState::GetPlayerDamageTaken()
{
	return fPlayerDamageTaken;
}

void APaydayPlayerState::RecordPlayerDied()
{
	iPlayerDiedCount++;
}

int APaydayPlayerState::GetPlayerDiedCount()
{
	return iPlayerDiedCount;
}

void APaydayPlayerState::RecordPlayerCollectedMoney( int iAmount )
{
	iPlayerCollectedMoney += iAmount;
}

int APaydayPlayerState::GetPlayerCollectedMoney()
{
	return iPlayerCollectedMoney;
}

void APaydayPlayerState::AddKillCountToRecord()
{
	AddKillCountToRecord( 1 );
}

void APaydayPlayerState::AddKillCountToRecord( int iToAddKillCount )
{
	iKillCount += iToAddKillCount;
}

int APaydayPlayerState::GetPlayerKillCount()
{
	return iKillCount;
}

TArray<EKeycardTypes> APaydayPlayerState::GetKeycard()
{
	return aKeycards;
}

void APaydayPlayerState::SetKeycard( EKeycardTypes eKeycard )
{
	if( !aKeycards.Contains( eKeycard ) )
	{
		aKeycards.Add( eKeycard );
	}
}

void APaydayPlayerState::RemoveKeycard( EKeycardTypes eKeycard )
{
	if( aKeycards.Contains( eKeycard ) )
	{
		aKeycards.Remove( eKeycard );
	}
}