#pragma once

#include "../UE4/finder.h"
#include "log.h"
#include "../options.h"
#include "../options.h"

UObject* EasySpawnObject(SpawnObject_Params params)
{
	auto GameplayStatics = UE4::FindObject(XOR(L"GameplayStatics /Script/Engine.Default__GameplayStatics"));
	auto SpawnObject = UE4::FindObject(XOR(L"Function /Script/Engine.GameplayStatics.SpawnObject"));

	ProcessEvent(GameplayStatics, SpawnObject, &params);

	return params.ReturnValue;

}

bool IsInLobby()
{
	auto FortMatchMakingContext = UE4::FindObject(XOR(L"FortMatchmakingContext /Script/Engine.Default__FortMatchmakingContext"));
	auto IsInLobbyF = UE4::FindObject(XOR(L"Function /Script/FortniteGame.FortMatchmakingContext.IsInLobby"));

    UObject_Params params;
	
	ProcessEvent(FortMatchMakingContext, IsInLobbyF, &params);

	return params.ReturnValue;
}

static __forceinline bool IsBadReadPtr(void* p)
{
	MEMORY_BASIC_INFORMATION mbi;
	if (VirtualQuery(p, &mbi, sizeof(mbi)))
	{
		DWORD mask = (PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY);
		bool b = !(mbi.Protect & mask);
		if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)) b = true;

		return b;
	}
	return true;
}

void GiveAmmoToPlayer(int32_t count)
{
	auto FortPlayerControllerClass = UE4::FindObject(XOR(L"Class /Script/FortniteGame.FortPlayerController"));
	auto GivePlayerAmmoF = UE4::FindObject(XOR(L"Function /Script/FortniteGame.FortPlayerController.GivePlayerAmmo"));

	GiveAmmoToPlayer_Params params {
		count
	};

	ProcessEvent(FortPlayerControllerClass, GivePlayerAmmoF, &params);
}

UObject* GetPlayerPawn()
{
	ObjectFinder EngineFinder = ObjectFinder::EntryPoint(uintptr_t(GEngine));
	ObjectFinder LocalPlayer = EngineFinder.Find(L"GameInstance").Find(L"LocalPlayers");

	ObjectFinder PlayerControllerFinder = LocalPlayer.Find(L"PlayerController");
	ObjectFinder PlayerPawnFinder = PlayerControllerFinder.Find(L"Pawn");
	if (!PlayerPawnFinder.GetObj())
	{
		Log("Error while finding Pawn!");
		return nullptr;
	}
	return PlayerPawnFinder.GetObj();
}

UObject* GetPlayerController()
{
	ObjectFinder EngineFinder = ObjectFinder::EntryPoint(uintptr_t(GEngine));
	ObjectFinder LocalPlayer = EngineFinder.Find(L"GameInstance").Find(L"LocalPlayers");

	ObjectFinder PlayerControllerFinder = LocalPlayer.Find(L"PlayerController");
	if (!PlayerControllerFinder.GetObj())
	{
		Log("Error while finding PlayerController!");
		return nullptr;
	}
	return PlayerControllerFinder.GetObj();
}

ObjectFinder GetPlayerControllerFinder()
{
	ObjectFinder EngineFinder = ObjectFinder::EntryPoint(uintptr_t(GEngine));
	ObjectFinder LocalPlayer = EngineFinder.Find(L"GameInstance").Find(L"LocalPlayers");

	ObjectFinder PlayerControllerFinder = LocalPlayer.Find(L"PlayerController");
	return PlayerControllerFinder;
}

void GiveWeapon(FString Weaponname, int32_t Requestedlevel, int32_t count)
{
	auto CheatManagerClass = UE4::FindObject(XOR(L"Class /Script/FortniteGame.FortCheatManager"));
	auto GiveWeaponF = UE4::FindObject(XOR(L"Function /Script/FortniteGame.FortCheatManager.GiveWeapon"));

	GiveWeapon_Params params;
	params.WeaponName = Weaponname;
	params.RequestedLevel = Requestedlevel;
	params.Count = count;

	ProcessEvent(CheatManagerClass, GiveWeaponF, &params);

}

void ToggleAuthority(ENetRole rolea = ENetRole::ROLE_MAX, UObject* Pawn = GetPlayerPawn())
{
	if (!Pawn) return;
	bHasAuthority = !bHasAuthority;
	ENetRole role = ENetRole::ROLE_None;
	if (bHasAuthority) role = ENetRole::ROLE_Authority;
	if (rolea != ENetRole::ROLE_MAX) role = rolea;
	const auto LocalRole = reinterpret_cast<TEnumAsByte<ENetRole>*>(reinterpret_cast<uintptr_t>(Pawn) + ObjectFinder::FindOffset(XOR(L"Class /Script/Engine.Actor"), XOR(L"Role")));

	const auto RemoteRole = reinterpret_cast<TEnumAsByte<ENetRole>*>(reinterpret_cast<uintptr_t>(Pawn) + ObjectFinder::FindOffset(XOR(L"Class /Script/Engine.Actor"), XOR(L"RemoteRole")));

	*LocalRole = role;
	*RemoteRole = role;
}

void ApplyWrap()
{
	// void ServerApplyOverrideWrapToVehicle(struct TSoftObjectPtr<UAthenaItemWrapDefinition> ItemWrap); // Function FortniteGame.FortPlayerControllerAthena.ServerApplyOverrideWrapToVehicle // (Net|NetReliableNative|Event|Public|NetServer|BlueprintCallable) // @ game+0x54f40c4
	// void ServerApplyOverrideWrapToItem(struct FGuid Guid, struct TSoftObjectPtr<UAthenaItemWrapDefinition> ItemWrap); // Function FortniteGame.FortPlayerControllerAthena.ServerApplyOverrideWrapToItem // (Net|NetReliableNative|Event|Public|NetServer|HasDefaults|BlueprintCallable|NetValidate) // @ game+0x54f3f6c
	
}

/* FString GetPlayerName()
{
	/*auto PlayerState = UE4::FindObject(XOR(L"PlayerState /Script/Engine.Default__PlayerState"));
	auto NameF = UE4::FindObject(L"Function /Script/Engine.PlayerState.GetPlayerName");
	FString_Params params;
	ProcessEvent(PlayerState, NameF, &params);
	return params.ReturnValue; 
	//reinterpret_cast<uintptr_t>(GetPlayerPawn()) + ObjectFinder::FindOffset(XOR(L"Class /Script/Engine.PlayerState"), L"Function /Script/Engine.PlayerState.GetPlayerName");
	void* PlayerState = reinterpret_cast<void*>(GetPlayerPawn() + 0x240);
	auto NameF = UE4::FindObject(L"Function /Script/Engine.PlayerState.GetPlayerName");
	FString_Params params;
	ProcessEvent(PlayerState, NameF, &params);
	return params.ReturnValue;
} */

namespace Console
{
	static void CreateCheatManager()
	{
		// struct UCheatManager* CheatManager; // 0x338(0x08)
		// struct UCheatManager* CheatClass; // 0x340(0x08)
		//UObject** CheatManagerObj = reinterpret_cast<UObject**>((GetPlayerControllerFinder().Find(L"CheatManager")).GetObj());
		//UObject* Outer = UE4::FindObject(XOR(L"Class /Script/FortniteGame.FortPlayerController"));
		int CheatManagerOff = ObjectFinder::FindOffset(L"Class /Script/Engine.PlayerController", L"CheatManager");
		UObject** CheatManagerObj = reinterpret_cast<UObject**>(__int64(GetPlayerController()) + __int64(CheatManagerOff));
		// auto CheatManagerC = PlayerFinder.Find(XOR(L"CheatManager"));
		// UObject* Outer = PlayerFinder.Find(XOR(L"CheatManager")).GetObj();
		auto CheatManagerClass = UE4::FindObject(XOR(L"Class /Script/Engine.CheatManager"));
		//auto CheatClass = PlayerFinder.Find(XOR(L"CheatClass"));

		//UObject* CheatManagerObj = CheatManagerC.GetObj();

		SpawnObject_Params params{
			//CheatClass.GetObj(),
			CheatManagerClass,
			GetPlayerController()
			//CheatManagerObj
		};
		auto ret = EasySpawnObject(params);

		*CheatManagerObj = ret;
	}

	static void ConsoleLog(std::wstring message)
	{
		ObjectFinder EngineFinder = ObjectFinder::EntryPoint(uintptr_t(GEngine));
		ObjectFinder GameViewPortClientFinder = EngineFinder.Find(XOR(L"GameViewport"));
		ObjectFinder WorldFinder = GameViewPortClientFinder.Find(XOR(L"World"));
		ObjectFinder GameModeFinder = WorldFinder.Find(XOR(L"AuthorityGameMode"));

		static auto fn = UE4::FindObject<UFunction*>(XOR(L"Function /Script/Engine.GameMode.Say"));

		const FString Msg = message.c_str();
		AGameMode_Say_Params params;
		params.Msg = Msg;

		ProcessEvent(GameModeFinder.GetObj(), fn, &params);
	}

	//This is a hacky way to handle custom commands
	static auto ExecuteConsoleCommand(const wchar_t* command)
	{
		ObjectFinder EngineFinder = ObjectFinder::EntryPoint(uintptr_t(GEngine));
		ObjectFinder GameViewPortClientFinder = EngineFinder.Find(XOR(L"GameViewport"));
		ObjectFinder WorldFinder = GameViewPortClientFinder.Find(XOR(L"World"));
		ObjectFinder LocalPlayer = EngineFinder.Find(XOR(L"GameInstance")).Find(XOR(L"LocalPlayers"));
		ObjectFinder PlayerControllerFinder = LocalPlayer.Find(XOR(L"PlayerController"));

		auto KismetSysLib = UE4::FindObject<UObject*>(XOR(L"KismetSystemLibrary /Script/Engine.Default__KismetSystemLibrary"));
		static auto fn = UE4::FindObject<UFunction*>(XOR(L"Function /Script/Engine.KismetSystemLibrary.ExecuteConsoleCommand"));

		UKismetSystemLibrary_ExecuteConsoleCommand_Params params;
		params.WorldContextObject = WorldFinder.GetObj();
		params.Command = command;
		params.SpecificPlayer = PlayerControllerFinder.GetObj();

		ProcessEvent(KismetSysLib, fn, &params);
		printf(XOR("Executed a console command!\n"));
	}

	static void ConstructConsole()
	{

		ObjectFinder EngineFinder = ObjectFinder::EntryPoint(uintptr_t(GEngine));
		auto ConsoleClass = UE4::FindObject(XOR(L"Class /Script/Engine.Console"));

		// auto FortGameViewportClient = UE4::FindObject(L"FortEngine /Engine/Transient.FortEngine.FortGameViewportClient");
		UObject* GameViewPortClientObj = EngineFinder.Find(XOR(L"GameViewport")).GetObj();

		while (!GameViewPortClientObj)
		{
			GameViewPortClientObj = EngineFinder.Find(XOR(L"GameViewport")).GetObj();
		}

		SpawnObject_Params params{
			ConsoleClass,
			GameViewPortClientObj
		};

		int ViewportConsoleOff = ObjectFinder::FindOffset(L"Class /Script/Engine.GameViewportClient", L"ViewportConsole");
		UObject** ViewportConsole = reinterpret_cast<UObject**>(__int64(GameViewPortClientObj) + __int64(ViewportConsoleOff));

		//std::cout << GameplayStatics << " " << SpawnObject << " " << ConsoleClass << " " << GameViewPortClientFinder.GetObj(); //<< " " << ViewportConsole << std::endl;
		auto ret = EasySpawnObject(params);

		*ViewportConsole = ret;

	}

};

/* void ApplyMobileVBucksScreen()
{
	// void ApplyMobileLayout(); // Function VBucksStoreScreen.VBucksStoreScreen_C.ApplyMobileLayout // (Public|BlueprintCallable|BlueprintEvent) // @ game+0xd4b69c
	auto VBucksClass = UE4::FindObject(XOR(L"WidgetBlueprintGeneratedClass /Script/VBucksStoreScreen.VBucksStoreScreen_C"));
	auto ApplyF = UE4::FindObject(XOR(L"Function /Script/VBucksStoreScreen.VBucksStoreScreen_C.ApplyMobileLayout"));

	void* params;
	ProcessEvent(VBucksClass, ApplyF, &params);
} */

std::vector<TEnumAsByte<ENetRole>*> GetRoles()
{
	// ENetRole GetRemoteRole(); // Function Engine.Actor.GetRemoteRole // (Final|Native|Public|BlueprintCallable|BlueprintPure|Const) // @ game+0x64b7300
	const auto LocalRole = reinterpret_cast<TEnumAsByte<ENetRole>*>(reinterpret_cast<uintptr_t>(GetPlayerPawn()) + ObjectFinder::FindOffset(XOR(L"Class /Script/Engine.Actor"), XOR(L"Role")));

	const auto RemoteRole = reinterpret_cast<TEnumAsByte<ENetRole>*>(reinterpret_cast<uintptr_t>(GetPlayerPawn()) + ObjectFinder::FindOffset(XOR(L"Class /Script/Engine.Actor"), XOR(L"RemoteRole")));
	std::vector<TEnumAsByte<ENetRole>*> Roles = { LocalRole, RemoteRole }; 

	return Roles;
}

class Player
{
private:
	static UObject* Controller;
	static UObject* Pawn;
	static UObject* Mesh;
	static UObject* AnimInstance;
	static void UpdateMesh()
	{
		ObjectFinder PawnFinder = ObjectFinder::EntryPoint(uintptr_t(GetPlayerPawn()));

		ObjectFinder MeshFinder = PawnFinder.Find(XOR(L"Mesh"));
		Mesh = MeshFinder.GetObj();
	}

	static void UpdateAnimInstance()
	{
		UpdateMesh();

		auto FUNC_GetAnimInstance = UE4::FindObject<UFunction*>(XOR(L"Function /Script/Engine.SkeletalMeshComponent.GetAnimInstance"));

		USkeletalMeshComponent_GetAnimInstance_Params GetAnimInstance_Params;

		ProcessEvent(Mesh, FUNC_GetAnimInstance, &GetAnimInstance_Params);

		AnimInstance = GetAnimInstance_Params.ReturnValue;
	}

public:

	static void UpdateAll()
	{
		UpdateAnimInstance();
		UpdateMesh();
		Controller = GetPlayerController();
	}

	static auto Emote(UObject* EmoteDef)
	{
		//We grab the mesh from the pawn then use it to get the animation instance
		//if (!this->Mesh || !this->AnimInstance || !IsBadReadPtr(this->Mesh) || !IsBadReadPtr(this->AnimInstance)) {}
		UpdateAll();

		if (EmoteDef && !IsBadReadPtr(EmoteDef))
		{
			//Emote Def is valid, now we grab the animation montage
			auto FUNC_GetAnimationHardReference = UE4::FindObject<UFunction*>(XOR(L"Function /Script/FortniteGame.FortMontageItemDefinitionBase.GetAnimationHardReference"));

			UFortMontageItemDefinitionBase_GetAnimationHardReference_Params GetAnimationHardReference_Params;
			GetAnimationHardReference_Params.BodyType = EFortCustomBodyType::All;
			GetAnimationHardReference_Params.Gender = EFortCustomGender::Both;
			GetAnimationHardReference_Params.PawnContext = Pawn;

			ProcessEvent(EmoteDef, FUNC_GetAnimationHardReference, &GetAnimationHardReference_Params);

			auto Animation = GetAnimationHardReference_Params.ReturnValue;

			//got the animation, now play :JAM:
			auto FUNC_Montage_Play = UE4::FindObject<UFunction*>(XOR(L"Function /Script/Engine.AnimInstance.Montage_Play"));

			UAnimInstance_Montage_Play_Params params;
			params.MontageToPlay = Animation;
			params.InPlayRate = 1;
			params.ReturnValueType = EMontagePlayReturnType::Duration;
			params.InTimeToStartMontageAt = 0;
			params.bStopAllMontages = true;

			ProcessEvent(AnimInstance, FUNC_Montage_Play, &params);
		}
		else
		{
			MessageBoxA(nullptr, XOR("This item doesn't exist!"), XOR(NAME), MB_OK);
		}
	}

	auto SetSkeletalMesh(const wchar_t* meshname)
	{
		if (!this->Mesh || !IsBadReadPtr(this->Mesh))
		{
			this->UpdateMesh();
		}

		static auto fn = UE4::FindObject<UFunction*>(XOR(L"Function /Script/Engine.SkinnedMeshComponent.SetSkeletalMesh"));

		std::wstring MeshName = meshname;

		std::wstring name = MeshName + L"." + MeshName;

		auto Mesh = UE4::FindObject<UObject*>(name.c_str(), true);

		if (Mesh)
		{
			USkinnedMeshComponent_SetSkeletalMesh_Params params;
			params.NewMesh = Mesh;
			params.bReinitPose = false;

			ProcessEvent(this->Mesh, fn, &params);
		}
	}
};

namespace Skins
{
	static void ApplySkin()
	{
		ObjectFinder EngineFinder = ObjectFinder::EntryPoint(uintptr_t(GEngine));
		ObjectFinder GameViewPortClientFinder = EngineFinder.Find(XOR(L"GameViewport"));
		ObjectFinder WorldFinder = GameViewPortClientFinder.Find(XOR(L"World"));
		ObjectFinder PawnFinder = ObjectFinder::EntryPoint(uintptr_t(GetPlayerPawn()));
		ObjectFinder PlayerStateFinder = PawnFinder.Find(XOR(L"PlayerState"));

		auto Hero = UE4::FindObject<UObject*>(XOR(L"FortHero /Engine/Transient.FortHero_"));
		auto CharacterParts = reinterpret_cast<TArray<UObject*>*>(reinterpret_cast<uintptr_t>(Hero) + ObjectFinder::FindOffset(XOR(L"Class /Script/FortniteGame.FortHero"), XOR(L"CharacterParts")));

		auto head = CharacterParts->operator[](1);
		auto body = CharacterParts->operator[](0);

		head = UE4::FindObject<UObject*>(XOR(L"CustomCharacterPart /Game/Athena/Heroes/Meshes/Heads/Dev_TestAsset_Head_M_XL.Dev_TestAsset_Head_M_XL"));
		body = UE4::FindObject<UObject*>(XOR(L"CustomCharacterPart /Game/Athena/Heroes/Meshes/Bodies/Dev_TestAsset_Body_M_XL.Dev_TestAsset_Body_M_XL"));

		auto KismetLib = UE4::FindObject<UObject*>(XOR(L"FortKismetLibrary /Script/FortniteGame.Default__FortKismetLibrary"));
		static auto fn = UE4::FindObject<UFunction*>(XOR(L"Function /Script/FortniteGame.FortKismetLibrary.ApplyCharacterCosmetics"));

		UFortKismetLibrary_ApplyCharacterCosmetics_Params params;
		params.WorldContextObject = WorldFinder.GetObj();
		params.CharacterParts = *CharacterParts;
		params.PlayerState = PlayerStateFinder.GetObj();

		ProcessEvent(KismetLib, fn, &params);
	}

	static void ShowSkin()
	{
		ObjectFinder PawnFinder = ObjectFinder::EntryPoint(uintptr_t(GetPlayerPawn()));
		ObjectFinder PlayerStateFinder = PawnFinder.Find(XOR(L"PlayerState"));

		auto KismetLib = UE4::FindObject<UObject*>(XOR(L"FortKismetLibrary /Script/FortniteGame.Default__FortKismetLibrary"));
		static auto fn = UE4::FindObject<UFunction*>(XOR(L"Function /Script/FortniteGame.FortKismetLibrary.UpdatePlayerCustomCharacterPartsVisualization"));

		UFortKismetLibrary_UpdatePlayerCustomCharacterPartsVisualization_Params params;
		params.PlayerState = PlayerStateFinder.GetObj();

		ProcessEvent(KismetLib, fn, &params);
		Log(XOR("Character Parts are now visible!"));
	}
};