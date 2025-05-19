#pragma once

#include "CoreMinimal.h"

#include "UserFilePath.generated.h"

USTRUCT()
struct FUserFilePath : public FTableRowBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	FString DataTableName = TEXT("");

	UPROPERTY(EditAnywhere)
	UDataTable* CachedDT = nullptr;
};


UCLASS()
class ARPG_API UFilePathInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UFilePathInstance()
	{
		static ConstructorHelpers::FObjectFinder<UDataTable> FoundDT(TEXT("/Script/Engine.DataTable'/Game/00_ProjectNA/01_Blueprint/05_Datatables/DT_FilePath.DT_FilePath'"));
		if (FoundDT.Succeeded())
			FilePathDataTable = FoundDT.Object;
	}
	
public:
	static UDataTable* FilePathDataTable;

	static FString Path(const FString& DTName)
	{
		FUserFilePath* Row = FilePathDataTable->FindRow<FUserFilePath>(FName(DTName), FString(""));
		FString result = TEXT("/Script/Engine.DataTable'") + Row->CachedDT->GetPathName() + TEXT("'"); 
		return result;
	}
};

inline ARPG_API const TCHAR* GetDTPath(const FString& DTName) { return *UFilePathInstance::Path(DTName); }