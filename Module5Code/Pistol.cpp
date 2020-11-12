// Fill out your copyright notice in the Description page of Project Settings.


#include "Pistol.h"

// Sets default values
APistol::APistol()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//default values
	Pi_AmmoCountMax = 12;
	Pf_FireRate = 2.5f;
	Pf_Damage = 25.0f;
	Pf_ReloadSpeed = 1.2f;
	Pf_PistolRange = 1600.0f;
	strGunName = "Pistol";
	cGunType = this;
	Pe_FireModeDefinition = EFireMode::FM_SemiAuto;
	afAIDamageValues.Init( 0.0f, 10 );
}

// Called when the game starts or when spawned
void APistol::BeginPlay()
{
	Super::BeginPlay();
	ABaseGun::SetPlayerSecondaryGunType( cGunType );
	ABaseGun::SetGunValues(strGunName, Pi_AmmoCountMax, Pf_Damage, Pf_FireRate, Pf_ReloadSpeed, Pf_PistolRange, Pe_FireModeDefinition, afAIDamageValues, Pi_AmmoReserve );
	ABaseGun::SetChildGun( cGunType );
	if (Pb_IsOnAI)
	{
		ABaseGun::SetAIGunType(cGunType);
	}

}

// Called every frame
void APistol::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
