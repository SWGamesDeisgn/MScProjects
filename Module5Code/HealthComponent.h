// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"


UCLASS( ClassGroup = (Custom) , meta = (BlueprintSpawnableComponent) )
class PAYDAY_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UHealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	/**
	* Does not have affect during runtime.
	*/
	UPROPERTY( EditAnywhere , Category = "Design Stats - Health" , meta = (ClampMin = 0.0f) )
		float 					Max_Health;
	/**
	* Does not have affect during runtime.
	*/
	UPROPERTY( VisibleAnywhere , Category = "Design Stats - Health" )
		float 					Current_Health;
	/**
	* Does not have affect during runtime.
	*/
	UPROPERTY( EditAnywhere , Category = "Design Stats - Health" , meta = (ClampMin = 1) )
		int 					Max_Lives;
	/**
	* Does not have affect during runtime.
	*/
	UPROPERTY( VisibleAnywhere , Category = "Design Stats - Health" )
		int 					Current_Lives;
	// Armour 
	/**
	* Does not have affect during runtime.
	*/
	UPROPERTY( VisibleAnywhere , Category = "Design Stats - Armour" )
		float 					Current_Armour;
	/**
	* Does not have affect during runtime.
	*/
	UPROPERTY( EditAnywhere , Category = "Design Stats - Armour" , meta = (ClampMin = 0.0f) )
		float 					Max_Armour;
	/**
	* Does not have affect during runtime.
	*/
	UPROPERTY( EditAnywhere , Category = "Design Stats - Armour" )
		float 					Armour_Regen_Speed;	
	/**
	* Does not have affect during runtime.
	*/
	UPROPERTY( EditAnywhere , Category = "Design Stats - Armour" )
		float 					Armour_Regen_Delay;
	/**
	* Does not have affect during runtime.
	*/
	UPROPERTY( EditAnywhere , Category = "Design Stats - Armour" )
		bool 					Armour_Regenerates;
	/**
	* Delay in seconds until the player revives if they have lives left.
	*/
	UPROPERTY( EditAnywhere )
		float							ReviveDelay;

	// Called every frame
	virtual void TickComponent( float DeltaTime , ELevelTick TickType , FActorComponentTickFunction* ThisTickFunction ) override;

	//Remove X amount of health from the player, if damage is greater than current health, health will stop at zero.
	void ReduceHealth			( float fReduceAmount );
	UFUNCTION( Server, Reliable )
	void Server_ReduceHealth( float fReduceAmount );
	void Server_ReduceHealth_Implementation( float fReduceAmount );
	
	//Restore X amount of health to the player, if healing is capped to max health of the player.
	void ReplenishHealth		( float fReplenishAmount );
	UFUNCTION( Server, Reliable )
	void Server_ReplenishHealth( float fReplenishAmount );
	void Server_ReplenishHealth_Implementation( float fReplenishAmount );

	//Fully heal the player back to max health.
	void FullHeal				();
	UFUNCTION( Server, Reliable )
	void Server_FullHeal();
	void Server_FullHeal_Implementation();
	
	//Revives a dead player back to full health, reviving costs a life.
	void RevivePlayer			();
	UFUNCTION( Server, Reliable )
	void Server_RevivePlayer();
	void Server_RevivePlayer_Implementation();
	
	//Give a player more lives.
	void AddPlayerLives			( int iAmountToAdd );
	UFUNCTION( Server, Reliable )
	void Server_AddPlayerLives( int iAmountToAdd );
	void Server_AddPlayerLives_Implementation( int iAmountToAdd );
	//Sets the players Max Health, also by default sets current health to max health.

	/*
	 * Replication Usage
	 */
	UFUNCTION( Client, Reliable )
	void Client_UpdateHealth( float fNewHealth );
	void Client_UpdateHealth_Implementation( float fNewHealth );

	UFUNCTION( Client, Reliable )
	void Client_UpdateLives( int iNewLivesCount );
	void Client_UpdateLives_Implementation( int iNewLivesCount );

	UFUNCTION( Client, Reliable )
	void Client_UpdatePlayerIsDead( bool bNewPlayerDead );
	void Client_UpdatePlayerIsDead_Implementation( bool bNewPlayerDead );

	UFUNCTION( Client , Reliable )
	void Client_UpdateArmour( float fNewArmour );
	void Client_UpdateArmour_Implementation( float fNewArmour );	

	UFUNCTION( Client , Reliable )
	void Client_UpdateReviveStatus( bool bNewRevive );
	void Client_UpdateReviveStatus_Implementation( bool bNewRevive );

	//Getters & Setters

	//Is our player dead yet? returns true if dead otherwise false.
	bool IsPlayerDead();
	//Returns a float value for the player's current health.
	float GetPlayerHealth();
	//Returns a float value for the player's maximum allowed health.
	float GetPlayerMaxHealth();
	//Returns an int value for the player's current remaining lives.
	int GetPlayerLives();
	//Returns and int value for the player's maximum allowed lives. 
	int GetPlayerMaxLives();

	void SetMaxHealth( float fSetMaxHealth , bool bSetCurrentHealth = true );
	//Sets the players Max Lives, also by default sets current lives to max lives.
	void SetMaxLives( int iSetMaxLives , bool bSetCurrentLives = true );

	//Armour Getters & Setters
	float GetCurrentArmourValue();

	// Reviving Getters
	bool GetPlayerReviveStatus();

private:
	/**
	* Testing switches to check functionality of the code.
	*/
	UPROPERTY( EditAnywhere , Category = "Functionality Testing" )
		bool							DamagePlayer;
	/**
	* Testing switches to check functionality of the code.
	*/
	UPROPERTY( EditAnywhere , Category = "Functionality Testing" )
		bool							HealPlayer;
	/**
	* Testing switches to check functionality of the code.
	*/
	UPROPERTY( EditAnywhere , Category = "Functionality Testing" )
		bool							FullHealPlayer;
	/**
	* Testing switches to check functionality of the code.
	*/
	UPROPERTY( EditAnywhere , Category = "Functionality Testing" )
		bool							ResurrectPlayer;
	/**
	* Testing switches to check functionality of the code.
	*/
	UPROPERTY( EditAnywhere , Category = "Functionality Testing" )
		bool							GivePlayerLives;

	float  								fMaxHealth;
	float								fCurrentHealth;
	bool								bPlayerDead;
	int									iCurrentLives;
	int									iMaxLives;

	// Armour
	float								fCurrentArmourValue;
	float								fMaxArmourValue;
	float								fArmourRegenRate;
	float								fArmourRegenDelay;
	float								fArmourRegenTimer;
	bool								bActorHasBeenHit;
	bool								bArmourRegenerate;
	bool								bArmourRegeneration;

	// Reviving
	UPROPERTY (VisibleAnywhere)
		float								fReviveDelay;
	bool									bIsReviving;

	void RegenerateArmour( float fDeltaTime );

public:

	virtual void GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const override;
};