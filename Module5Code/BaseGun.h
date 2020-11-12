// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseGun.generated.h"

UENUM(BlueprintType)
enum class EFireMode: uint8
{
	FM_SemiAuto UMETA(DisplayName = "Semi-Automatic"),
	FM_FullAuto UMETA(DisplayName = "Full-Auto"),
};

UCLASS()
class PAYDAY_API ABaseGun : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseGun();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	UPROPERTY( EditAnywhere, Category = "Base Gun Config" )
		class UStaticMeshComponent*			Pc_GunMeshComponent;
	/** Sound to play each time we fire */
	UPROPERTY( EditAnywhere , BlueprintReadWrite )
		class USoundBase*		FireSound;
	/** AnimMontage to play each time we fire */
	UPROPERTY( EditAnywhere , BlueprintReadWrite )
		class UAnimMontage*		FireAnimation;
	UPROPERTY( EditAnywhere )
		class USoundBase* cGunReloadInsert;
	UPROPERTY( EditAnywhere )
		class USoundBase* cGunReloadRemove;
	UPROPERTY( EditAnywhere )
		class USoundBase* cGunDryFire;
	UPROPERTY( EditAnywhere )
		class USoundBase* cBulletImpactEnvironment;
	UPROPERTY( EditAnywhere )
		class USoundBase* cBulletImpactFlesh;

	// VFX
	UPROPERTY( EditAnywhere , BlueprintReadWrite )
		class UParticleSystem*	MuzzleFlashEmitter;
	UPROPERTY( EditAnywhere , BlueprintReadWrite )
		class UParticleSystem*	SparksEmitter;
	UPROPERTY( EditAnywhere , BlueprintReadWrite )
		class UParticleSystem*	BloodSplatterEmitter;
	/** Locator used to specify the location of the muzzle of the barrel. */
	UPROPERTY( EditAnywhere , BlueprintReadWrite )
		class UBoxComponent*	Pc_MuzzleLocation;

	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void SetGunValues( FString strName, int iMaxAmmo, float fBulletDamage, float fGunFireRate, float fReloadTime, float fGunRange, EFireMode eFireMode, TArray<float> afAIDamageValues, int iAmmoReserveCount );
	virtual void RayCast();
	//Returns the first actor hit by the weapon
	AActor* GetRaycastTarget();
	void SetPlayerPrimaryGunType(ABaseGun* cGun);
	void SetPlayerSecondaryGunType(ABaseGun* cGun);
	void SetAIGunType(ABaseGun* cGun);
	void SetChildGun(ABaseGun* cGun);
	void Fire(bool bFireGun);
	void Reload();
	int  GetAmmoCount();
	int  GetReserveAmmoCount();
	bool IsReloading();

	/**
	 * \brief Get the current gun is cooling down or not
	 * \return cooldown flag
	 */
	bool CanShoot();
	void ResetReloadStatus();
private:
	// Gun variables
	EFireMode								eFireModeDefinition;
	bool									bBaseGunIsActive;
	int										iAmmoCount;
	int										iAmmoCountMax;
	float									fFireRate;
	float									fDamage;
	float									fReloadSpeed;
	float									fBaseGunRange;
	FString									strGunName;
	int										iAmmoReserve;
	int										iAmmoReserveMax;

	//pointer to child gun
	class ABaseGun*							cChildGun;

	// Owning Actor
	class APaydayCharacter*					cOwningPlayer;
	class ABaseAICharacter*					cOwningAI;
	class UAnimInstance*					AnimInstance;
	UPROPERTY(VisibleAnywhere)
	class UCameraComponent*					cPlayerCamera;
	//Raycast variables
	UPROPERTY( VisibleAnywhere, Category = "Base Gun Debug Info" )
		class AActor*						cRayTarget;
	FCollisionQueryParams					cCollisionParams;
	FHitResult*								cRayHitResult;
	FVector									v3RayStart;
	FVector									v3RayEnd;
	class UHealthComponent*					cTargetHealth;
	//Timer variables
	FTimerHandle							cBaseGunReloadTimer;
	bool									bTimerPause;
	bool									bValuesSet;
	float									fFireTimer;
	float									fReloadTimer;
	bool									bReadyToFire;
	bool									bReloading;
	bool									bHalfReloaded;
	UPROPERTY( VisibleAnywhere )
		class UReplicatedInteractable*		cReplicatedComp;
	bool									bIsGunFiring;
	bool									bToggleFullAuto;
	TArray<float>							afAIDamageValues;
	int										iAIShotCount;
	float									fAIDamageValue;

	// VFX
	class UParticleSystemComponent*			cParticleSystem;
	class UParticleSystemComponent*			cHitLocationParticles;

	// HUD Updating

	class APaydayPlayerState*				cPlayerState;
	class APaydayPlayerController*			cPlayerController;
	class APaydayHUD*						cHUD;

	class APaydayGameState*					cGameState;

	void DealDamage();
	void FullAuto();
	void ReloadGun();
	void PlayRemoveMagazineSFX();
	void PlayInsertMagazineSFX();
	void PlayGunDrySFX();
	void PlayBulletImpactSFX();

	UFUNCTION( Client , Reliable )
	void Client_UpdateAmmo( int iNewAmmo );
	void Client_UpdateAmmo_Implementation( int iNewAmmo );
	UFUNCTION( Client , Reliable )
	void Client_GunVFX( class UParticleSystem* cParticles );
	void Client_GunVFX_Implementation( class UParticleSystem* cParticles );
};
