// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NAManagedAsset.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnAssetNameSet, const FName&, PreviousName);

// This class does not need to be modified.
UINTERFACE()
class UNAManagedAsset : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ARPG_API INAManagedAsset
{
	GENERATED_BODY()

	virtual void RetrieveAssetIfAssetNameChanged(const FName& PreviousAssetName);

protected:
	// 적용하려는 에셋의 이름을 설정하는 가상 함수 (상속하는 객체가 구현해야 함)
	// 이름이 바뀐 경우 자연스럽게 에셋을 설정하기 위해 함수를 감쌌음
	UFUNCTION()
	virtual void SetAssetNameDerivedImplementation(const FName& InAssetName) PURE_VIRTUAL(IAssetManaged::SetAssetNameDerivedImplementation, );
	
public:
	// 에셋을 적용하는 순수 가상 함수 (상속하는 객체가 구현해야 함)
	UFUNCTION()
	virtual void RetrieveAsset(const AActor* InCDO) PURE_VIRTUAL(IAssetManaged::UpdateAsset, );

	// 적용하려는 에셋의 이름을 설정하는 함수
	// 인터페이스에서는 UPROPERTY를 쓸 수 없기 때문에 에셋 데이터는 자식이 관리해야 함
	void SetAssetName(const FName& InAssetName);
	
	// 적용하려는 에셋의 이름을 반환하는 순수 가상함수 (상속하는 객체가 구현해야 함)
	// 인터페이스에서는 UPROPERTY를 쓸 수 없기 때문에 자식이 관리해야 함
	UFUNCTION()
	virtual FName GetAssetName() const PURE_VIRTUAL(IAssetManaged::GetAssetName, return NAME_None; );
};
