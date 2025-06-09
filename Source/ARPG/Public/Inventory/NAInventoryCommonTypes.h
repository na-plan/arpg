#pragma once

// 슬롯 개수
static constexpr int32 InventoryRowCount = 5;
static constexpr int32 InventoryColumnCount = 5;
static constexpr int32 MaxInventorySlots = InventoryRowCount * InventoryColumnCount;
static constexpr int32 MaxWeaponSlots = 4;
//static constexpr int32 MaxTotalSlots = MaxInventorySlots + MaxWeaponSlots;

// 슬롯 ID 문자열 포맷
static constexpr TCHAR InventorySlotFormat[] = TEXT("Inven_%02d");
static constexpr TCHAR WeaponSlotFormat[]    = TEXT("Weapon_%02d");
