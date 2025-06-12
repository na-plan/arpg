// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/AttributeSet/NAAttributeSet.h"

#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "Monster/AI/MonsterAIController.h"


void UNAAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME( UNAAttributeSet, Health );
	DOREPLIFETIME( UNAAttributeSet, MaxHealth );
	DOREPLIFETIME( UNAAttributeSet, AP );
	DOREPLIFETIME( UNAAttributeSet, MovementSpeed );
}

void UNAAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetContext();
		AActor* InstigatorActor = EffectContext.GetInstigator();
		if (InstigatorActor)
		{
			AActor* OwnerActor = GetOwningActor();					
			if (AMonsterAIController* AIController = Cast<AMonsterAIController>(OwnerActor->GetInstigatorController()))
			{
				if (UBlackboardComponent* MonsterAIBB = AIController->GetBlackboardComponent())
				{
					if (!MonsterAIBB->GetValueAsObject(TEXT("DetectPlayer")))
					{
						MonsterAIBB->SetValueAsObject(TEXT("DetectPlayer"), Cast<UObject>(InstigatorActor));
					}
				}
			}
		}
	}


}
