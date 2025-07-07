// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/GeoPin.h"

// Sets default values
AGeoPin::AGeoPin()
{
        // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
        PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void AGeoPin::BeginPlay()
{
        Super::BeginPlay();
}

// Called every frame
void AGeoPin::Tick(float DeltaTime)
{
        Super::Tick(DeltaTime);
}
