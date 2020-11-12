// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Components/AnimatedUserWidget.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Payday/UI/Components/DetectionWidget.h"
#include "Payday/UI/Components/ObjectiveHints.h"
#include "Payday/UI/Components/ResultStatPanel.h"
#include "Payday/UI/Menus/OptionsMenu.h"

#include "IngameScreen.generated.h"

/**
 * 
 */
UCLASS()
class PAYDAY_API UIngameScreen : public UAnimatedUserWidget
{
	GENERATED_BODY()

	/*
	 * UI References from in-game Screen Blueprint
	 */
	UPROPERTY( meta = ( BindWidget ) )
	UDetectionWidget* DetectionComponent;
	UPROPERTY( meta = ( BindWidget ) )
	UProgressBar* ArmourBar;
	UPROPERTY( meta = ( BindWidget ) )
	UProgressBar* HealthBar;
	UPROPERTY( meta = ( BindWidget ) )
	UTextBlock* AmmoText;
	UPROPERTY( meta = ( BindWidget ) )
	UTextBlock* MoneyText;
	UPROPERTY( meta = ( BindWidget ) )
	UBorder* TimerContainer;
	UPROPERTY( meta = ( BindWidget ) )
	UTextBlock* TimerText;
	UPROPERTY( meta = ( BindWidget ) )
	UObjectiveHints* ObjectiveComponent;
	UPROPERTY( meta = ( BindWidget ) )
	UResultStatPanel* ResultPanel;
	UPROPERTY( meta = ( BindWidget ) )
	UOptionsMenu* OptionsMenu;

public:

	/*
	 * User Widget Virtual Methods
	 */
	virtual void NativeConstruct() override;

	/*
	 * Interfaces
	 */
	/**
	 * \brief Update the detection value
	 * \param fNewDetectionValue Detection value 
	 */
	void UpdateDetection( float fNewDetectionValue );
	/**
	 * \brief Update the player health
	 * \param fNewHealthValue New health value
	 */
	void UpdateHealth( float fNewHealthValue );
	/**
	 * \brief Update the player armour
	 * \param fNewArmourValue New armour value
	 */
	void UpdateArmour( float fNewArmourValue );
	/**
	 * \brief Update the ammo text
	 * \param iCurrentAmmoCount Current weapon current ammo count
	 * \param iMaxAmmoCount Current weapon max ammo count
	 */
	void UpdateAmmo( int iCurrentAmmoCount, int iMaxAmmoCount );

	/**
	 * \brief Update the money text
	 * \param iCurrentMoney Current money that the player is carrying
	 */
	void UpdateMoney( int iCurrentMoney );

	/**
	 * \brief Update the timer text
	 * \param fRemainSeconds Current remaining seconds from the timer
	 */
	void UpdateTimer( float fRemainSeconds );

	/**
	 * \brief Update the objective text
	 * \param strObjectiveText Set the objective text
	 */
	void SetObjectiveText( FString strObjectiveText );
	
	/**
	 * \brief Update objective animated texture
	 * \param eHintType Hint type
	 */
	void SetObjectiveMessage( EPaydayObjectiveState eHintType );

	/**
	 * \brief Show result panel
	 */
	void ShowResultPanel();

	/**
	 * \brief Hide result panel
	 */
	void HideResultPanel();

	/**
	 * \brief Show the pause menu
	 */
	void ShowPauseMenu();

	/**
	 * \brief Hide the pause menu
	 */
	void HidePauseMenu();

	/**
	 * \brief Show options menu
	 */
	void ShowOptionsMenu();

	/**
	 * \brief Hide options menu
	 */
	void HideOptionsMenu();
	
};