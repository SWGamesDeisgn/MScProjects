// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "PaydayCharacter.h"
#include "PaydayProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "XRMotionControllerBase.h" // for FXRMotionControllerBase::RightHandSourceId
#include "Engine/Engine.h"
#include "Gameplay/Pickups/MoneyPickup.h"
#include "Gameplay/ActorComponents/HealthComponent.h"
#include "Gameplay/BankVault.h"
#include "Gameplay/Drill.h"
#include "Payday/Gameplay/Weapons/BaseGun.h"
#include "Payday/Gameplay/WorldInteractables/ExtractionPoint.h"
#include "Payday/Core/GameStates/PaydayGameState.h"
#include "Payday/Gameplay/ActorComponents/DetectionComponent.h"
#include "Payday/PaydayHUD.h"
#include "Components/AudioComponent.h"
#include "TimerManager.h"
#include "Payday/Core/PlayerControllers/PaydayPlayerController.h"
#include "Payday/Core/PlayerStates/PaydayPlayerState.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleEmitter.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// APaydayCharacter

APaydayCharacter::APaydayCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->SetIsReplicated( true );

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	// TODO Need to wait for full body mesh and animations
	Mesh1P->SetOnlyOwnerSee(false);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));
	Mesh1P->SetIsReplicated( true );

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(false);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);
	FP_Gun->SetIsReplicated( true );

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));
	FP_MuzzleLocation->SetIsReplicated( true );

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.

	// Create VR Controllers.
	R_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("R_MotionController"));
	R_MotionController->MotionSource = FXRMotionControllerBase::RightHandSourceId;
	R_MotionController->SetupAttachment(RootComponent);
	L_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("L_MotionController"));
	L_MotionController->SetupAttachment(RootComponent);

	// Create a gun and attach it to the right-hand VR controller.
	// Create a gun mesh component
	VR_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VR_Gun"));
	VR_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	VR_Gun->bCastDynamicShadow = false;
	VR_Gun->CastShadow = false;
	VR_Gun->SetupAttachment(R_MotionController);
	VR_Gun->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	VR_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("VR_MuzzleLocation"));
	VR_MuzzleLocation->SetupAttachment(VR_Gun);
	VR_MuzzleLocation->SetRelativeLocation(FVector(0.000004, 53.999992, 10.000000));
	VR_MuzzleLocation->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));		// Counteract the rotation of the VR gun model.

	// Uncomment the following line to turn motion controllers on by default:
	//bUsingMotionControllers = true;

	cPlayerHealthComp = CreateDefaultSubobject<UHealthComponent>( TEXT( "Player Health Component" ) );
	bIsPlayerHostile = false;
	fMoneyHeld = 0.0f;
	bPlayerIsShooting = false;

	//for detection
	cPlayerDetectionComp = CreateDefaultSubobject<UDetectionComponent>(TEXT("Player Detection Component"));

	bIsDrillPickedUp = false;
	bIsDrillActivated = false;
}

void APaydayCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
	
	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.
	if (bUsingMotionControllers)
	{
		VR_Gun->SetHiddenInGame(false, true);
		Mesh1P->SetHiddenInGame(true, true);
	}
	else
	{
		VR_Gun->SetHiddenInGame(true, true);
		Mesh1P->SetHiddenInGame(false, true);
	}

	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &APaydayCharacter::BeginOverlap);
	GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &APaydayCharacter::EndOverlap);
	bIsPlayerHostile = false;
	if( Mesh1P )
	{
		Mesh1P->SetHiddenInGame( true );
	}
	if( cPlayerPrimaryGun || cPlayerSecondaryGun )
	{
		FP_Gun->SetHiddenInGame( true );
	}
	if( GetWorld() )
	{
		cPaydayGameState = GetWorld()->GetGameState<APaydayGameState>();
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void APaydayCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &APaydayCharacter::OnFireClicked );
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &APaydayCharacter::OnFireReleased );
	PlayerInputComponent->BindAction("ReloadGun" , IE_Released , this , &APaydayCharacter::ReloadWeapon );

	// Bind use item event
	PlayerInputComponent->BindAction( "UseItem" , IE_Pressed , this , &APaydayCharacter::StartInteracting );
	PlayerInputComponent->BindAction( "UseItem" , IE_Released , this , &APaydayCharacter::StopInteracting );

	//Bind Extract event
	PlayerInputComponent->BindAction("Extract", IE_Pressed, this, &APaydayCharacter::Extract);

	// Enable touchscreen input
	EnableTouchscreenMovement(PlayerInputComponent);

	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &APaydayCharacter::OnResetVR);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &APaydayCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APaydayCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &APaydayCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &APaydayCharacter::LookUpAtRate);

	// Bind debug events for testing health component
	PlayerInputComponent->BindAction( "Debug_Health_Decrease", IE_Pressed, this, &APaydayCharacter::OnDebugHealthDecrease );
	PlayerInputComponent->BindAction( "Debug_Health_Increase", IE_Pressed, this, &APaydayCharacter::OnDebugHealthIncrease );
	PlayerInputComponent->BindAction( "Debug_Print_All_Health", IE_Pressed, this, &APaydayCharacter::OnDebugPrintAllPlayersHealth );

	PlayerInputComponent->BindAction( "PrimaryWeapon" , IE_Pressed , this , &APaydayCharacter::EquipPrimary );
	PlayerInputComponent->BindAction( "SecondaryWeapon" , IE_Pressed , this , &APaydayCharacter::EquipSecondary );
	PlayerInputComponent->BindAction( "DeployEquipment" , IE_Pressed , this , &APaydayCharacter::UseDeployable );
}

void APaydayCharacter::OnFireClicked()
{
	// Trigger the RPC function to notify the server fire for you
	if( cPaydayGameState )
	{
		if( cPlayerCurrentGun && !cPlayerCurrentGun->IsHidden() && cPlayerHealthComp->GetPlayerLives() > 0 && cPaydayGameState->GetCurrentGameState() != EPaydayGameState::GAME_END )
		{
			cPlayerCurrentGun->Fire( true );
			bPlayerIsShooting = true;
		}
	}

}

void APaydayCharacter::OnFireReleased()
{
	if( cPlayerCurrentGun )
	{
		cPlayerCurrentGun->Fire( false );
		bPlayerIsShooting = false;
	}
}

void APaydayCharacter::OnFire_Implementation()
{
	if( cPlayerCurrentGun )
	{
		// Notify the client to play animation and SFX if they are able to fire
		if( cPlayerPrimaryGun->CanShoot() )
		{
			Client_OnFireFinished();
		}
		cPlayerCurrentGun->Fire(true);
	}
	else
	{
		// try and fire a projectile
		if( ProjectileClass != NULL )
		{
			UWorld* const World = GetWorld();
			if( World != NULL )
			{
				if( bUsingMotionControllers )
				{
					const FRotator SpawnRotation = VR_MuzzleLocation->GetComponentRotation();
					const FVector SpawnLocation = VR_MuzzleLocation->GetComponentLocation();
					World->SpawnActor<APaydayProjectile>( ProjectileClass , SpawnLocation , SpawnRotation );
				}
				else
				{
					const FRotator SpawnRotation = GetControlRotation();
					// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
					const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector( GunOffset );

					//Set Spawn Collision Handling Override
					FActorSpawnParameters ActorSpawnParams;
					ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

					// spawn the projectile at the muzzle
					World->SpawnActor<APaydayProjectile>( ProjectileClass , SpawnLocation , SpawnRotation , ActorSpawnParams );
				}
			}
		}
	}
}

void APaydayCharacter::Client_OnFireFinished_Implementation()
{
	/* Moved to the BaseGun class.
	// Play sound effect on local because client can't hear server side sounds
	// try and play the sound if specified
	if( FireSound != NULL )
	{
		UGameplayStatics::PlaySoundAtLocation( this, FireSound, GetActorLocation() );
	}

	// try and play a firing animation if specified
	if( FireAnimation != NULL )
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if( AnimInstance != NULL )
		{
			AnimInstance->Montage_Play( FireAnimation, 1.f );
		}
	}
	*/
}

void APaydayCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void APaydayCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		OnFire();
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void APaydayCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	TouchItem.bIsPressed = false;
}

//Commenting this section out to be consistent with FPS BP template.
//This allows the user to turn without using the right virtual joystick

//void APaydayCharacter::TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location)
//{
//	if ((TouchItem.bIsPressed == true) && (TouchItem.FingerIndex == FingerIndex))
//	{
//		if (TouchItem.bIsPressed)
//		{
//			if (GetWorld() != nullptr)
//			{
//				UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
//				if (ViewportClient != nullptr)
//				{
//					FVector MoveDelta = Location - TouchItem.Location;
//					FVector2D ScreenSize;
//					ViewportClient->GetViewportSize(ScreenSize);
//					FVector2D ScaledDelta = FVector2D(MoveDelta.X, MoveDelta.Y) / ScreenSize;
//					if (FMath::Abs(ScaledDelta.X) >= 4.0 / ScreenSize.X)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.X * BaseTurnRate;
//						AddControllerYawInput(Value);
//					}
//					if (FMath::Abs(ScaledDelta.Y) >= 4.0 / ScreenSize.Y)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.Y * BaseTurnRate;
//						AddControllerPitchInput(Value);
//					}
//					TouchItem.Location = Location;
//				}
//				TouchItem.Location = Location;
//			}
//		}
//	}
//}

void APaydayCharacter::MoveForward(float Value)
{
	if( cPaydayGameState )
	{
		if( cPaydayGameState )
		{
			if( cPaydayGameState->GetCurrentGameState() == EPaydayGameState::GAME_END || cPlayerHealthComp->IsPlayerDead() )
			{
				Value = 0.0f;
			}
		}
	}
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void APaydayCharacter::MoveRight(float Value)
{
	if( cPaydayGameState )
	{
		if( cPaydayGameState->GetCurrentGameState() == EPaydayGameState::GAME_END || cPlayerHealthComp->IsPlayerDead() )
		{
			Value = 0.0f;
		}
	}

	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void APaydayCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void APaydayCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

bool APaydayCharacter::EnableTouchscreenMovement(class UInputComponent* PlayerInputComponent)
{
	if (FPlatformMisc::SupportsTouchInput() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		PlayerInputComponent->BindTouch(EInputEvent::IE_Pressed, this, &APaydayCharacter::BeginTouch);
		PlayerInputComponent->BindTouch(EInputEvent::IE_Released, this, &APaydayCharacter::EndTouch);

		//Commenting this out to be more consistent with FPS BP template.
		//PlayerInputComponent->BindTouch(EInputEvent::IE_Repeat, this, &APaydayCharacter::TouchUpdate);
		return true;
	}
	
	return false;
}

void APaydayCharacter::SetTargetInteractive(UReplicatedInteractable* cTargetItem)
{
	pCurrentInteractable = cTargetItem;
	GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Green, TEXT( "Gonna set the interactable to player" ) );
	// GetOwner from the replicated interactable means to get the parent actor, then set the owner to this player controller to grant the RPC access right
	pCurrentInteractable->GetOwner()->SetOwner( this );
}

void APaydayCharacter::StartInteracting()
{
	// Update the player state
	if( !cPlayerState )
	{
		cPlayerState = Cast<APaydayPlayerState>( GetWorld()->GetFirstPlayerController()->PlayerState );
	}
	
	bIsInteracting = true;
	GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Green, TEXT( "Is Interacting" ) );
	// If the current interactable not null, it will trigger the RPC to handle the interact event on the server-side
	/* Deprecated
	if( pCurrentInteractable )
	{
		GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Green, TEXT( "1" ) );
		pCurrentInteractable->InteractItem();

		// Set to nullptr after player trigger the interactable
		// TODO - Might need to implement a validation for InteractItem RPC instead of setting to nullptr
		pCurrentInteractable = nullptr;
	}
	*/
	if( cPaydayGameState )
	{
		if( cPaydayGameState->GetArePlayersHostile() )
		{

			//interact with bank vault
			if (bIsDrillPickedUp && !bIsDrillActivated )
			{
				if (cBankVault)
				{
					// Call the server side to deploy drill
					Server_DeployDrill();
					FActorSpawnParameters SpawnInfo;
					ADrill* spawnedDrill = GetWorld()->SpawnActor<ADrill>( cDrillClass, GetActorLocation(), GetActorRotation(), SpawnInfo );
					spawnedDrill->SetBankVault( cBankVault );
					bIsDrillActivated = true;
				}
			}

			//interact with the extraction point
			if( cExtractionPoint )
			{
				if( cPlayerState && cPlayerState->GetCurrentMoney() != 0 )
				{
					if( cParticles )
					{
						cParticleSystem = UGameplayStatics::SpawnEmitterAtLocation( GetWorld() , cParticles , FTransform( FRotator( 0.0f , 0.0f , 0.0f ) , (GetActorForwardVector() * 5.0f + GetActorLocation()) , { 1.0f,1.0f,1.0f } ) );
						GetWorldTimerManager().SetTimer( cParticleTimer , this , &APaydayCharacter::StopEmitter , 1.0f , true , 1.0f );
					}
					cExtractionPoint->DepositMoney( cPlayerState->GetCurrentMoney() );
					cPlayerState->SetCurrentMoney( 0 );
				}
				/* swapped to using the PlayerState to manage the money instead, functionality is identical though.
				if( fMoneyHeld != 0.0f )
				{
					cExtractionPoint->DepositMoney( fMoneyHeld );
					fMoneyHeld = 0.0f;
				}
				*/
			}
			if (cDrill)
			{
				if (!bIsDrillPickedUp)
				{
					bIsDrillPickedUp = true;
					GetWorld()->DestroyActor(cDrill);

					GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("Drill Picked Up"));

					// Play interaction sound effect
					UGameplayStatics::PlaySoundAtLocation( GetWorld(), ItemCollectedSound, GetActorLocation() );

					// Update UI
					if (cHUD)
					{
						cHUD->SetObjectiveMessage( EPaydayObjectiveState::OPEN_THE_VAULT );
					}

					// Update the game objective flag
					cPaydayGameState->SetCurrentObjective( EPaydayObjectiveState::OPEN_THE_VAULT );
				}
				if (bIsDrillPickedUp && bIsDrillActivated)
				{
					cDrill->StartActivatingDrill();
				}
			}
		}
	}
}

void APaydayCharacter::StopInteracting()
{
	bIsInteracting = false;
	GEngine->AddOnScreenDebugMessage( -1 , 3.0f , FColor::Green , TEXT( "Stopped Interacting" ) );

	if (cPaydayGameState)
	{
		if (cPaydayGameState->GetArePlayersHostile())
		{
			if (cDrill)
			{
				if (bIsDrillPickedUp && bIsDrillActivated)
				{
					cDrill->StopActivatingDrill();
				}
			}
		}
	}
}

void APaydayCharacter::PlayDrillingSFX()
{
	DrillAudioComponent->SetPaused( false );
}

void APaydayCharacter::StopDrillingSFX()
{
	DrillAudioComponent->SetPaused( true );
}

void APaydayCharacter::Extract()
{
	if (cExtractionPoint && bIsPlayerHostile)
	{
		cExtractionPoint->Extract(GetName(),fMoneyHeld);
	}
}

UHealthComponent* APaydayCharacter::GetHealthComp()
{
	if( cPlayerHealthComp != nullptr )
	{
		return cPlayerHealthComp;
	}
	return nullptr;
}
// To Do Make GetPlayerInteracting, SetPlayerHasKey and GetPlayerHasKey network friendly
bool APaydayCharacter::GetPlayerInteracting()
{
	return bIsInteracting;
}

void APaydayCharacter::SetPlayerHasKey( bool bKey )
{
	bHasKeycard = bKey;
}

bool APaydayCharacter::GetPlayerHasKey()
{
	return bHasKeycard;
}

void APaydayCharacter::OnDebugHealthIncrease()
{
	GetHealthComp()->ReplenishHealth( 10.0f );
	GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Green, FString::Printf( TEXT( "Increased health - Current Health: %.f" ), GetHealthComp()->GetPlayerHealth() ) );
}

void APaydayCharacter::OnDebugHealthDecrease()
{
	GetHealthComp()->ReduceHealth( 10.0f );
	GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Green, FString::Printf( TEXT( "Decrease health - Current Health: %.f" ), GetHealthComp()->GetPlayerHealth() ) );
}

void APaydayCharacter::OnDebugPrintAllPlayersHealth()
{
	GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Green, FString::Printf( TEXT( "Player Count: %d" ), GetWorld()->GetNumPlayerControllers() ) );

	for( FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator )
	{
		// Get player controller from iterator
		APlayerController* cPlayerController = Iterator->Get();
		APaydayCharacter* cCharacter = Cast<APaydayCharacter>( cPlayerController->GetCharacter() );
		if( cCharacter )
		{
			UHealthComponent* cHealthComponent = cCharacter->GetHealthComp();
			if( cHealthComponent )
			{
				GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Green,
					FString::Printf( TEXT( "%s - Current Health: %.f, Lives: %d" ), *cPlayerController->GetName(), cHealthComponent->GetPlayerHealth(), cHealthComponent->GetPlayerLives() ) );
			}
		}
	}
	
	// PrintAllPlayersHealth();
}

void APaydayCharacter::PrintAllPlayersHealth_Implementation()
{
	GEngine->AddOnScreenDebugMessage( -1, 2.0f, FColor::Blue, TEXT( "Going to show all players health from server..." ) );
	for( FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator )
	{
		// Get player controller from iterator
		APlayerController* cPlayerController = Iterator->Get();		
		APaydayCharacter* cCharacter = Cast<APaydayCharacter>( cPlayerController->GetCharacter() );
		if( cCharacter )
		{
			UHealthComponent* cHealthComponent = cCharacter->GetHealthComp();
			if( cHealthComponent )
			{
				GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Green, 
					FString::Printf( TEXT( "%s - Current Health: %.f, Lives: %d" ), *cPlayerController->GetName(), cHealthComponent->GetPlayerHealth(), cHealthComponent->GetPlayerLives() ) );
			}
		}
	}
}

void APaydayCharacter::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

	// Check for null and ensure we aren't overlapping ourselves
	if (nullptr != OtherActor && nullptr != OtherComp && this != OtherActor)
	{
		ABankVault* cOtheVault = Cast<ABankVault>(OtherActor);
		if (cOtheVault)
		{
			cBankVault = cOtheVault;
		}

		AExtractionPoint* cExePoint = Cast<AExtractionPoint>(OtherActor);
		if (cExePoint)
		{
			cExtractionPoint = cExePoint;
		}

		ADrill* cOtherDrill = Cast<ADrill>(OtherActor);
		if (cOtherDrill)
		{
			cDrill = cOtherDrill;
		}
	}
}

void APaydayCharacter::EndOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// Check for null and ensure we aren't overlapping ourselves
	if (nullptr != OtherActor && nullptr != OtherComp && this != OtherActor)
	{
		if (cBankVault)
		{
			if (cBankVault->GetName().Equals(OtherActor->GetName()))
			{
				cBankVault = nullptr;
			}
		}

		if (cExtractionPoint)
		{
			if (cExtractionPoint->GetName().Equals(OtherActor->GetName()))
			{
				cExtractionPoint = nullptr;
			}
		}

		if (cDrill)
		{
			if (cDrill->GetName().Equals(OtherActor->GetName()))
			{
				cDrill = nullptr;
			}
		}
	}
}

void APaydayCharacter::SetPlayerPrimaryGun( class ABaseGun* cGun )
{
	cPlayerPrimaryGun = cGun;
	cPlayerCurrentGun = cPlayerPrimaryGun;
	cPlayerPrimaryGun->SetActorHiddenInGame( true );
}
void APaydayCharacter::SetPlayerSecondaryGun( class ABaseGun* cGun )
{
	cPlayerSecondaryGun = cGun;
	cPlayerSecondaryGun->SetActorHiddenInGame( true );
}

bool APaydayCharacter::IsPlayerHostile()
{
	return bIsPlayerHostile;
}

void APaydayCharacter::ReloadWeapon()
{
	if( cPlayerCurrentGun )
	{
		cPlayerCurrentGun->Reload();
		//RPC Function
		//Server_ReloadGun();
	}
}

void APaydayCharacter::Server_ReloadGun_Implementation()
{
	cPlayerPrimaryGun->Reload();
}

void APaydayCharacter::Server_DeployDrill_Implementation()
{
	GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Green, TEXT( "Bank Vault Intreaction" ) );
	//FActorSpawnParameters SpawnInfo;
	//ADrill* spawnedDrill = GetWorld()->SpawnActor<ADrill>( cDrillClass, GetActorLocation(), GetActorRotation(), SpawnInfo );
	//spawnedDrill->SetBankVault( cBankVault );
}

//If the player isn't trying to fire a gun and they are flagged as hostile then switch their current weapon to the primary weapon.
void APaydayCharacter::EquipPrimary()
{
	if( cPlayerCurrentGun->IsReloading() )
	{
		cPlayerCurrentGun->ResetReloadStatus();
	}
	if( !cPlayerHealthComp->IsPlayerDead() )
	{
		if( (cPaydayGameState && cPlayerCurrentGun->CanShoot()) || (!cPlayerCurrentGun->IsReloading() && cPlayerCurrentGun->GetAmmoCount() == 0))
		{
			if( !bPlayerIsShooting && cPaydayGameState->GetArePlayersHostile() )
			{
				if( cPlayerPrimaryGun && cPlayerCurrentGun != cPlayerPrimaryGun )
				{
					cPlayerCurrentGun = cPlayerPrimaryGun;
					cPlayerPrimaryGun->SetActorHiddenInGame( false );
					if( cPlayerSecondaryGun )
					{
						cPlayerSecondaryGun->SetActorHiddenInGame( true );
					}
					if( cHUD )
					{
						cHUD->UpdateAmmo( cPlayerCurrentGun->GetAmmoCount() , cPlayerCurrentGun->GetReserveAmmoCount() );
					}
				}
			}
		}
	}
}

//If the player isn't trying to fire a gun and they are flagged as hostile then switch their current weapon to the secondary weapon.
void APaydayCharacter::EquipSecondary()
{
	if( cPlayerCurrentGun->IsReloading() )
	{
		cPlayerCurrentGun->ResetReloadStatus();
	}
	if( ( cPaydayGameState && cPlayerCurrentGun->CanShoot() ) || ( !cPlayerCurrentGun->IsReloading() && cPlayerCurrentGun->GetAmmoCount() == 0 ) )
	{
		if( !bPlayerIsShooting && cPaydayGameState->GetArePlayersHostile() )
		{
			if( cPlayerSecondaryGun && cPlayerCurrentGun != cPlayerSecondaryGun )
			{
				cPlayerCurrentGun = cPlayerSecondaryGun;
				cPlayerSecondaryGun->SetActorHiddenInGame( false );
				if( cPlayerPrimaryGun )
				{
					cPlayerPrimaryGun->SetActorHiddenInGame( true );
				}
				if( cHUD )
				{
					cHUD->UpdateAmmo( cPlayerCurrentGun->GetAmmoCount() , cPlayerCurrentGun->GetReserveAmmoCount() );
				}
			}
		}
	}
}

// returns the player's arm mesh for animation
USkeletalMeshComponent* APaydayCharacter::GetArmMesh()
{
	return Mesh1P;
}

void APaydayCharacter::SetMoney(float fMoney)
{
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Purple, TEXT("Money Recieved = ") + FString::SanitizeFloat(fMoney));
	fMoneyHeld = fMoney;
}

//Equips the mask if not already equipped allowing the player to fire their weapons and complete the heist.
void APaydayCharacter::UseDeployable()
{
	if( !bIsPlayerHostile && nullptr != cPlayerCurrentGun )
	{
		// Update the player flags and show the gun mesh
		bIsPlayerHostile = true;
		cPlayerCurrentGun->SetActorHiddenInGame( false );

		// Update the game state to let other components that will need the change state event to handle extra logic
		APaydayGameState* cGameState = GetWorld()->GetGameState<APaydayGameState>();
		if( cGameState )
		{
			cGameState->SetPlayersAreHostile( bIsPlayerHostile );
		}

		// Update the HUD to show the ammo count
		cThisPaydayController = Cast<APaydayPlayerController>( this->GetController() );
		if( cThisPaydayController )
		{
			// Get HUD from PlayerController
			cHUD = Cast<APaydayHUD>( cThisPaydayController->GetHUD() );
			if( cHUD )
			{
				cHUD->UpdateAmmo( cPlayerCurrentGun->GetAmmoCount() , cPlayerCurrentGun->GetReserveAmmoCount() );
			}
		}

		// Notify all AI spawners to spawn AI enemies
		TArray<UActorComponent*> cDetectionComponentList = GetComponentsByClass( UDetectionComponent::StaticClass() );
		if( cDetectionComponentList.Num() > 0 )
		{
			UDetectionComponent* cDetectionComponent = Cast<UDetectionComponent>( cDetectionComponentList[ 0 ] );
			if( cDetectionComponent )
			{
				cDetectionComponent->PlayerCompletlyDetected();
			}
		}
	}
	if( bIsPlayerHostile )
	{
		// Deploy something.
		return;
	}

}

void APaydayCharacter::PlayerHasDied()
{
	if( cPlayerHealthComp->IsPlayerDead() )
	{
		if( !bIsPlayerHostile )
		{
			UseDeployable();
		}
		if( cPlayerPrimaryGun && cPlayerSecondaryGun )
		{
			bPlayerIsShooting = false;
			cPlayerCurrentGun->Fire( false );
			cPlayerCurrentGun->ResetReloadStatus();
			if( cPlayerSecondaryGun && cPlayerCurrentGun != cPlayerSecondaryGun )
			{
				cPlayerCurrentGun = cPlayerSecondaryGun;
				cPlayerSecondaryGun->SetActorHiddenInGame( false );
				if( cPlayerPrimaryGun )
				{
					cPlayerPrimaryGun->SetActorHiddenInGame( true );
				}
				cThisPaydayController = Cast<APaydayPlayerController>( this->GetController() );
				if( cThisPaydayController )
				{
					// Get HUD from PlayerController
					cHUD = Cast<APaydayHUD>( cThisPaydayController->GetHUD() );
					if( cHUD )
					{
						cHUD->UpdateAmmo( cPlayerCurrentGun->GetAmmoCount() , cPlayerCurrentGun->GetReserveAmmoCount() );
					}
				}
			}			
		}
	}
}

void APaydayCharacter::Jump()
{
	if( cPaydayGameState )
	{
		if( cPaydayGameState->GetCurrentGameState() != EPaydayGameState::GAME_END || !cPlayerHealthComp->IsPlayerDead() )
		{
			bPressedJump = true;
			JumpKeyHoldTime = 0.0f;
		}
	}
}

void APaydayCharacter::StopEmitter()
{
	if( cParticles )
	{
		cParticleSystem->bSuppressSpawning = true;
		GetWorldTimerManager().ClearTimer( cParticleTimer );
	}
}