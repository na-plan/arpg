// Fill out your copyright notice in the Description page of Project Settings.


#include "ARPG/Public/NAPlayerController.h"

#include "ARPG/Public/NAPlayerState.h"

void ANAPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// PlayerState가 존재하면 빙의한 폰에 대해 체력 변화를 추적, 전파
	// todo: Client일 경우 Server에서 OnRep으로 델레게이션 재전파
	if (const ANAPlayerState* NAPlayerState = GetPlayerState<ANAPlayerState>())
	{
		InPawn->OnTakeAnyDamage.AddUniqueDynamic(NAPlayerState, &ANAPlayerState::OnCharacterTakeAnyDamage);
	}
	
}

void ANAPlayerController::OnUnPossess()
{
	// Pawn에 대해 UnPossess를 수행하기 직전에 델레게이션을 해제
	if (APawn* Possessed = GetPawn())
	{
		if (ANAPlayerState* NAPlayerState = GetPlayerState<ANAPlayerState>())
		{
			FScriptDelegate Delegate;
			Delegate.BindUFunction(NAPlayerState, "OnCharacterTakeAnyDamage");
#if WITH_EDITOR || UE_BUILD_DEBUG
			// 빙의한 폰에 체력 변화 델레게이션이 설정되지 않은 경우를 확인
			check(Possessed->OnTakeAnyDamage.Contains(Delegate));
#endif

			// 빙의하면서 바인딩한 PlayerState의 체력 변화 델레게이션을 제거
			Possessed->OnTakeAnyDamage.Remove(Delegate);	
		}
	}
	
	Super::OnUnPossess();
}
