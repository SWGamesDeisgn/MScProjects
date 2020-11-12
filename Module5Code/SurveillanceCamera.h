// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SurveillanceCamera.generated.h"

UCLASS()
class PAYDAY_API ASurveillanceCamera : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASurveillanceCamera();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UPROPERTY( EditAnywhere, Category = "Surveillance Camera Config" )
		class UStaticMeshComponent*			Pc_SurveillanceCameraMesh;
	UPROPERTY( EditAnywhere, Category = "Surveillance Camera Config" )
		class UStaticMeshComponent*			Pc_FieldOfVisionMesh;
	/*UPROPERTY( EditAnywhere, Category = "AI Spawn Point Config" )
		TSubclassOf<AActor> Pc_WaveAISpawnerClass;*/
	/**
	* Value of the speed at which the Camera will poll the player's position in seconds i.e 0.25 is 1/4 of a second
	*/
	UPROPERTY( EditAnywhere, Category = "Surveillance Camera Config", meta = ( ClampMin = 0.25f ) )
		float								Pf_SurveillancePollingRate;
	// Called every frame
	virtual void Tick( float DeltaTime ) override;
	void SetSurveillanceCameraActiveState( bool bState );

private:
	bool									bCameraIsActive;
	bool									bPlayerDetected;
	UPROPERTY( VisibleAnywhere, Category = "Surveillance Camera Debug Info" )
		class AActor*						cRayTarget;
	class APaydayGameMode*					cGameMode;
	FHitResult*								cRayHitResult;
	FVector									v3RayStart;
	FVector									v3RayEnd;
	FTimerHandle							cSurveillanceTimer;
	bool									bTimerPause;
	class APaydayCharacter*					cPlayerController;
	UPROPERTY( VisibleAnywhere, Category = "Surveillance Camera Debug Info" )
		TArray<APaydayCharacter*>			aPlayerArray;
	/*UPROPERTY( VisibleAnywhere, Category = "AI Spawning Points" )
		TArray<AActor*>						aAISpawningPointArray;*/

	// Old code: will fire a single raycast at the location provided on collision channel ECC_Camera
	void RayCast( FVector PlayerLocation );
	// Repeating because it is called by a timer
	void RepeatingRayCast();
	UFUNCTION()
		void OnOverlapBegin( UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult );
	UFUNCTION()
		void OnOverlapEnd( UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex );
	void AddToPlayerArray( APaydayCharacter* Player );
	void RemoveFromPlayerArray( APaydayCharacter* Player );

	void UpdatePlayerDetection(APaydayCharacter* Player, bool bIsDetected);

	/*
	 * RPC Functions
	 */
	/**
	 * \brief Notify the host the player is detected
	 */
	 /*UFUNCTION( Server, Reliable )
	 void Server_PlayerIsDetected();
	 void Server_PlayerIsDetected_Implementation();*/

	/*float fTime;
	float fMaxTime;

	class APaydayHUD*			PcGameHUD;*/
};