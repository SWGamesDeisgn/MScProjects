// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BankVault.generated.h"

UCLASS()
class PAYDAY_API ABankVault : public AActor
{
	GENERATED_BODY()
	
public:

	UPROPERTY( EditAnywhere )
	USoundBase* cOpenedSound;
	
	UPROPERTY( EditAnywhere )
	UParticleSystem* cParticles;

	// Sets default values for this actor's properties
	ABankVault();

	/**
	 * \brief Notify all clients that the vault has been destroyed
	 */
	UFUNCTION( NetMulticast, Reliable )
	void DestroyVault();
	void DestroyVault_Implementation();

private:

	UPROPERTY(VisibleAnywhere, Category = "Capsule Component", meta = (AllowPrivateAccess = "true"))
		class UCapsuleComponent* cCapsuleComponent;

	UPROPERTY(VisibleAnywhere, Category = "Static Mesh", meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent* cStaticMeshComponent;

	UPROPERTY( VisibleAnywhere, Category = "SFX Settings", meta = ( AllowPrivateAccess = "true" ) )
		class UAudioComponent* cAudioComponent;
	
	UPROPERTY(EditAnywhere, Category = "Static Mesh", DisplayName = "Mesh When Destroyed", meta = (AllowPrivateAccess = "true"))
		UStaticMesh* cBrokenVaultMesh;
	
	FTimerHandle							cParticleTimer;
	UParticleSystemComponent*				cParticleSystem;

	void PlayVFX();
	void StopVFX();
};
