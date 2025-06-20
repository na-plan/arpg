// Fill out your copyright notice in the Description page of Project Settings.


#include "Skill/AnimNotify/NAAnimNotify_SuplexDmg.h"
#include "AbilitySystemComponent.h"
#include "HP/GameplayEffect/NAGE_Damage.h"
#include "Ability/AttributeSet/NAAttributeSet.h"


void UNAAnimNotify_SuplexDmg::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (MeshComp->GetWorld()->IsGameWorld())
	{
		if (MeshComp->GetOwner()->HasAuthority())
		{

			if (UAbilitySystemComponent* OwnerASC = MeshComp->GetOwner()->FindComponentByClass<UAbilitySystemComponent>())
			{
				ContextHandle = OwnerASC->MakeEffectContext();
				ContextHandle.AddInstigator(MeshComp->GetOwner()->GetInstigatorController(), MeshComp->GetOwner());
				ContextHandle.SetAbility(OwnerASC->GetAnimatingAbility());
				ContextHandle.AddSourceObject(this);
				SpecHandle = OwnerASC->MakeOutgoingSpec(UNAGE_Damage::StaticClass(), 1.f, ContextHandle);
				SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Damage"), -100);

				float HP = Cast<UNAAttributeSet>(OwnerASC->GetAttributeSet(UNAAttributeSet::StaticClass()))->GetHealth();

				OwnerASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), OwnerASC);
				float HP2 = Cast<UNAAttributeSet>(OwnerASC->GetAttributeSet(UNAAttributeSet::StaticClass()))->GetHealth();

			}

		}
	}



}
