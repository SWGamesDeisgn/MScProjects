// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once 

#include "CoreMinimal.h"
#include "UI/Components/DetectionWidget.h"
#include "UI/HUDs/BaseHUD.h"
#include "UI/Screens/IngameScreen.h"
#include "UI/Styles/PaydayGameThemeWidgetStyle.h"
#include "PaydayHUD.generated.h"

UENUM()
enum EPaydayHUDState
{
	IN_GAME_SCREEN,
	SHOWING_RESULT_PANEL,
	SHOWING_PAUSE_MENU,
	SHOWING_OPTIONS_MENU,
};

UCLASS()
class APaydayHUD : public ABaseHUD
{
	GENERATED_BODY()

	// UI Theme Styleset
	USlateWidgetStyleAsset* cDefaultThemeClass;
	UPaydayGameThemeWidgetStyle* cDefaultTheme;

	// UI State
	EPaydayHUDState eState;
	
public:
	APaydayHUD();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;
	virtual void BeginPlay() override;

	/*
	 * Key Action Binding
	 */
	UFUNCTION()
	void OnEscapeKeyPressed();
	
	/*
	 * Get UI Theme
	 */
	UPaydayGameThemeWidgetStyle* GetTheme();
	
	/*
	 * Debug
	 */
	UPROPERTY(EditAnywhere)
	TSubclassOf<UDetectionWidget> DectionWidgetClass;
	UDetectionWidget* DetectionWidget;

	/*
	 * UI Settings
	 */
	UPROPERTY( EditAnywhere, Category = "UI Settings" )
	TSubclassOf<UIngameScreen> IngameScreenClass;
	UIngameScreen* ingameScreen;

	UPROPERTY( EditAnywhere, Category = "UI Settings" )
	bool bIsTutorialLevel;
	
	/*
	 * UI Access Interfaces
	 */	
	/**
	 * \brief Update the detection value
	 * \param fDetectionPercentage Detection value 
	 */
	void UpdateDetection( float fDetectionPercentage );
	/**
	 * \brief Update the player health
	 * \param fNewHealthPercentage New health value
	 */
	void UpdateHealth( float fNewHealthPercentage );
	/**
	 * \brief Update the player armour
	 * \param fNewArmourPercentage New armour value
	 */
	void UpdateArmour( float fNewArmourPercentage );
	/**
	 * \brief Update the ammo text
	 * \param iCurrentAmmoCount Current weapon current ammo count
	 * \param iMaxAmmoCount Current weapon max ammo count
	 */
	void UpdateAmmo( int iCurrentAmmoCount, int iMaxAmmoCount );

	/**
	 * \brief Update the money text
	 * \param iCurrentMoney Current money amount that the player is carrying
	 */
	void UpdateMoney( int iCurrentMoney );

	/**
	 * \brief Update the timer text
	 * \param fRemainSeconds Current remaining seconds from the timer
	 */
	void UpdateTimer( float fRemainSeconds );

	/**
	 * \brief Update objective text
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
	 * \brief Show pause menu
	 */
	void ShowPauseMenu();

	/**
	 * \brief Hide pause menu
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

	/*
	 * Debug Command-line
	 */
	UFUNCTION( Exec, Category = ExecFunctions )
	void DebugCLI_UpdateHealth( float fNewHealthPercentage );
	UFUNCTION( Exec, Category = ExecFunctions )
	void DebugCLI_UpdateDetection( float fNewDetectionPercentage );
	UFUNCTION( Exec, Category = ExecFunctions )
	void DebugCLI_UpdateArmour( float fNewArmourPercentage );
	UFUNCTION( Exec, Category = ExecFunctions )
	void DebugCLI_UpdateAmmo( int iCurrentAmmoCount, int iMaxAmmoCount );
	UFUNCTION( Exec, Category = ExecFunctions )
	void DebugCLI_UpdateMoney( int iCurrentMoney );
	UFUNCTION( Exec, Category = ExecFunctions )
	void DebugCLI_ShowTimer( float fRemainSeconds );
	UFUNCTION( Exec, Category = ExecFunctions )
	void DebugCLI_SetObjectiveText( FString strObjectiveText );
	UFUNCTION( Exec, Category = ExecFunctions )
	void DebugCLI_SetObjectiveHint( uint8 iHintType );
	UFUNCTION( Exec, Category = ExecFunctions )
	void DebugCLI_RecordPlayerDamageTaken( float fNewDamageTaken  );
	UFUNCTION( Exec, Category = ExecFunctions )
	void DebugCLI_RecordPlayerCollectedMoney( int iAmount );
	UFUNCTION( Exec, Category = ExecFunctions )
	void DebugCLI_RecordPlayerDeathCount();
	UFUNCTION( Exec, Category = ExecFunctions )
	void DebugCLI_ShowResultPanel();
	UFUNCTION( Exec, Category = ExecFunctions )
	void DebugCLI_HideResultPanel();
	UFUNCTION( Exec, Category = ExecFunctions )
	void DebugCLI_PauseGame();
	UFUNCTION( Exec, Category = ExecFunctions )
	void DebugCLI_ResumeGame();
	UFUNCTION( Exec, Category = ExecFunctions )
	void DebugCLI_RecordPlayerKillCount( int iToAddKillCount );
	
	FTimerHandle cDebugTimerHandle;
	UFUNCTION()
	void Debug_OnTimerFinished();
	
private:
	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;

};

