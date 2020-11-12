// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "PaydayGameState.generated.h"

UENUM()
enum EPaydayGameState
{
	INIT, // Player is waiting for the connection or initialisation
	NORMAL, // Player is not wearing mask
	HIDDEN_HEIST, // Player wear mask but the alert is not be triggered
	ALERT_TRIGGERED, // Player wear mask and the alert is triggered
	ESCAPE, // Player opened the vault and might got or not money from the vault and they are now escaping
	GAME_END, // Player died or the escaped
	ERROR // Unexpected situation occured
};

UENUM()
enum class EPaydayObjectiveState
{
	FIND_THE_HIDDEN_DRILL,
	OPEN_THE_VAULT,
	GRAB_THE_MONEY,
	TAKE_THE_MONEY_TO_THE_VAN,
	EXTRACT,
	REACTIVATE_THE_DRILL,

	// Tutorial State, extend states
	TUTORIAL_FIND_KEY_CARD,
	TUTORIAL_OPEN_THE_DOOR
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE( FPlayerDetectedDelegate );
DECLARE_DYNAMIC_MULTICAST_DELEGATE( FDrillActivatedDelegate );
DECLARE_DYNAMIC_MULTICAST_DELEGATE( FGameEndedDelegate );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FPaydayGameObjectiveOnChangedDelegate, EPaydayObjectiveState, eObjectiveState );

/**
 * A game state class that is used to store some global values or states.
 * PS: If you want to save or get some values that are related to player, please use PlayerState.
 */
UCLASS()
class PAYDAY_API APaydayGameState : public AGameStateBase
{
	GENERATED_BODY()

	/**
	 * \brief A value that is used to represent the current detection value, which will trigger the alert when it reaches a certain value
	 */
	float fDetectionValue;

	/**
	 * \brief The current game state of the game
	 */
	EPaydayGameState ePaydayGameState;

	/**
	 * \brief The current objective of the game
	 */
	EPaydayObjectiveState ePaydayCurrentObjective;

	/**
	* \brief A bool that represents if the players have equip their masks or not.
	*/
	bool			bPlayersAreHostile;

	bool			bIsDrillActivated;
	/**
	 * \brief Sync the detection value with other players / clients
	 * \param fNewDetectionValue Detection Value
	 */
	UFUNCTION( NetMulticast, Reliable )
	void MultiCast_SetDetectionValue( float fNewDetectionValue );
	void MultiCast_SetDetectionValue_Implementation( float fNewDetectionValue );

	/**
	 * \brief Sync the current game state with other players / clients
	 * \param eGameState New current game state value 
	 */
	UFUNCTION( NetMulticast, Reliable )
	void MultiCast_SetCurrentGameState( EPaydayGameState eGameState );
	void MultiCast_SetCurrentGameState_Implementation( EPaydayGameState eGameState );

	UFUNCTION( NetMulticast, Reliable )
	void MultiCast_SetCurrentObjectiveState( EPaydayObjectiveState eNewState );
	void MultiCast_SetCurrentObjectiveState_Implementation( EPaydayObjectiveState eNewState );

public:
	/**
	 * \brief Update the detection value
	 * \param fNewDetectionValue New detection value
	 */
	void SetDetectionValue( float fNewDetectionValue );
	/**
	 * \brief Get the current detection value
	 * \return Current detection value
	 */
	UFUNCTION( BlueprintCallable )
	float GetDetectionValue();

	/**
	 * \brief Update the current game state
	 * \param eGameState New game state value
	 */
	void SetCurrentGameState( EPaydayGameState eGameState );
	/**
	 * \brief Get the current game state
	 * \return Current game state
	 */
	EPaydayGameState GetCurrentGameState();

	/**
	 * \brief Sets bPlayersAreHostile to true when any player has equip a mask.
	*/
	void SetPlayersAreHostile( bool bIsPlayerHostile );

	/**
	 * \brief Returns true if any player has equip a mask, false if they have not.
	*/
	bool GetArePlayersHostile();

	/**
	 * \brief Set the current objective state
	 * \param eCurrentObjectiveState New objective state
	 */
	void SetCurrentObjective( EPaydayObjectiveState eCurrentObjectiveState );

	/**
	 * \brief Get the current objective state
	 * \return Current objective state
	 */
	EPaydayObjectiveState GetCurrentObjectiveState();

	void SetDrillActivationStatus(bool bDrillActivationStatus);

	bool GetDrillActivationStatus();


	UPROPERTY(BlueprintAssignable, Category = "EventDispatcher")
		FPlayerDetectedDelegate OnPlayerDetectedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "EventDispatcher")
		FDrillActivatedDelegate OnDrillActivatedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "EventDispatcher")
		FGameEndedDelegate OnGameEndedDelegate;

	/**
	 * \brief A delegate function for the game state is changed
	 */
	UPROPERTY( BlueprintAssignable, Category = "EventDispatcher" )
	FPaydayGameObjectiveOnChangedDelegate OnPaydayObjectiveChanged;

};