#include "Interaction/NAInteractableDataStructs.h"


bool FNAInteractableData::IsDifferentFromDefault() const
{
	// 이건 기본 생성자로 만들어진 FNAInteractableData의 진짜 기본값
	static const FNAInteractableData DefaultInteractableData;
	
	static UScriptStruct* Struct = FNAInteractableData::StaticStruct();
	for (TFieldIterator<FProperty> It(Struct); It; ++It)
	{
		FProperty* Prop = *It;
		if (!Prop->HasAnyPropertyFlags(CPF_DisableEditOnTemplate))
			continue;
		
		if (!Prop->Identical_InContainer(this, &DefaultInteractableData))
		{
			return true;
		}
	}
	
	return false;
}

