#pragma once

// 슬롯 개수
static constexpr int32 InventoryRowCount = 5;
static constexpr int32 InventoryColumnCount = 5;
static constexpr int32 MaxInventorySlotCount = InventoryRowCount * InventoryColumnCount;
static constexpr int32 MaxWeaponSlotCount = 4;
//static constexpr int32 MaxTotalSlots = MaxInventorySlotCount + MaxWeaponSlotCount;

// 슬롯 ID 문자열 포맷
static constexpr TCHAR InventorySlotFormat[] = TEXT("Inven_%02d");
static constexpr TCHAR WeaponSlotFormat[]    = TEXT("Weapon_%02d");

static constexpr TCHAR InventorySlotPrefix[] = TEXT("Inven_");
static constexpr TCHAR WeaponSlotPrefix[]    = TEXT("Weapon_");