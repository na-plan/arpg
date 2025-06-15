// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

struct ObjectClassFinderHelper
{
	template <typename T, typename U>
	static void DoCommonRouteObject(T& Variable, U& TempVar)
	{
		if (TempVar.Succeeded())
		{
			Variable = TempVar.Object;
		}
	}

	template <typename T, typename U>
	static void DoCommonRouteClass(T& Variable, U& TempVar)
	{
		if (TempVar.Succeeded())
		{
			Variable = TempVar.Class;
		}
	}
};

template <typename T, typename KeyT, size_t Unique>
void DoFindObject( T& Variable, const TCHAR* Path )
{
	// 다른 클래스와 함수가 겹치지 않도록 정적 영역을 확보
	using Placeholder = KeyT;
	size_t UniqueIndex = Unique;
	if constexpr ( TIsTSubclassOf<T>::Value )
	{
		static ConstructorHelpers::FObjectFinder<typename T::ElementType> TempVar( Path );
		ObjectClassFinderHelper::DoCommonRouteObject( Variable, TempVar );
	}
	else
	{
		static ConstructorHelpers::FObjectFinder<std::remove_pointer_t<T>> TempVar( Path );
		ObjectClassFinderHelper::DoCommonRouteObject( Variable, TempVar );
	}
}

template <typename T, typename KeyT, size_t Unique>
void DoFindClass( T& Variable, const TCHAR* Path )
{
	// 다른 클래스와 함수가 겹치지 않도록 정적 영역을 확보
	using Placeholder = KeyT;
	size_t UniqueIndex = Unique;
	if constexpr ( TIsTSubclassOf<T>::Value )
	{
		static ConstructorHelpers::FClassFinder<typename T::ElementType> TempVar( Path );
		ObjectClassFinderHelper::DoCommonRouteClass( Variable, TempVar );
	}
	else
	{
		static ConstructorHelpers::FClassFinder<std::remove_pointer_t<T>> TempVar( Path );
		ObjectClassFinderHelper::DoCommonRouteClass( Variable, TempVar );
	}
}

#define FIND_OBJECT(VariableName, Path, Index) \
DoFindObject<decltype(VariableName), std::remove_reference_t<decltype(*this)>, Index>( VariableName, TEXT(Path) )

#define FIND_CLASS(VariableName, Path, Index) \
DoFindClass<decltype(VariableName), std::remove_reference_t<decltype(*this)>, Index>( VariableName, TEXT(Path) )

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UENUM(BLueprintType)
enum class EAbilityInputID : uint8
{
	None,
	Revive,
	Grab
};

struct FObjectPropertyUtility
{
private:
	template <typename ContainerT, typename PropertyT>
	static void CopyClassPropertyIfTypeEqualsCondition( const FClassProperty* Property, ContainerT* Dst, const ContainerT* Src ) // 정확히 매칭되는 데이터 구조 상에서 안전하게 유효함
	{
		if constexpr (TIsTSubclassOf<PropertyT>::Value)
		{
			using InnerType = typename PropertyT::ElementType;

			if ( Property && Property->MetaClass->IsChildOf<InnerType>() )
			{
				const void* SrcPropertyAddress = Property->ContainerPtrToValuePtr<void>( Src );
				const auto SrcProperty = Property->GetPropertyValue( SrcPropertyAddress );
				void* DstProperty = Property->ContainerPtrToValuePtr<void>( Dst );
				Property->SetPropertyValue( DstProperty, SrcProperty );
			}
		}
	}
	
	template <typename ContainerT, typename PropertyT>
	static void CopyClassPropertyIfTypeEqualsCondition( const FObjectProperty* Property, ContainerT* Dst, const ContainerT* Src ) // 정확히 매칭되는 데이터 구조 상에서 안전하게 유효함
	{
		if constexpr ( std::is_base_of_v<UObject, PropertyT> )
		{
			if ( Property->PropertyClass->IsChildOf<PropertyT>() )
			{
				const auto* Other = Cast<PropertyT>( Property->GetPropertyValue_InContainer( Src ) );
				Property->SetPropertyValue_InContainer( Dst, const_cast<PropertyT*>( Other ) );
			}
		}
	}

public:
	template <typename T, typename... Ts>
	static void CopyClassPropertyIfTypeEquals( T* Dst, const T* Src )
	{
		for (TFieldIterator<FClassProperty> It(T::StaticClass()); It; ++It)
		{
			const FClassProperty* Property = *It;

			if (!Property)
			{
				continue;
			}

			(CopyClassPropertyIfTypeEqualsCondition<T, Ts>( Property, Dst, Src ), ... );
		}

		for (TFieldIterator<FObjectProperty> It(T::StaticClass()); It; ++It)
		{
			const FObjectProperty* Property = *It;

			if (!Property)
			{
				continue;
			}

			(CopyClassPropertyIfTypeEqualsCondition<T, Ts>( Property, Dst, Src ), ... );
		}
	}
};