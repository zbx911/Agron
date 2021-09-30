/**
 * Copyright (c) 2020-2021 Kareem Olim (Kemo)
 * All Rights Reserved. Licensed under the Neo License
 * https://neonite.dev/LICENSE.html
 */

#pragma once

#include <Windows.h>
#include <iostream>
#include <regex>

#include "UE4/ue4.h"

#include "curldefs.h"
#include "options.h"
#include "Util/helper.h"

typedef void CURL;

auto (*curl_easy_setopt_original)(CURL* cURL, uintptr_t opt, ...)->CURLcode;

void Log(const char* msg, bool nl = true)
{
    std::cout << msg;
    if (nl) std::cout << "\n";
}

static auto cURLDetour(CURL* Curl, uintptr_t opt, va_list info)->CURLcode
{
    int OFF = 0;
    switch (opt)
    {
    case CURL_NOPROXY:
        return curl_easy_setopt_original(Curl, opt, "");
        break;

    case CURL_VERIFYPEER:
        return curl_easy_setopt_original(Curl, opt, OFF);
        break;
    case CURL_VERIFYHOST:
        return curl_easy_setopt_original(Curl, opt, OFF);
        break;
    case CURL_PINNEDPUBLICKEY:
        return CURLcode::CURLE_OK;
        break;
    case CURL_URL:
        std::string url = info;
        std::regex Host(XOR("(.*).ol.epicgames.com"));

        /*if (std::regex_search(info, std::regex(XOR ("/fortnite/api/cloudstorage/system")))) {
            url = std::regex_replace(info, Host, FNhost);
            Log(XOR ("Redirected Cloudstorage / System"));
        } */
        /* else */if (std::regex_search(info, std::regex(XOR("/fortnite/api/v2/versioncheck/")))) {
            url = std::regex_replace(info, Host, FNhost);
            Log(XOR("Redirected Versioncheck"));
        }
        else if (std::regex_search(info, std::regex(XOR("/fortnite/api/game/v2/profile")))) {
            url = std::regex_replace(info, Host, FNhost);
            Log(XOR("Redirected Profile"));
        }
        else if (std::regex_search(info, std::regex(XOR("/content/api/pages/fortnite-game")))) {
            url = std::regex_replace(info, Host, FNhost);
            Log(XOR("Redirected fortnite-game"));
        }
        else if (std::regex_search(info, std::regex(XOR("/affiliate/api/public/affiliates/slug")))) {
            url = std::regex_replace(info, Host, FNhost);
            Log(XOR("Redirected Affiliates"));
        }
        else if (std::regex_search(info, std::regex(XOR("/socialban/api/public/v1")))) {
            url = std::regex_replace(info, Host, FNhost);
            Log(XOR("Redirected Socialban"));
        }

        return curl_easy_setopt_original(Curl, opt, url.c_str());
        break;
    }
    return curl_easy_setopt_original(Curl, opt, info);
}

static void* ProcessEventDetour(UObject* pObj, UFunction* pFunc, void* pParams)
{
    if (pObj && pFunc)
    {
        auto decObj = pObj->GetName();
        auto decFunc = pFunc->GetName();

        if (decFunc.find(XOR(L"SendClientHello")) != std::string::npos ||
            decFunc.find(XOR(L"SendPacketToServer")) != std::string::npos ||
            decFunc.find(XOR(L"SendPacketToClient")) != std::string::npos )
        {

            //std::cout << "Func: " << decFunc << " Index: " << Function->InternalIndex << " ClassPrivate: " << Function->ClassPrivate << " ObjectFlags: " << Function->ObjectFlags << " OuterPrivate: " << Function->OuterPrivate << " VTable: " << Function->VTable << std::endl;
            //std::cout << "Object: " << decObject << " Index: " << Object->InternalIndex << " ClassPrivate: " << Object->ClassPrivate << " ObjectFlags: " << Object->ObjectFlags << " OuterPrivate: " << Object->OuterPrivate << " VTable: " << Object->VTable << std::endl;
            //std::cout << "Params: " << Params << std::endl;         
            return NULL;
        }
        /* else if (decFunc.find(XOR(L"BP_OnDeactivated")) && decObj.find(XOR(L"PickerOverlay_EmoteWheel")))
        {
            ObjectFinder PlayerControllerFinder = ObjectFinder::EntryPoint(uintptr_t(GetPlayerController()));

            ObjectFinder LastEmotePlayedFinder = PlayerControllerFinder.Find(XOR(L"LastEmotePlayed"));

            auto LastEmotePlayed = LastEmotePlayedFinder.GetObj();

            if (LastEmotePlayed && !IsBadReadPtr(LastEmotePlayed))
            {
                Player::Emote(LastEmotePlayed);
                //emoteLoc = NeoPlayer.GetLocation();
            }
        } */
        else if (//decFunc.find(XOR(L"ServerReadyToStartMatch")) != std::string::npos ||
                 decFunc.find(XOR(L"ServerLoadingScreenDropped")) != std::string::npos)
        {
            Console::CreateCheatManager(); // idk if works too lazy
        }

        return ProcessEvent(pObj, pFunc, pParams);
    }
}