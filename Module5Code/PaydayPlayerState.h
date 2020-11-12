// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Payday/EKeycardTypes.h"
#include "PaydayPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class PAYDAY_API APaydayPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	
	// Player health
	UPROPERTY( BlueprintReadWrite )
	float fPlayerHealth;

	// Player Lives
	UPROPERTY( BlueprintReadWrite )
	int iPlayerLives;

	// Player Ammo
	UPROPERTY( BlueprintReadWrite )
	int iCurrentAmmo;
	UPROPERTY( BlueprintReadWrite )
	int iMaxAmmo;
	UPROPERTY( BlueprintReadWrite )
	int iAmmoReserve;
	UPROPERTY( BlueprintReadWrite )
	int iAmmoReserveMax;

	// Player Money
	UPROPERTY( BlueprintReadWrite )
	int iPlayerCarriedMoney;

	// Player Armour
	UPROPERTY( BlueprintReadWrite )
	float fPlayerArmour;

	// Result Report Properties
	/**
	 * \brief How many damage did the player take
	 */
	float fPlayerDamageTaken;
	/**
	 * \brief How many times the player died during the game
	 */
	int iPlayerDiedCount;
	/**
	 * \brief How much player earned from the mission
	 */
	int iPlayerCollectedMoney;
	/**
	 * \brief How many enemies have been killed by the player
	 */
	int iKillCount;
	bool bGreenCard;
	bool bBlueCard;
	/*
	 * Getters & Setters
	 */
	
	// Player Health
	void SetPlayerHealth( float fNewHealth );
	UFUNCTION( BlueprintCallable )
	float GetPlayerHealth();

	// Player Lives
	void SetPlayerLives( int iNewLives );
	UFUNCTION( BlueprintCallable )
	int GetPlayerLives();

	// Player Ammo and Max Ammo
	void SetCurrentAmmo( int iNewCurrentAmmo );
	UFUNCTION( BlueprintCallable )
	int GetCurrentAmmo();
	void SetMaxAmmo( int iNewMaxAmmo );
	UFUNCTION( BlueprintCallable )
	int GetMaxAmmo();

	void SetAmmoReserve( int iNewAmmoReserve );
	UFUNCTION( BlueprintCallable )
	int GetAmmoReserve();
	void SetMaxAmmoReserve( int iNewAmmoReserveMax );
	UFUNCTION( BlueprintCallable )
	int GetMaxAmmoReserve();

	// Player Money
	void SetCurrentMoney( int iNewMoney );
	UFUNCTION( BlueprintCallable )
	int GetCurrentMoney();

	// Player Armour
	void SetPlayerArmour( float fNewArmour );
	UFUNCTION( BlueprintCallable )
	float GetPlayerArmour();

	// Result Report

	// Taken damage
	void RecordPlayerDamageTaken( float fDamage );
	float GetPlayerDamageTaken();

	// Died counter
	void RecordPlayerDied();
	int GetPlayerDiedCount();

	// Total collected money amount
	void RecordPlayerCollectedMoney( int iAmount );
	int GetPlayerCollectedMoney();

	// Kill count
	void AddKillCountToRecord();
	void AddKillCountToRecord( int iToAddKillCount );
	int GetPlayerKillCount();

	TArray<EKeycardTypes> GetKeycard();
	void SetKeycard( EKeycardTypes eKeycard );
	void RemoveKeycard( EKeycardTypes eKeycard );

private:
	TArray<EKeycardTypes> aKeycards;
};