// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/GameInstanceSubsystem/NAAbilityGameInstanceSubsystem.h"

#include "Ability/TableRow/AttributesTableRow.h"

UDataTable* UNAAbilityGameInstanceSubsystem::GetAttributeDataTable(const FName& InRowName) const
{
	if (AttributesDataTable)
	{
		const FAttributesTableRow* Row = AttributesDataTable->FindRow<FAttributesTableRow>(InRowName, "");

		if (!Row)
		{
			return nullptr;
		}

		return Row->GetDataTable();
	}

	return nullptr;
}

void UNAAbilityGameInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG( LogInit, Log, TEXT("%hs"), __FUNCTION__ )
	AttributesDataTable = Cast<UDataTable>( StaticLoadObject( UDataTable::StaticClass(), nullptr, TEXT("/Script/Engine.DataTable'/Game/00_ProjectNA/04_DataTable/01_Ability/AttributesTable.AttributesTable'") ) );
	KineticDataTable = Cast<UDataTable>( StaticLoadObject( UDataTable::StaticClass(), nullptr, TEXT("/Script/Engine.DataTable'/Game/00_ProjectNA/04_DataTable/01_Ability/KineticAttribute.KineticAttribute'") ) );
}
