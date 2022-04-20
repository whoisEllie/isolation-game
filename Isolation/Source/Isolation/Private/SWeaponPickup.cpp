// Fill out your copyright notice in the Description page of Project Settings.


#include "SWeaponPickup.h"
#include "SWeaponBase.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ASWeaponPickup::ASWeaponPickup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	MainMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainMesh"));
	MainMesh->SetupAttachment(RootComponent);

	BarrelAttachment = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BarrelAttachment"));
	BarrelAttachment->SetupAttachment(MainMesh);

	MagazineAttachment = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MagazineAttachment"));
	MagazineAttachment->SetupAttachment(MainMesh);
	
	SightsAttachment = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SightsAttachment"));
	SightsAttachment->SetupAttachment(MainMesh);

	StockAttachment = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StockAttachment"));
	StockAttachment->SetupAttachment(MainMesh);
	
	GripAttachment = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GripAttachment"));
	GripAttachment->SetupAttachment(MainMesh);
}

// Called when the game starts or when spawned
void ASWeaponPickup::BeginPlay()
{
	Super::BeginPlay();
	SpawnAttachmentMesh();
	
	if (!bStatic)
	{
		MainMesh->SetSimulatePhysics(true);
	}
}

void ASWeaponPickup::SpawnAttachmentMesh()
{
	ASWeaponBase* WeaponVariables =  WeaponReference.GetDefaultObject();
	const FWeaponData* WeaponData = WeaponDataTable->FindRow<FWeaponData>(FName(WeaponVariables->DataTableNameRef), FString(WeaponVariables->DataTableNameRef), true);

	if (WeaponData->bHasAttachments)
	{
		if (AttachmentsDataTable)
		{
			for (FName RowName : AttachmentArray)
			{
				const FAttachmentData* AttachmentData = AttachmentsDataTable->FindRow<FAttachmentData>(RowName, RowName.ToString(), true);
				
				if (AttachmentData)
				{
					if (AttachmentData->AttachmentType == EAttachmentType::Barrel)
					{
						BarrelAttachment->SetStaticMesh(AttachmentData->PickupMesh);
					}
					else if (AttachmentData->AttachmentType == EAttachmentType::Magazine)
					{
						MagazineAttachment->SetStaticMesh(AttachmentData->PickupMesh);
						if (!bRuntimeSpawned)
						{
							DataStruct.AmmoType = AttachmentData->AmmoToUse;
							DataStruct.ClipCapacity = AttachmentData->ClipCapacity;
							DataStruct.ClipSize = AttachmentData->ClipSize;
							DataStruct.WeaponHealth = 100.0f;
						}
					}
					else if (AttachmentData->AttachmentType == EAttachmentType::Sights)
					{
						SightsAttachment->SetStaticMesh(AttachmentData->PickupMesh);
					}
					else if (AttachmentData->AttachmentType == EAttachmentType::Stock)
					{
						StockAttachment->SetStaticMesh(AttachmentData->PickupMesh);
					}
					else if (AttachmentData->AttachmentType == EAttachmentType::Grip)
					{
						GripAttachment->SetStaticMesh(AttachmentData->PickupMesh);
					}
				}
			}
		}
	}
	else
	{
		if (!bRuntimeSpawned)
		{
			DataStruct.AmmoType = WeaponData->AmmoToUse;
			DataStruct.ClipCapacity = WeaponData->ClipCapacity;
			DataStruct.ClipSize = WeaponData->ClipSize;
			DataStruct.WeaponHealth = 100.0f;
		}
	}
}

void ASWeaponPickup::Interact()
{
	DataStruct.WeaponAttachments = AttachmentArray;

	ASCharacter* PlayerCharacter = Cast<ASCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	
	if ((PlayerCharacter->PrimaryWeapon == nullptr && PlayerCharacter->SecondaryWeapon == nullptr) || (PlayerCharacter->PrimaryWeapon == nullptr && PlayerCharacter->SecondaryWeapon != nullptr))
	{
		PlayerCharacter->PrimaryWeaponCacheMap = DataStruct;
		PlayerCharacter->bNewPrimarySpawn = true;

		
		PlayerCharacter->UpdateWeapon(WeaponReference, false, DataStruct, bStatic, FTransform::Identity);
		if (PlayerCharacter->CurrentWeapon)
		{
			PlayerCharacter->CurrentWeapon->SpawnAttachments(PlayerCharacter->PrimaryWeaponCacheMap.WeaponAttachments);
		}
		PlayerCharacter->PrimaryWeapon = WeaponReference;
		PlayerCharacter->bIsPrimary = true;
	}
	else if (PlayerCharacter->SecondaryWeapon == nullptr && PlayerCharacter->PrimaryWeapon != nullptr)
	{
		PlayerCharacter->SecondaryWeaponCacheMap = DataStruct;
		PlayerCharacter->bNewSecondarySpawn = true;
		
		PlayerCharacter->UpdateWeapon(WeaponReference, false, DataStruct, bStatic, FTransform::Identity);
		if (PlayerCharacter->CurrentWeapon)
		{
			PlayerCharacter->CurrentWeapon->SpawnAttachments(PlayerCharacter->SecondaryWeaponCacheMap.WeaponAttachments);
		}
		PlayerCharacter->SecondaryWeapon = WeaponReference;
		PlayerCharacter->bIsPrimary = false;
	}
	else
	{
		if (PlayerCharacter->bIsPrimary)
		{
			PlayerCharacter->UpdateWeapon(WeaponReference, true, PlayerCharacter->PrimaryWeaponCacheMap, bStatic, GetTransform());
			PlayerCharacter->PrimaryWeaponCacheMap = DataStruct;
			PlayerCharacter->bNewPrimarySpawn = true;
			
			if (PlayerCharacter->CurrentWeapon)
			{
				PlayerCharacter->CurrentWeapon->SpawnAttachments(PlayerCharacter->PrimaryWeaponCacheMap.WeaponAttachments);
			}
			PlayerCharacter->PrimaryWeapon = WeaponReference;
		}
		else
		{
			PlayerCharacter->UpdateWeapon(WeaponReference, true, PlayerCharacter->SecondaryWeaponCacheMap, bStatic, GetTransform());
			PlayerCharacter->SecondaryWeaponCacheMap = DataStruct;
			PlayerCharacter->bNewSecondarySpawn = true;
			
			if (PlayerCharacter->CurrentWeapon)
			{
				PlayerCharacter->CurrentWeapon->SpawnAttachments(PlayerCharacter->SecondaryWeaponCacheMap.WeaponAttachments);
			}
			PlayerCharacter->SecondaryWeapon = WeaponReference;
		}
	}
	
	InteractionCompleted();
}

// Called every frame
void ASWeaponPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASWeaponPickup::InteractionCompleted()
{
	Destroy();
}

