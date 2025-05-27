// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MonsterOwnTableRow.generated.h"

/**
 * 
 */
USTRUCT()
struct ARPG_API FMonsterOwnTableRow : public FTableRowBase
{
	GENERATED_BODY()

	//Own 해야할것들 ->  drop 할 아이템(확률적?), 몬스터가 사용 가능한 Skill 등등
public:
	//Skill들은 그러면 BP로 만들어야 하나? Skill 내용은 Attribute로 세팅을 해놓았는데 이제 이걸 스킬로 만들때 적용을 시켜야 함.

	
};

//Player도 사용 가능하게 할까?
USTRUCT()
struct FSkillTable : public FTableRowBase
{
	GENERATED_BODY()
public:
	//Skill Montage
	UPROPERTY(EditAnywhere)
	UAnimMontage* SkillMontage;

	UPROPERTY(EditAnywhere)
	float Damage;
	UPROPERTY(EditAnywhere)
	float Cooltime;
	UPROPERTY(EditAnywhere)
	float Cost;

	// Monster가 사용시 사용 가능한 사거리
	UPROPERTY(EditAnywhere)
	float Range;

	//SpawnProjectile (Projectile Data를 만들고 Spawn시키도록 해야함 -> 그런데 쓸지 안쓸지를 모르겠어어 일단 주석)

};

// 하나의 Pawn(Character)가 보유중인 Skill
USTRUCT()
struct FOwnSkillTable : public FTableRowBase
{
	GENERATED_BODY()
public:
	//Array로 가지고 갈 스킬들을 가져가기
	//UPROPERTY(EditAnywhere, meta = (RowType = "/Script/ARPG.SkillTable"))
	//TArray<FDataTableRowHandle> OwnSkillArray;
	
	// 여기서 Array로 만들어가지고 monster한테 전달하는 방식으로 ㄱㄱ
	UPROPERTY(EditAnywhere)
	TArray<FSkillTable> OwnSkillArray;


};


