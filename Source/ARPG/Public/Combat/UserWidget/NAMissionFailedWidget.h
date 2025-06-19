

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/VerticalBox.h"
#include "NAMissionFailedWidget.generated.h"

class UCheckBox;
class UButton;
class UHorizontalBox;
/**
 * 
 */
UCLASS()
class ARPG_API UNAMissionFailedWidget : public UUserWidget
{
	GENERATED_BODY()

	UFUNCTION()
	void OnConfirmButtonClicked();

	UFUNCTION()
	void Disconnect();
	
	virtual void NativeOnInitialized() override;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Widget", meta = (AllowPrivateAccess = "true", BindWidget) )
	UButton* ConfirmButton;
	
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Widget", meta = (AllowPrivateAccess = "true", BindWidget) )
	UButton* DisconnectButton;
	
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Widget", meta = (AllowPrivateAccess = "true", BindWidget) )
	UHorizontalBox* CheckBoxHorizontalBox;
	
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Widget", meta = (AllowPrivateAccess = "true" ) )
	TArray<UCheckBox*> HaveConfirmed;

	void AdjustCheckboxCount();
	
	void AdjustCheckboxCount( const TArray<bool>& BaseArray );
	
public:
	void UpdateVoteArray( const TArray<bool>& Array );
};
