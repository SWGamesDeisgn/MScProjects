// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Drill.generated.h"

UCLASS()
class PAYDAY_API ADrill : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADrill();

	// Called every frame
	virtual void Tick(float DeltaTime) override;



	/**
	 * \brief Wrapper for calling the server side (See Server_ActivateDrill)
	 * \param cVault Target bank vault
	 */
	void SetBankVault						(class ABankVault* cVault);

	/**
	 * \brief Call the server and start the drill
	 * \param cVault The target vault
	 */
	UFUNCTION( Server, Reliable, WithValidation )
	void Server_ActivateDrill				( class ABankVault* cVault );
	bool Server_ActivateDrill_Validate		( class ABankVault* cVault );
	void Server_ActivateDrill_Implementation( class ABankVault* cVault );

	/*
	 * Timer for drill activation
	 */
	FTimerHandle DestroyDrillTimerHandler;
	UFUNCTION()
	void OnDrillDestroyed();

	//functions to handle deactivation and reactivation of the drill
	void DeactivateDrill();

	void StartActivatingDrill();

	void StopActivatingDrill();

	// SFX
	void PlayDrillingSFX();
	void StopDrillingSFX();
	// VFX
	void PlayDrillingVFX();
	void StopDrillingVFX();
	
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void ActivateDrill();

	UPROPERTY(VisibleAnywhere, Category = "Box Component", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent*				cBoxComponent;

	UPROPERTY(VisibleAnywhere, Category = "Static Mesh", meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent*			cStaticMeshComponent;

	UPROPERTY(EditAnywhere, Category = "Design Stats", meta = (AllowPrivateAccess = "true"))
	float								fMaxDrillTime;

	UPROPERTY(EditAnywhere, Category = "Design Stats", meta = (AllowPrivateAccess = "true"))
	float								fMaxDrillReactivationTime;

	/** Specify which VFX to use for the Drill's VFX emitter. */
	UPROPERTY( EditAnywhere , BlueprintReadWrite, Category = "Design Stats" , meta = (AllowPrivateAccess = "true") )
	class UParticleSystem*				cSparksEmitter;
	
	/** Locator used to specify the location of the VFX emitter. */
	UPROPERTY( EditAnywhere , BlueprintReadWrite, Category = "Design Stats" , meta = (AllowPrivateAccess = "true") )
	class UBoxComponent*				cEmitterLocation;

	class UParticleSystemComponent*		cParticleSystem;
	class ABankVault*					cBankVault;

	/**
	 * \brief Play audio sound cue
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "SFX Settings", meta = ( AllowPrivateAccess = "true" ) )
	class UAudioComponent*				cAudioComponent;

	bool								bIsValutDestroyed;

	//Variables used for deactivation and activation of drill;

	FTimerHandle						ReactivateDrillTimer;

	bool								bIsDrillDeactivated;

	// Drill HUD
	class UWidgetComponent*				cWidgetComponent;
	class UDrillProgressPanel*			cPanelWidget;

};
