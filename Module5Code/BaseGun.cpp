// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseGun.h"
#include "Components/StaticMeshComponent.h"
#include "Engine.h"
#include "PayDay/PaydayCharacter.h"
#include "Payday/Gameplay/ActorComponents/ReplicatedInteractable.h"
#include "Payday/AI/BaseAICharacter.h"
#include "Payday/Gameplay/ActorComponents/HealthComponent.h"
#include "CollisionQueryParams.h"
#include "Payday/Core/PlayerStates/PaydayPlayerState.h"
#include "Payday/Core/PlayerControllers/PaydayPlayerController.h"
#include "Payday/PaydayHUD.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleEmitter.h"
#include "Components/BoxComponent.h"
#include "Payday/Core/GameStates/PaydayGameState.h"

// Sets default values
ABaseGun::ABaseGun()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Pc_GunMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>( TEXT( "Gun Mesh" ) );
	Pc_GunMeshComponent->SetCollisionProfileName( "NoCollision" );
	RootComponent = Pc_GunMeshComponent;
	//cReplicatedComp = CreateDefaultSubobject<UReplicatedInteractable>( "ReplicatedInteractable" );
	Pc_MuzzleLocation = CreateDefaultSubobject<UBoxComponent>( TEXT( "Muzzle Location" ) );
	Pc_MuzzleLocation->SetBoxExtent( FVector( 2.0f , 2.0f , 2.0f ) );
	Pc_MuzzleLocation->SetupAttachment( RootComponent );
	Pc_MuzzleLocation->SetHiddenInGame( true );
	Pc_MuzzleLocation->SetCollisionEnabled( ECollisionEnabled::NoCollision );
	bBaseGunIsActive = false;
	iAmmoCount = 0;
	iAmmoCountMax = 0;
	fFireRate = 0.0f;
	fDamage = 0.0f;
	fReloadSpeed = 0.0f;
	strGunName = FString("");
	iAmmoReserve = 0;
	iAmmoReserveMax = 0;
	cChildGun = nullptr;
	cOwningPlayer = nullptr;
	AnimInstance = nullptr;
	cRayTarget = nullptr;
	fBaseGunRange = 500.0f;
	cRayHitResult = new FHitResult();
	v3RayStart = FVector(0.0f, 0.0f, 0.0f);
	v3RayEnd = FVector(0.0f, 0.0f, 0.0f);
	cTargetHealth = nullptr;
	bToggleFullAuto = false;
	afAIDamageValues.Init( 0.0f, 10 );
	iAIShotCount = 0;
	fAIDamageValue = 0.0f;
	cPlayerCamera = nullptr;

}

// Called when the game starts or when spawned
void ABaseGun::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
//if using an unreal timer the poll rate would be 1 / fire rate, for reloading it would be just reload time.

void ABaseGun::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
	if( cOwningPlayer || cOwningAI )
	{
		if( iAmmoCount > 0 )
		{
			if( !bReadyToFire )
			{
				// 1.0f is in relation to seconds so 1.0f is 1 second, meaning if fire rate = 2, then every .5 seconds we fire.
				if( fFireTimer <= 1.0f )
				{
					fFireTimer += DeltaTime * fFireRate;
				}
				if( fFireTimer >= 1.0f )
				{
					fFireTimer = fFireRate;
					bReadyToFire = true;
				}
			}
		}
		if( bReloading )
		{
			fReloadTimer += DeltaTime;
			if( fReloadTimer >= fReloadSpeed / 1.2f && !bHalfReloaded )
			{
				PlayInsertMagazineSFX();
				bHalfReloaded = true;

			}
			if( fReloadTimer >= fReloadSpeed )
			{
				ReloadGun();
			}
		}

		if( bToggleFullAuto )
		{
			FullAuto();
		}
	}
}

// Fire the gun based on whether it is a fully automatic or semi-automatic weapon.
// If we're full automatic then we will divert to a different function to handle the firing of the gun.
void ABaseGun::Fire( bool bFireGun )
{
	switch (eFireModeDefinition)
	{
	case EFireMode::FM_SemiAuto:
		if( bReadyToFire && iAmmoCount > 0 && !bReloading && bFireGun)
		{
			RayCast();
			fFireTimer -= fFireTimer;
			iAmmoCount--;
			Client_UpdateAmmo( iAmmoCount );
			GEngine->AddOnScreenDebugMessage( -1 , 2.0f , FColor::Green , FString::Printf( TEXT( "Ammo Count is: %d" ) , iAmmoCount ) );
			bReadyToFire = false;
			if( FireSound != nullptr )
			{
				UGameplayStatics::PlaySoundAtLocation( this , FireSound , GetActorLocation() );
			}
			Client_GunVFX_Implementation( MuzzleFlashEmitter );
			// try and play a firing animation if specified
			if( FireAnimation != nullptr )
			{
				if( AnimInstance != nullptr )
				{
					AnimInstance->Montage_Play( FireAnimation , 1.f );
				}
			}
		}
		if(  iAmmoCount == 0 && bFireGun )
		{
			PlayGunDrySFX();
		}
		break;
	case EFireMode::FM_FullAuto:
		bToggleFullAuto = bFireGun;
		break;
	default:
		break;
	}

}

void ABaseGun::RayCast()
{
	if (cChildGun)
	{
		v3RayStart = cChildGun->GetActorLocation();
		v3RayEnd = (cChildGun->GetActorForwardVector() * fBaseGunRange) + v3RayStart;
		// If owner is a Player then we change the location for the start and end point of the raycast to the location of the player camera.
		if( cOwningPlayer )
		{
			v3RayStart = cPlayerCamera->GetComponentLocation() + FVector( 0.0f , 0.0f , 0.0f );
			v3RayEnd = (cPlayerCamera->GetForwardVector() * fBaseGunRange) + v3RayStart;
		}
		// If owner is an AI then we change the location for the start and end point of the raycast
		if( cOwningAI )
		{
			v3RayStart = Pc_MuzzleLocation->GetComponentLocation();
			v3RayEnd = (cOwningAI->GetActorForwardVector() * fBaseGunRange) + v3RayStart;
		}
		if (GetWorld()->LineTraceSingleByChannel(*cRayHitResult, v3RayStart, v3RayEnd, ECC_Camera, cCollisionParams))
		{
			v3RayEnd = cRayHitResult->Location;
			cRayTarget = cRayHitResult->GetActor();
			if( cRayTarget )
			{
				cTargetHealth = Cast<UHealthComponent>( cRayTarget->FindComponentByClass<UHealthComponent>() );
				// We check for a health component so that we know if we're supposed to deal damage on impact, and also so we know what VFX to play.
				if( cTargetHealth )
				{
					DealDamage();
					Client_GunVFX( BloodSplatterEmitter );
				}
				else
				{
					Client_GunVFX( SparksEmitter );
				}
				PlayBulletImpactSFX();
				GEngine->AddOnScreenDebugMessage( -1 , 2.0f , FColor::Yellow , FString::Printf( TEXT( "Target Is: %s" ) , *cRayTarget->GetName() ) );	
			}
			DrawDebugLine(GetWorld(), v3RayStart, v3RayEnd, FColor::Red, false, 2.0f);
		}
	}
}

// Sets values based on child blueprint/class, its possible to set the BaseGun values directly in the child blueprint but they are not available in an obvious method.
void ABaseGun::SetGunValues( FString strName, int iMaxAmmo, float fBulletDamage, float fGunFireRate, float fReloadTime, float fGunRange, EFireMode eFireMode, TArray<float> AIDamageValues, int iAmmoReserveCount )
{
	iAmmoCountMax = iMaxAmmo;
	fFireRate = fGunFireRate;
	fDamage = fBulletDamage;
	fReloadSpeed = fReloadTime;
	strGunName = strName;
	fBaseGunRange = fGunRange;
	iAmmoCount = iAmmoCountMax;
	fFireTimer = 0.0f;
	fReloadTimer = 0.0f;
	iAmmoReserveMax = iAmmoReserveCount;
	iAmmoReserve = iAmmoReserveMax;
	eFireModeDefinition = eFireMode;
	afAIDamageValues = AIDamageValues;
	bValuesSet = true;
	bReadyToFire = true;
	return;
}

AActor* ABaseGun::GetRaycastTarget()
{
	return cRayTarget;
}

// We do this so the owning player/AI class knows about their weapon.
void ABaseGun::SetPlayerPrimaryGunType(ABaseGun* cGun)
{
	if(nullptr != GetParentActor() )
	{
		if( Cast<APaydayCharacter>( cGun->GetParentActor() ) )
		{
			cOwningPlayer = Cast<APaydayCharacter>( cGun->GetParentActor() );
			if( nullptr != cOwningPlayer )
			{
				cOwningPlayer->SetPlayerPrimaryGun( cGun );
				cCollisionParams.AddIgnoredActor( cOwningPlayer );
				AnimInstance = cOwningPlayer->GetArmMesh()->GetAnimInstance();
				cPlayerCamera = Cast<UCameraComponent>( cGun->GetParentActor()->FindComponentByClass<UCameraComponent>() );
				cGameState = Cast<APaydayGameState>( GetWorld()->GetGameState() );
			}
		}
	}
}

// We do this so the owning player/AI class knows about their weapon.
void ABaseGun::SetPlayerSecondaryGunType( ABaseGun* cGun )
{
	if( nullptr != GetParentActor() )
	{
		if( Cast<APaydayCharacter>( cGun->GetParentActor() ) )
		{
			cOwningPlayer = Cast<APaydayCharacter>( cGun->GetParentActor() );
			if( nullptr != cOwningPlayer )
			{
				cOwningPlayer->SetPlayerSecondaryGun( cGun );
				cCollisionParams.AddIgnoredActor( cOwningPlayer );
				AnimInstance = cOwningPlayer->GetArmMesh()->GetAnimInstance();
				cPlayerCamera = Cast<UCameraComponent>( cGun->GetParentActor()->FindComponentByClass<UCameraComponent>() );
				cGameState = Cast<APaydayGameState>( GetWorld()->GetGameState() );
			}
		}
	}
}

// We do this so the owning player/AI class knows about their weapon.
void ABaseGun::SetAIGunType( ABaseGun* cGun )
{
	if( nullptr != GetParentActor() )
	{
		if( Cast<ABaseAICharacter>( cGun->GetParentActor() ) )
		{
			cOwningAI = Cast<ABaseAICharacter>( cGun->GetParentActor() );
			if( nullptr != cOwningAI )
			{
				cOwningAI->SetAIGun( cGun );
				cCollisionParams.AddIgnoredActor( cOwningAI );	
				cGameState = Cast<APaydayGameState>( GetWorld()->GetGameState() );
			}
		}
	}
}

// Pointer to the child gun. It is never used but may be useful for something.
void ABaseGun::SetChildGun(ABaseGun* cGun)
{
	cChildGun = cGun;
}

// Set reloading to true so the reload in Tick can start its function.
void ABaseGun::Reload()
{
	if( iAmmoReserve > 0 )
	{
		if( iAmmoCount < iAmmoCountMax && !bReloading )
		{
			bReloading = true;
			GEngine->AddOnScreenDebugMessage( -1 , 2.0f , FColor::Green , FString::Printf( TEXT( "Reloading" ) ) );
			PlayRemoveMagazineSFX();
		}
	}

}

int ABaseGun::GetAmmoCount()
{
	return iAmmoCount;
}
int ABaseGun::GetReserveAmmoCount()
{
	return iAmmoReserve;
}
bool ABaseGun::CanShoot()
{
	return bReadyToFire && iAmmoCount > 0 && !bReloading;
}

// Deal damage to an actor based on whether it is a player or AI.
// I should have implemented a check to see who the owner of the gun was here to prevent friendly fire on AIs from other AIs.
void ABaseGun::DealDamage()
{
	if( cOwningPlayer )
	{
		cTargetHealth->ReduceHealth( fDamage );
	}
	if( cOwningAI )
	{
		if( afAIDamageValues[ iAIShotCount ] > 0.0f )
		{
			cTargetHealth->ReduceHealth( afAIDamageValues[ iAIShotCount ] );
			if( iAIShotCount < afAIDamageValues.Num() - 1 )
			{
				iAIShotCount++;
			}
		}
	}
}

// Our fully automatic firemode.
void ABaseGun::FullAuto()
{
	if( cGameState )
	{
		if( cGameState->GetCurrentGameState() == EPaydayGameState::GAME_END )
		{
			bToggleFullAuto = false;
		}
	}
	if( bReadyToFire && iAmmoCount > 0 && !bReloading )
	{
		RayCast();
		fFireTimer -= fFireTimer;
		iAmmoCount--;
		Client_UpdateAmmo( iAmmoCount );
		bReadyToFire = false;
		// Play a sound effect when we fire the gun.
		if( FireSound != nullptr )
		{
			UGameplayStatics::PlaySoundAtLocation( this , FireSound , GetActorLocation() );
		}
		// Play the VFX when we fire the gun.
		Client_GunVFX_Implementation( MuzzleFlashEmitter );

		// try and play a firing animation if specified
		if( FireAnimation != nullptr )
		{
			if( AnimInstance != nullptr )
			{
				AnimInstance->Montage_Play( FireAnimation , 1.f );
			}
		}
	}
	if( iAmmoCount == 0 )
	{
		PlayGunDrySFX();
		bToggleFullAuto = false;
	}
}

// This is the actual reload function that gets called from Tick when the timer completes.
void ABaseGun::ReloadGun()
{
	if( iAmmoReserve > (iAmmoCountMax - iAmmoCount) )
	{
		iAmmoReserve -= (iAmmoCountMax - iAmmoCount);
		iAmmoCount = iAmmoCountMax;
		Client_UpdateAmmo( iAmmoCount );
	}
	if( iAmmoReserve <= (iAmmoCountMax - iAmmoCount) )
	{
		iAmmoCount += iAmmoReserve;
		iAmmoReserve -= iAmmoReserve;
		Client_UpdateAmmo( iAmmoCount );
	}
	fReloadTimer -= fReloadTimer;
	bReloading = false;
	bHalfReloaded = false;
	GEngine->AddOnScreenDebugMessage( -1 , 2.0f , FColor::Green , FString::Printf( TEXT( "Reloading Finished" ) ) );
	GEngine->AddOnScreenDebugMessage( -1 , 2.0f , FColor::Green , FString::Printf( TEXT( "Ammo Reserves at: %d" ) , iAmmoReserve ) );
	GEngine->AddOnScreenDebugMessage( -1 , 2.0f , FColor::Green , FString::Printf( TEXT( "Ammo in weapon: %d" ) , iAmmoCount ) );
}

void ABaseGun::Client_UpdateAmmo_Implementation( int iNewAmmo )
{
	GEngine->AddOnScreenDebugMessage( -1 , 3.0f , FColor::Blue , FString::Printf( TEXT( "New Ammo: %d" ) , iNewAmmo ) );
	// Update the player state
	cPlayerState = Cast<APaydayPlayerState>( GetWorld()->GetFirstPlayerController()->PlayerState );
	if( cPlayerState )
	{
		cPlayerState->SetCurrentAmmo( iAmmoCount );
		cPlayerState->SetAmmoReserve( iAmmoReserve );
	}

	if( cOwningPlayer )
	{
		// Get Controller from this player and cast it to PlayerController
		cPlayerController = Cast<APaydayPlayerController>( cOwningPlayer->GetController() );
		if( cPlayerController )
		{
			// Get HUD from PlayerController
			cHUD = Cast<APaydayHUD>( cPlayerController->GetHUD() );
			if( cHUD )
			{
				cHUD->UpdateAmmo( iAmmoCount, iAmmoReserve );
			}
		}
	}
}

bool ABaseGun::IsReloading()
{
	return bReloading;
}

void ABaseGun::ResetReloadStatus()
{
	bReloading = false;
	fReloadTimer -= fReloadTimer;
}

// If MuzzleFlashEmitter has been set in the blueprint then spawn an emitter.
void ABaseGun::Client_GunVFX_Implementation( class UParticleSystem* cParticles)
{
	if( nullptr != MuzzleFlashEmitter && cParticles == MuzzleFlashEmitter)
	{
		cParticleSystem = UGameplayStatics::SpawnEmitterAttached( cParticles , Pc_MuzzleLocation );
		// The AI using the guns don't spawn emitters when we're playing in viewport, they do otherwise though.
		if( cParticleSystem )
		{
			cParticleSystem->SetRelativeScale3D( FVector( 0.5f , 0.5f , 0.5f ) );
		}
	}

	if( nullptr != BloodSplatterEmitter && cParticles == BloodSplatterEmitter )
	{
		cHitLocationParticles = UGameplayStatics::SpawnEmitterAtLocation( GetWorld() , cParticles , FTransform( GetActorForwardVector().ToOrientationRotator() , v3RayEnd, {1.0f,1.0f,1.0f} ) );
		// The AI using the guns don't spawn emitters when we're playing in viewport, they do otherwise though.
		if( cHitLocationParticles )
		{
			cHitLocationParticles->SetRelativeScale3D( FVector( 1.0f, 1.0f, 1.0f ) );
		}
	}

	if( nullptr != SparksEmitter && cParticles == SparksEmitter )
	{
		cHitLocationParticles = UGameplayStatics::SpawnEmitterAtLocation( GetWorld() , cParticles , FTransform( GetActorForwardVector().ToOrientationRotator() , v3RayEnd , { 1.0f,1.0f,1.0f } ) );
		// The AI using the guns don't spawn emitters when we're playing in viewport, they do otherwise though.
		if( cHitLocationParticles )
		{
			cHitLocationParticles->SetRelativeScale3D( FVector( 1.0f , 1.0f , 1.0f ) );
		}
	}
}

void ABaseGun::PlayRemoveMagazineSFX()
{
	if( cGunReloadRemove )
	{
		UGameplayStatics::PlaySoundAtLocation( GetWorld() , cGunReloadRemove , GetActorLocation() );
	}
}

void ABaseGun::PlayInsertMagazineSFX()
{
	if( cGunReloadInsert )
	{
		UGameplayStatics::PlaySoundAtLocation( GetWorld() , cGunReloadInsert , GetActorLocation() );
	}
}

void ABaseGun::PlayGunDrySFX()
{
	if( cGunDryFire )
	{
		UGameplayStatics::PlaySoundAtLocation( GetWorld() , cGunDryFire , GetActorLocation() );
	}
}

void ABaseGun::PlayBulletImpactSFX()
{
	if( cRayTarget )
	{
		if( cTargetHealth )
		{
			//Play SFX for Bullet Impact Flesh.
			if( cBulletImpactFlesh )
			{
				UGameplayStatics::PlaySoundAtLocation( GetWorld() , cBulletImpactFlesh , v3RayEnd );
			}
			GEngine->AddOnScreenDebugMessage( -1 , 2.0f , FColor::Green , FString::Printf( TEXT( "Playing SFX: Bullet Contact: Flesh" ) ) );

			
		}
		else
		{
			//Play SFX for Bullet Impact Environment.
			if( cBulletImpactEnvironment )
			{
				UGameplayStatics::PlaySoundAtLocation( GetWorld() , cBulletImpactFlesh , v3RayEnd );
			}
			GEngine->AddOnScreenDebugMessage( -1 , 2.0f , FColor::Green , FString::Printf( TEXT( "Playing SFX: Bullet Contact: Environment" ) ) );
			
		}
	}
}