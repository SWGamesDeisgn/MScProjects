// Fill out your copyright notice in the Description page of Project Settings.


#include "AssaultRifle.h"

// Sets default values
AAssaultRifle::AAssaultRifle()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//default values
	Pi_AmmoCountMax = 20;
	Pf_FireRate = 9.0f;
	Pf_Damage = 42;
	Pf_ReloadSpeed = 4.0f;
	Pf_Range = 3200.0f;
	strGunName = "AssaultRifle";
	cGunType = this;
	Pe_FireModeDefinition = EFireMode::FM_FullAuto;
	afAIDamageValues.Init( 0.0f , 10 );
}

// Called when the game starts or when spawned
void AAssaultRifle::BeginPlay()
{
	Super::BeginPlay();
	ABaseGun::SetPlayerPrimaryGunType( cGunType );
	ABaseGun::SetGunValues( strGunName , Pi_AmmoCountMax , Pf_Damage , Pf_FireRate , Pf_ReloadSpeed , Pf_Range , Pe_FireModeDefinition , afAIDamageValues, Pi_AmmoReserve );
	ABaseGun::SetChildGun( cGunType );
	if (Pb_IsOnAI)
	{
		ABaseGun::SetAIGunType(cGunType);
	}
}

// Called every frame
void AAssaultRifle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

