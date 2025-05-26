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

	// CDO GameInstance 초기화 시점에 엔진 에셋 로딩 Fragment가 없는 관계로 SubObject로 만들지 않고, 동적으로 초기화
	if (UDataTable* DataTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, TEXT("/Script/Engine.DataTable'/Game/00_ProjectNA/03_DataTable/01_Ability/AttributesTable.AttributesTable'"))))
	{
		AttributesDataTable = DataTable;
	}
}
