#include "Item/ItemManagerStatics.h"

FItemManagerImpl& FItemManagerStatics::Get( const UWorld* InWorld )
{
#if WITH_EDITOR || !UE_BUILD_SHIPPING
	// 에디터일 경우에는 게임인스턴스가 없기 떄문에, 에디터 전용 싱글톤 객체를 만들어 아이템 관리 
	if (InWorld->IsEditorWorld() || InWorld->WorldType == EWorldType::Inactive)
	{
		static TUniquePtr<FItemManagerImpl> Instance = MakeUnique<FItemManagerImpl>();

		if (!Instance->IsInitialized())
		{
			// 엔진이 정리되기 전에 마무리
			FCoreDelegates::OnExit.AddLambda([]()
			{
				Instance.Reset();
			});
			Instance->Initialize();
		}
		
		return *Instance;
	}
	check(false);
#endif
	return InWorld->GetGameInstance()->GetSubsystem<UNAItemGameInstanceSubsystem>()->GetManager();
}
