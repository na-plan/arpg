#pragma once

#include "CoreMinimal.h"
#include "NAInteractableDataStructs.generated.h"

UENUM(BlueprintType)
enum class ENAInteractableType : uint8
{
	None					UMETA(DisplayName = "None"),
	
	Pickable				UMETA(DisplayName = "Pickable"),
	Placeable				UMETA(DisplayName = "Placeable"),
	NonPlayerController		UMETA(DisplayName = "NonPlayerController"),
	Toggle					UMETA(DisplayName = "Toggle"),
	Container				UMETA(DisplayName = "Container"),
};
ENUM_CLASS_FLAGS(ENAInteractableType)

// 아이템 인스턴스 마다 구분되어야 하는 상호작용 데이터
USTRUCT(BlueprintType)
struct FNAInteractableData
{
	GENERATED_BODY()

public:
	// 기본값 지정, Base line 아님
	// Base line: 이 데이터 구조체가 쓰일 문맥에서 지정한 의미상의 기본값
	FNAInteractableData()
		: InteractableType(ENAInteractableType::None),
		InteractionName(FText::GetEmpty()),
		//InteractionScript(FText::GetEmpty()),
		InteractionDuration(0.f),
		Quantity(0),
		InteractingCharacter(nullptr)
	{
	}
	
	bool IsModifiedFromBaseline() const
	{
		return bModifiedFromBaseline ;
	}

	UPROPERTY(EditInstanceOnly, Category = "Interactable Data")
	ENAInteractableType InteractableType;

	UPROPERTY(/*EditInstanceOnly,*/VisibleInstanceOnly, Category = "Interactable Data")
	FText InteractionName;

	//UPROPERTY(EditInstanceOnly, Category = "Interactable Data")
	//FText InteractionScript;
	
	UPROPERTY(EditInstanceOnly, Category = "Interactable Data")
	float InteractionDuration;
	
	// ANAPickableItemActor 전용
	UPROPERTY(EditInstanceOnly, Category = "Interactable Data")
	int32 Quantity;

	UPROPERTY(VisibleInstanceOnly, Category = "Interactable Data")
	TWeakObjectPtr<class ANACharacter> InteractingCharacter;

private:
	friend class INAInteractableInterface;
	
	mutable uint8 bModifiedFromBaseline : 1 = false;
	bool IsDifferentFromDefault() const;
};