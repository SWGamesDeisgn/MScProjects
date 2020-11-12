// Fill out your copyright notice in the Description page of Project Settings.

#include "ObjectiveHighlighter.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AObjectiveHighlighter::AObjectiveHighlighter()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create static mesh to show shinning material
	cStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>( "Static Mesh" );
	cStaticMeshComponent->SetupAttachment( RootComponent );
}

void AObjectiveHighlighter::BeginPlay()
{
	Super::BeginPlay();

	// Bind event
	cPaydayGameState = GetWorld()->GetGameState<APaydayGameState>();
	if( cPaydayGameState )
	{
		cPaydayGameState->OnPaydayObjectiveChanged.AddDynamic( this, &AObjectiveHighlighter::OnPaydayObjectiveChanged );
	}

	// Apply the default value
	cStaticMeshComponent->SetVisibility( bIsDefaultHighlighted );
	if( cOwningActor )
	{
		if( cOwningActor->IsHidden() )
		{
			SetActorHiddenInGame( true );
		}
	}
}
// Needed Tick to check if the keycards had been interacted with to disable the highlighter visibility/ set them to hidden.
// This seems to be stable from my short testing runs.
void AObjectiveHighlighter::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
	if( cOwningActor )
	{
		if( cOwningActor->IsHidden() && !bHasBeenInteracted )
		{
			cStaticMeshComponent->SetVisibility( false );
			SetActorHiddenInGame( true );
			PrimaryActorTick.bCanEverTick = false;
			bHasBeenInteracted = true;
		}
	}
}

void AObjectiveHighlighter::OnPaydayObjectiveChanged( EPaydayObjectiveState eState )
{
	// Show the mesh when the new objective state is our target state
	if( eTargetState == eState )
	{
		cStaticMeshComponent->SetVisibility( true );
	}
	else
	{
		cStaticMeshComponent->SetVisibility( false );
	}
}
