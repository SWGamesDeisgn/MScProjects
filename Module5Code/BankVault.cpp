// Fill out your copyright notice in the Description page of Project Settings.


#include "BankVault.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleEmitter.h"

// Sets default values
ABankVault::ABankVault()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	cCapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	RootComponent = cCapsuleComponent;

	cStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	cStaticMeshComponent->SetupAttachment(RootComponent);

	// Create audio component
	cAudioComponent = CreateDefaultSubobject<UAudioComponent>( TEXT( "AudioComponent" ) );
	cAudioComponent->SetupAttachment( RootComponent );

}

void ABankVault::DestroyVault_Implementation()
{
	cStaticMeshComponent->SetStaticMesh( cBrokenVaultMesh );
	SetActorEnableCollision( false );

	// Play sound effect when the vault is opened
	cAudioComponent->Play();
	PlayVFX();
}

void ABankVault::PlayVFX()
{
	if( cParticles )
	{
		cParticleSystem = UGameplayStatics::SpawnEmitterAttached( cParticles , RootComponent );
		GetWorldTimerManager().SetTimer( cParticleTimer , this , &ABankVault::StopVFX , 0.75f , false , 0.75f );
	}
}

void ABankVault::StopVFX()
{
	if( cParticles )
	{
		cParticleSystem->bSuppressSpawning = true;
		GetWorldTimerManager().ClearTimer( cParticleTimer );
	}
}