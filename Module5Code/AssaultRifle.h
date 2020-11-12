// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseGun.h"
#include "AssaultRifle.generated.h"

UCLASS()
class PAYDAY_API AAssaultRifle : public ABaseGun
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAssaultRifle();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UPROPERTY( EditAnywhere )
		int										Pi_AmmoCountMax;
	UPROPERTY( EditAnywhere )
		float									Pf_FireRate;
	UPROPERTY( EditAnywhere )
		float									Pf_Damage;
	UPROPERTY( EditAnywhere )
		float									Pf_ReloadSpeed;
	UPROPERTY( EditAnywhere )
		float									Pf_Range;
	UPROPERTY(EditAnywhere)
		bool									Pb_IsOnAI;
	UPROPERTY( EditAnywhere , BlueprintReadWrite )
		EFireMode Pe_FireModeDefinition;
	UPROPERTY( EditAnywhere , BlueprintReadWrite )
		TArray<float>							afAIDamageValues;
	UPROPERTY( EditAnywhere , BlueprintReadWrite )
		int										Pi_AmmoReserve;
	// Called every frame
	virtual void Tick( float DeltaTime ) override;
private:
	class ABaseGun* cGunType;
	UPROPERTY( VisibleAnywhere )
		FString									strGunName;

};
