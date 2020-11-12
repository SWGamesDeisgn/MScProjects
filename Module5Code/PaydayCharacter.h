// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Gameplay/ActorComponents/ReplicatedInteractable.h"
#include "PaydayCharacter.generated.h"


class UInputComponent;

UCLASS(config=Game)
class APaydayCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	class USkeletalMeshComponent* Mesh1P;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* FP_Gun;

	/** Location on gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USceneComponent* FP_MuzzleLocation;

	/** Gun mesh: VR view (attached to the VR controller directly, no arm, just the actual gun) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* VR_Gun;

	/** Location on VR gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USceneComponent* VR_MuzzleLocation;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;

	/** Motion controller (right hand) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UMotionControllerComponent* R_MotionController;

	/** Motion controller (left hand) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UMotionControllerComponent* L_MotionController;

public:
	APaydayCharacter();
protected:
	virtual void BeginPlay();

public:
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector GunOffset;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	TSubclassOf<class APaydayProjectile> ProjectileClass;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	class USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class UAnimMontage* FireAnimation;

	/** Whether to use motion controller location for aiming. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	uint32 bUsingMotionControllers : 1;

protected:
	
	/** Fires a projectile. */
	// Handle the click event
	void OnFireClicked();
	void OnFireReleased();

	// Declare the RPC function for shooting
	UFUNCTION( Server, Reliable )
	void OnFire();
	void OnFire_Implementation();

	UFUNCTION( Client, Reliable )
	void Client_OnFireFinished();
	void Client_OnFireFinished_Implementation();

	/** Resets HMD orientation and position in VR. */
	void OnResetVR();

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	struct TouchData
	{
		TouchData() { bIsPressed = false;Location=FVector::ZeroVector;}
		bool bIsPressed;
		ETouchIndex::Type FingerIndex;
		FVector Location;
		bool bMoved;
	};
	void BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location);
	TouchData	TouchItem;
	void ReloadWeapon();
	
	/**
	 * \brief Call the server-side to reload
	 */
	UFUNCTION( Server , Reliable )
	void Server_ReloadGun();
	void Server_ReloadGun_Implementation();

	UFUNCTION( Server, Reliable )
	void Server_DeployDrill();
	void Server_DeployDrill_Implementation();
	
	// We're overriding the base Jump function from ACharacter so that we can stop the player jumping when they are dead.
	virtual void Jump() override;

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

	/* 
	 * Configures input for touchscreen devices if there is a valid touch interface for doing so 
	 *
	 * @param	InputComponent	The input component pointer to bind controls to
	 * @returns true if touch controls were enabled.
	 */
	bool EnableTouchscreenMovement(UInputComponent* InputComponent);

public:
	/** Returns Mesh1P subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	UPROPERTY( EditAnywhere )
	UParticleSystem* cParticles;

	/**
	 * \brief Set target interactive
	 * \param cTargetItem Target interactive
	 */
	UReplicatedInteractable* pCurrentInteractable;
	void SetTargetInteractive( UReplicatedInteractable* cTargetItem );
	
	/**
	 * \brief Interact item event handler
	 */
	void StartInteracting();
	void StopInteracting();

	/*
	 * Work around for Drill to play sfx...
	 */
	void PlayDrillingSFX();
	void StopDrillingSFX();

	void Extract();

	class UHealthComponent* GetHealthComp();

	/*
	 * Debug Health Component functionality
	 */
	void OnDebugHealthIncrease();
	void OnDebugHealthDecrease();
	void OnDebugPrintAllPlayersHealth();

	UFUNCTION( Server, Reliable )
	void PrintAllPlayersHealth();
	void PrintAllPlayersHealth_Implementation();

	// To Do - Make an inventory to store this in.
	void SetPlayerHasKey( bool bKey );
	bool GetPlayerInteracting();
	bool GetPlayerHasKey();
	void SetPlayerPrimaryGun( class ABaseGun* cGun );
	void SetPlayerSecondaryGun( class ABaseGun* cGun );
	bool IsPlayerHostile();
	USkeletalMeshComponent* GetArmMesh();

	void SetMoney(float fMoney);
	void PlayerHasDied();


private:
	// This might need to be moved to a "gamemaster" object if the keycard is meant to be player independant
	// meaning that if ANY player has the keycard then ALL players have it etc.
	// but for now we're just using this to test with.
	bool							bHasKeycard;
	bool							bIsInteracting;
	UPROPERTY( EditAnywhere )
	class UHealthComponent*			cPlayerHealthComp;
	class ABaseGun*					cPlayerPrimaryGun;
	class ABaseGun*					cPlayerSecondaryGun;
	class ABaseGun*					cPlayerCurrentGun;
	bool							bIsPlayerHostile;
	bool							bPlayerIsShooting;
	UPROPERTY(EditAnywhere)
	class UDetectionComponent*		cPlayerDetectionComp;
	UPROPERTY( EditAnywhere )
	class USoundBase* ItemCollectedSound;
	UPROPERTY( EditAnywhere )
	class USoundBase* DrillingSound;
	
	class UAudioComponent* DrillAudioComponent;

	class APaydayGameState*			cPaydayGameState;
	class APaydayHUD*				cHUD;
	class APaydayPlayerController*  cThisPaydayController;
	class APaydayPlayerState*		cPlayerState;

	class ABankVault*				cBankVault;
	float							fMoneyHeld;
	class AExtractionPoint*			cExtractionPoint;
	UPROPERTY(Category = "Design Stat", EditAnywhere)
	TSubclassOf<class ADrill>		cDrillClass;
	UPROPERTY(VisibleAnywhere)
	class ADrill*					cDrill;

	FTimerHandle							cParticleTimer;
	UParticleSystemComponent*				cParticleSystem;
	
	//booleans for the drill;
	bool							bIsDrillPickedUp;
	bool							bIsDrillActivated;


	UFUNCTION()
	void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// declare overlap end function
	UFUNCTION()
	void EndOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	void EquipPrimary();
	void EquipSecondary();
	void UseDeployable();
	void StopEmitter();
};

