// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MonsterOwnTableRow.generated.h"

/**
 * 
 */
USTRUCT()
struct ARPG_API FMonsterOwnTable : public FTableRowBase
{
	GENERATED_BODY()

	//Own 해야할것들 ->  drop 할 아이템(확률적?), 몬스터가 사용 가능한 Skill 등등
public:
	//Skill들은 그러면 BP로 만들어야 하나? Skill 내용은 Attribute로 세팅을 해놓았는데 이제 이걸 스킬로 만들때 적용을 시켜야 함.
	UPROPERTY(EditAnywhere, Category = "Pawn|Status")
	float Health = 0;
	UPROPERTY(EditAnywhere, Category = "Pawn|Status")
	float MaxHealth = 0;
	UPROPERTY(EditAnywhere, Category = "Pawn|Status")
	float MovementSpeed = 0;
	//양수로 해야 나중에 데미지로 마이너스됨
	UPROPERTY(EditAnywhere, Category = "Pawn|Status")
	float BaseDamaage = 10;
	//Own 해야할것들 ->  drop 할 아이템
	
};




