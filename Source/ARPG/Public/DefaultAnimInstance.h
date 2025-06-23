// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DefaultAnimInstance.generated.h"

enum class EFireArmType : uint8;
/**
 * 
 */
// 현재 해당 파일 위치는 monster 입니다 추후 player 나 다른곳에 사용한다면 이동시켜 주세요
UCLASS()
class ARPG_API UDefaultAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	void SetAimRotation(const FRotator& NewAimRotation) { AimRotation = NewAimRotation; }
	const FRotator& GetAimRotation() { return AimRotation; }
protected:
	UPROPERTY()
	class UPawnMovementComponent* MovementComponent;

	UPROPERTY(BlueprintReadOnly)
	double Vertical;
	
	UPROPERTY(BlueprintReadOnly)
	double Speed = 0.0;

	UPROPERTY(BlueprintReadOnly)
	double Direction = 0.0;

	UPROPERTY(BlueprintReadOnly)
	bool bShoudMove = false;

	UPROPERTY(BlueprintReadOnly)
	bool bLeftHandEmpty = true;
	
	UPROPERTY(BlueprintReadOnly)
	bool bRightHandEmpty = true;

	UPROPERTY(BlueprintReadOnly)
	EFireArmType RightFireArmType;
	
	UPROPERTY(BlueprintReadOnly)
	EFireArmType LeftFireArmType;

	/*쓸지 안쓸지 고민중 + fall anim not exist */
	UPROPERTY(BlueprintReadOnly)
	bool bIsCrouch = false;
	
	UPROPERTY(BlueprintReadOnly)
	bool bIsFalling = false;

	UPROPERTY(BlueprintReadOnly)
	bool bShouldCrawl = false;
	
	UPROPERTY(BlueprintReadOnly)
	FRotator AimRotation;

	//Playerable Character Var
	UPROPERTY(BlueprintReadOnly)
	bool ZoomAim;
	
	
	
};
