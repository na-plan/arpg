// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/GameplayAbility/AttackGameplayAbility.h"
#include "Monster/Pawn/MonsterBase.h"
#include "AbilitySystemComponent.h"

void UAttackGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
    
    //Animinstance가지고 오는거
    UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();

    //OwnerActor 가지고 오는거 
<<<<<<< HEAD
    AActor* OwnerActor = GetAvatarActorFromActorInfo(); // AActor* OwnerActor = Cast<AActor>(ActorInfo->OwnerActor);
    if (!OwnerActor) return ;

    // OwnerActor의 물리적인거
    AActor* AvatarActor = Cast<AActor>(ActorInfo->AvatarActor.Get());


    //AvatarActor로 casting 안되면 return
    if (!AvatarActor) { return; }


    //서버 또는 클라이언트에서 실행 권한을 확인 -> 멀티플레이어 환경에서 클라이언트 예측(Replication) 가능 여부를 체크
    // 서버에서 능력을 실행할 권한이 있는 경우 계속 진행
    
    //실행 권한 확인 
    if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
    {
        //능력 실행이 가능한지 확인 (CommitAbility()) 
        // 마나나 특정 한게 부족하면 능력을 실행하지 않고 return 
        // 능력이 정상적으로 실행되면 다음 단계 진행

        // 능력 실행 가능한지 확인
        if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
        {
            //불가능하면 return
            return;
        }

<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 93b5625 (33)
        //Monster
        if (AMonsterBase* OwnerMonster = Cast<AMonsterBase>(ActorInfo->AvatarActor.Get()))
        {
            UAnimMontage* AttackMontage = OwnerMonster->GetAttackMontage();

            UAbilitySystemComponent* AbilitySystemComponent = OwnerMonster->GetAbilitySystemComponent();
            //UGameplayAbility* GameplayAbility;
            if (AttackMontage&& AbilitySystemComponent)
            {
                AbilitySystemComponent->PlayMontage(this, GetCurrentActivationInfo(), AttackMontage, 1.0f);
            }
        }

        //Player
        //if (ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get()))
        //{
        //    UAnimMontage* AttackMontage = Character->GetAttackMontage();
        //    UAbilitySystemComponent* AbilitySystemComponent = Character->GetAbilitySystemComponent();
        //    //UGameplayAbility* GameplayAbility;
        //    if (AttackMontage&& AbilitySystemComponent)
        //    {
        //        AbilitySystemComponent->PlayMontage(this, GetCurrentActivationInfo(), AttackMontage, 1.0f);
        //    }
        //}
<<<<<<< HEAD

=======

        //ACharacter* Character = CastChecked<ACharacter>(ActorInfo->AvatarActor.Get());
>>>>>>> 6efa875 (22)
=======

>>>>>>> 93b5625 (33)
        //UAnimMontage* AttackMontage = OwnerMonster->GetAttackMontage();
        //ActorInfo->AbilitySystemComponent->PlayMontage();
    }

=======
    AActor* OwnerActor = GetAvatarActorFromActorInfo();
    if (!OwnerActor) return ;

    // Target 대상
    AActor* TargetCharacter = Cast<AActor>(ActorInfo->AvatarActor.Get());


>>>>>>> d34d6c1 (11)

    //if (MontageToPlay && AnimInstance)
    //{
    //    float Duration = AnimInstance->Montage_Play(MontageToPlay);
    //}

    /*CommitAbility()는 특정 능력을 실행하기 전에 사용됩니다. 
    이 함수는 능력 실행을 위한 다양한 조건을 확인하고, 해당 능력이 실제로 실행될 수 있도록 리소스를 소비하는 역할을 합니다. 
    주요 기능은 다음과 같습니다:    자원 소비: 능력 실행에 필요한 마나, 스태미나, 기타 리소스를 소모함
    태그 확인: 특정 태그 조건을 충족하는지 검사하여 능력 실행을 제한할 수 있음
    재사용 대기시간 적용: 능력이 정상적으로 실행될 경우, 필요한 쿨다운을 설정함
    즉, 능력이 실행될 준비가 되었는지를 확인하고, 필요한 리소스를 소비한 뒤 실행하는 역할을 합니다.*/
    if (CommitAbility(Handle, ActorInfo, ActivationInfo))
    {

        /*EndAbility()는 능력이 완료되거나 강제로 종료될 때 호출됩니다. 주요 기능은 다음과 같습니다:
        능력 종료 처리: 능력 사용이 끝나면 관련된 상태를 정리함
        재사용 대기시간 해제: 능력이 끝나고 새로운 능력을 실행할 수 있도록 대기시간 관리
        이벤트 트리거: 능력 종료 시 필요한 후속 이벤트를 발생시킬 수 있음
        능력 사용 중 캐릭터가 상태 변화(예: 기절, 사망)를 겪거나, 능력이 취소되었을 때도 EndAbility()가 호출될 수 있습니다.
        */
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    float AttackRange = 200.0f; // 공격 범위 설정

}

void UAttackGameplayAbility::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{

}

void UAttackGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{

}

bool UAttackGameplayAbility::IsTargetInRange(AActor* TargetActor, float Range)
{
    if (!TargetActor) return false;

    AActor* OwnerActor = GetAvatarActorFromActorInfo();
    if (!OwnerActor) return false;

    float Distance = FVector::Dist(OwnerActor->GetActorLocation(), TargetActor->GetActorLocation());
    return Distance <= Range;
}
