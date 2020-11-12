// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameTimer.generated.h"

UCLASS()
class PAYDAY_API AGameTimer : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGameTimer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	// Pause the timer.
	void						PauseTimer();
	// Resume the timer
	void						ResumeTimer();


	UPROPERTY( EditAnywhere , Category = "Debugging" )
		bool					bPause;
	UPROPERTY( EditAnywhere , Category = "Debugging" )
		bool					bResume;
private:
	// Timer Handle to store our timer.
	FTimerHandle				cGameTimerHandle;
	int							iSeconds;

	class APaydayPlayerController*		cPlayerController;
	class APaydayHUD*					cHUD;
	class APaydayGameState*				cPaydayGameState;

	// The function which manages all the timer intervals.
	void						UpdateTimer();
};
