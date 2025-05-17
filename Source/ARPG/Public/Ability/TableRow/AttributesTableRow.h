#pragma once

#include "CoreMinimal.h"

#include "AttributesTableRow.generated.h"

USTRUCT(BlueprintType)
struct FAttributesTableRow : public FTableRowBase
{
	GENERATED_BODY()

	// 데이터 테이블 타입 제한: 언리얼에서 기본으로 제공하는 FAttributeMetaData를 사용해야 InitStats을 활용할 수 있음
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta=(AllowPrivateAccess="true", RequiredAssetDataTags="RowStructure=/Script/GameplayAbilities.AttributeMetaData"))
	UDataTable* AttributeDataTable = nullptr;

	UDataTable* GetDataTable() const { return AttributeDataTable; }
};
