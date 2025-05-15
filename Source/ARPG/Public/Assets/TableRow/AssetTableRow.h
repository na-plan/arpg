#pragma once

#include "CoreMinimal.h"

#include "AssetTableRow.generated.h"

USTRUCT()
struct FAssetTableRow : public FTableRowBase
{
	GENERATED_BODY()

private:
	// 복사의 대상이 될 블루프린트 에셋
	// todo: 에셋 타입 제한하기?
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta=(AllowPrivateAccess="true"))
	TSubclassOf<AActor> BlueprintAsset;

public:
	AActor* GetCDOAsset() const { return BlueprintAsset.GetDefaultObject(); }
	
};
