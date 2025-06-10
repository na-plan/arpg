// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UENUM(BLueprintType)
enum class EAbilityInputID : uint8
{
	None,
	Revive
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