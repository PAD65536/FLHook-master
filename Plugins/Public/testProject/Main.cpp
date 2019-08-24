// Events for FLHookPlugin
// December 2015 by BestDiscoveryHookDevs2015
//
// 
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <float.h>
#include <stdio.h>
#include <string>
#include <time.h>
#include <math.h>
#include <list>
#include <map>
#include <algorithm>
#include <FLHook.h>
#include <plugin.h>
#include "Main.h"
#include <sstream>
#include <iostream>
#include <hookext_exports.h>
#include "minijson_writer.hpp"
#include <set>
#include <PluginUtilities.h>
//��¼��ֻ����
map <uint, wstring> shipclassnames;

static list<uint> idlist;
//�ж��Ƿ�Ϊ����Ա
bool isAdmin = false;
//����ս�������ж�
bool factionEnable = false;
//��¼����
map <wstring, wstring> faction;
//wstring faction1 = L"";
//wstring faction2 = L"";
//������¼
map <wstring, uint> factionScore;
//uint faction1Score = 0;
//uint faction2Score = 0;
//��¼��������
uint factionCount;
void LoadSettings();

FILE *FactionWarLogfile = fopen("./flhook_logs/factionWar.log", "at");

void factionWarLogging(const char *szString, ...)
{
	char szBufString[2048];
	va_list marker;
	va_start(marker, szString);
	_vsnprintf(szBufString, sizeof(szBufString) - 1, szString, marker);

	if (FactionWarLogfile) {
		char szBuf[64];
		time_t tNow = time(0);
		struct tm *t = localtime(&tNow);
		strftime(szBuf, sizeof(szBuf), "%d/%m/%Y %H:%M:%S", t);
		fprintf(FactionWarLogfile, "%s %s\n", szBuf, szBufString);
		fflush(FactionWarLogfile);
		fclose(FactionWarLogfile);
	}
	else {
		ConPrint(L"Failed to write FactionWar log! This might be due to inability to create the directory - are you running as an administrator?\n");
	}
	FactionWarLogfile = fopen("./flhook_logs/factionWar.log", "at");
}


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	srand((uint)time(0));
	// If we're being loaded from the command line while FLHook is running then
	// set_scCfgFile will not be empty so load the settings as FLHook only
	// calls load settings on FLHook startup and .rehash.
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		if (set_scCfgFile.length()>0)
			LoadSettings();
	}
	else if (fdwReason == DLL_PROCESS_DETACH)
	{
		LoadSettings();
	}
	return true;
}

void LoadSettings()
{
	// The path to the configuration file.
	char szCurDir[MAX_PATH];
	GetCurrentDirectory(sizeof(szCurDir), szCurDir);
	string scPluginCfgFile = string(szCurDir) + "\\flhook_plugins\\testProject.cfg";

	INI_Reader ini;
	if (ini.open(scPluginCfgFile.c_str(), false))
	{
		while (ini.read_header())
		{
			if (ini.is_header("commandpermission"))
			{
				while (ini.read_value())
				{
					if (ini.is_value("id"))
					{
						idlist.push_back(CreateID(ini.get_value_string(0)));
					}
				}
			}
			else if (ini.is_header("shipclasses"))
			{
				while (ini.read_value())
				{
					if (ini.is_value("class"))
					{
						shipclassnames[ini.get_value_int(0)] = stows(ini.get_value_string(1));
					}
				}
			}
		}
		ini.close();

	}
}

/// A return code to indicate to FLHook if we want the hook processing to continue.
PLUGIN_RETURNCODE returncode;



/// Hook will call this function after calling a plugin function to see if we the
/// processing to continue
EXPORT PLUGIN_RETURNCODE Get_PluginReturnCode()
{
	return returncode;
}
//WStringתString
std::string WString2String(const std::wstring& ws)
{
	std::string strLocale = setlocale(LC_ALL, "");
	const wchar_t* wchSrc = ws.c_str();
	size_t nDestSize = wcstombs(NULL, wchSrc, 0) + 1;
	char *chDest = new char[nDestSize];
	memset(chDest, 0, nDestSize);
	wcstombs(chDest, wchSrc, nDestSize);
	std::string strResult = chDest;
	delete[]chDest;
	setlocale(LC_ALL, strLocale.c_str());
	return strResult;
}


/// Hook for ship distruction. It's easier to hook this than the PlayerDeath one.
//�����ɱ����
/*
void SendDeathMsg(const wstring &wscMsg, uint iSystem, uint iClientIDVictim, uint iClientIDKiller)
{
	returncode = DEFAULT_RETURNCODE;

	if (!factionEnable)
		return;
	//PrintUserCmdText(iClientIDKiller, L"��һ��Ѫ������");
	//PrintUserCmdText(iClientIDVictim, L"ʤ���˱��ҳ��£��������������� �ж��Ƿ�Ϊ��ɱ%s", (const wchar_t*)Players.GetActiveCharacterName(iClientIDKiller));
	//�����ϵ��ҷ���ս����Ϣ

	uint iSystemID;
	pub::Player::GetSystem(iClientIDVictim, iSystemID);
	Archetype::Ship *ship = Archetype::GetShip(Players[iClientIDVictim].iShipArchetype);
	uint currshipclass = ship->iShipClass;
	uint killScore = currshipclass + 1;

	if (wcsstr((const wchar_t*)Players.GetActiveCharacterName(iClientIDVictim), faction1.c_str())) {
		if (!(const wchar_t*)Players.GetActiveCharacterName(iClientIDKiller)) {
			wstring wscMsg = L"%faction1������ɱ����%faction2  ��ö���+1�ֲ�������";
			wscMsg = ReplaceStr(wscMsg, L"%faction1", faction1);
			wscMsg = ReplaceStr(wscMsg, L"%faction2", faction2);
			struct PlayerData *pPD = 0;
			while (pPD = Players.traverse_active(pPD))
			{
				uint iClientID = HkGetClientIdFromPD(pPD);
				uint iClientSystemID = 0;
				pub::Player::GetSystem(iClientID, iClientSystemID);
				//if (iSystemID == iClientSystemID)
				//{
				// Send the message a player in this system.
				PrintUserCmdText(iClientID, wscMsg);
				//}
			}
			string scText = WString2String(wscMsg);
			factionWarLogging("%s", scText.c_str());
			faction2Score += (killScore + 1);
		}
		else {

			wstring wscMsg2 = L"���%player1��ɱ�����%player2��%shipclass(+%killScore��)";
			wscMsg2 = ReplaceStr(wscMsg2, L"%player1", (const wchar_t*)Players.GetActiveCharacterName(iClientIDKiller));
			wscMsg2 = ReplaceStr(wscMsg2, L"%player2", (const wchar_t*)Players.GetActiveCharacterName(iClientIDVictim));
			wscMsg2 = ReplaceStr(wscMsg2, L"%shipclass", shipclassnames[currshipclass]);
			wscMsg2 = ReplaceStr(wscMsg2, L"%killScore", stows(itos(killScore)));
			struct PlayerData *pPD = 0;
			while (pPD = Players.traverse_active(pPD))
			{
				uint iClientID = HkGetClientIdFromPD(pPD);
				uint iClientSystemID = 0;
				pub::Player::GetSystem(iClientID, iClientSystemID);
				//if (iSystemID == iClientSystemID)
				//{
				// Send the message a player in this system.
				PrintUserCmdText(iClientID, wscMsg2);
				//}
			}
			string scText = WString2String(wscMsg2);
			factionWarLogging("%s", scText.c_str());
			faction2Score += killScore;
		}
	}
	if (wcsstr((const wchar_t*)Players.GetActiveCharacterName(iClientIDVictim), faction2.c_str())) {
		if (!(const wchar_t*)Players.GetActiveCharacterName(iClientIDKiller)) {
			wstring wscMsg = L"%faction2������ɱ����%faction1  ��ö���+1�ֲ�������";
			wscMsg = ReplaceStr(wscMsg, L"%faction1", faction1);
			wscMsg = ReplaceStr(wscMsg, L"%faction2", faction2);
			struct PlayerData *pPD = 0;
			while (pPD = Players.traverse_active(pPD))
			{
				uint iClientID = HkGetClientIdFromPD(pPD);
				uint iClientSystemID = 0;
				pub::Player::GetSystem(iClientID, iClientSystemID);
				//if (iSystemID == iClientSystemID)
				//{
				// Send the message a player in this system.
				PrintUserCmdText(iClientID, wscMsg);
				//}
			}
			string scText = WString2String(wscMsg);
			factionWarLogging("%s", scText.c_str());
			faction1Score += (killScore + 1);
		}
		else {
			wstring wscMsg2 = L"���%player1��ɱ�����%player2��%shipclass(+%killScore��)";
			wscMsg2 = ReplaceStr(wscMsg2, L"%player1", (const wchar_t*)Players.GetActiveCharacterName(iClientIDKiller));
			wscMsg2 = ReplaceStr(wscMsg2, L"%player2", (const wchar_t*)Players.GetActiveCharacterName(iClientIDVictim));
			wscMsg2 = ReplaceStr(wscMsg2, L"%shipclass", shipclassnames[currshipclass]);
			wscMsg2 = ReplaceStr(wscMsg2, L"%killScore", stows(itos(killScore)));
			struct PlayerData *pPD = 0;
			while (pPD = Players.traverse_active(pPD))
			{
				uint iClientID = HkGetClientIdFromPD(pPD);
				uint iClientSystemID = 0;
				pub::Player::GetSystem(iClientID, iClientSystemID);
				//if (iSystemID == iClientSystemID)
				//{
				// Send the message a player in this system.
				PrintUserCmdText(iClientID, wscMsg2);
				//}
			}
			string scText = WString2String(wscMsg2);
			faction1Score += killScore;
		}
	}
	wstring wscMsg1 = L"���ڵ÷�״��Ϊ��%faction1��%Score1 %faction2: %Score2";
	wscMsg1 = ReplaceStr(wscMsg1, L"%faction1", faction1);
	wscMsg1 = ReplaceStr(wscMsg1, L"%faction2", faction2);
	wscMsg1 = ReplaceStr(wscMsg1, L"%Score1", stows(itos(faction1Score)));
	wscMsg1 = ReplaceStr(wscMsg1, L"%Score2", stows(itos(faction2Score)));
	struct PlayerData *pPD = 0;
	while (pPD = Players.traverse_active(pPD))
	{
		uint iClientID = HkGetClientIdFromPD(pPD);
		uint iClientSystemID = 0;
		pub::Player::GetSystem(iClientID, iClientSystemID);
		//if (iSystemID == iClientSystemID)
		//{
		// Send the message a player in this system.
		PrintUserCmdText(iClientID, wscMsg1);
		//}
	}
	string scText = WString2String(wscMsg1);
	factionWarLogging("%s", scText.c_str());
}
*/
/*
void __stdcall ShipDestroyed(DamageList *_dmg, DWORD *ecx, uint iKill)
{
returncode = DEFAULT_RETURNCODE;
//CShip *cship = (CShip*)ecx[4];

//todo, combat event npc destruction
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Client command processing
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//���ڶԷ�����ս��Ա������Ӧ����

namespace HkIServerImpl {
	void __stdcall JumpInComplete_AFTER(unsigned int iSystem, unsigned int iShip)
	{
		returncode = DEFAULT_RETURNCODE;
		uint iClientID = HkGetClientIDByShip(iShip);
		wstring wscMsg = L"%time ���չ�����Ϣ��%player������ͣ����";
		wscMsg = ReplaceStr(wscMsg, L"%player", (const wchar_t*)Players.GetActiveCharacterName(iClientID));
		PrintUserCmdText(iClientID, L"���ո������һ����Ծ");
	}
	void __stdcall CharacterSelect_AFTER(struct CHARACTER_ID const &charId, unsigned int iClientID)
	{
		returncode = DEFAULT_RETURNCODE;
		PrintUserCmdText(iClientID, L"��¼�ɹ�");
		uint iSystemID;
		pub::Player::GetSystem(iClientID, iSystemID);
		struct PlayerData *pPD = 0;
		//��ȡ��ֻ����
		Archetype::Ship *ship = Archetype::GetShip(Players[iClientID].iShipArchetype);
		uint currshipclass = ship->iShipClass;
		wstring wscMsg1 = L"����VIP�û�%player��������%shipclass����÷�����";
		wscMsg1 = ReplaceStr(wscMsg1, L"%player", (const wchar_t*)Players.GetActiveCharacterName(iClientID));
		wscMsg1 = ReplaceStr(wscMsg1, L"%shipclass", shipclassnames[currshipclass]);
		/*while (pPD = Players.traverse_active(pPD))
		{
		uint iClientID = HkGetClientIdFromPD(pPD);
		uint iClientSystemID = 0;
		pub::Player::GetSystem(iClientID, iClientSystemID);
		if (iSystemID == iClientSystemID)
		{
		// Send the message a player in this system.
		PrintUserCmdText(iClientID, wscMsg1);
		}
		}*/
		PrintUserCmdText(iClientID, wscMsg1);
	}
}

//��ȡ�û�ָ���������ս���Ŀ��͹�
bool  UserCmd_SystemTester(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage)
{
	/*wstring wscReputationID = GetParam(wscParam, L' ', 0);
	PrintUserCmdText(iClientID, L"��⵽�����IDΪ" + wscReputationID);
	char szCurDir[MAX_PATH];
	GetCurrentDirectory(sizeof(szCurDir), szCurDir);
	string scPluginCfgFile = string(szCurDir) + "\\flhook_plugins\\alley.cfg";*/
	//��һ���������Ƿ�����
	wstring wscFactionEnable = GetParam(wscParam, L' ', 0);
	//�ڶ�������������1Tag
	//wstring wscFactionTag1 = GetParam(wscParam, L' ', 1);
	//����������������2Tag
	//wstring wscFactionTag2 = GetParam(wscParam, L' ', 2);
	wstring wscFactionTag;
	//�ж�ʹ��ָ����û��Ƿ�Ϊ����Ա
	int iHoldSize;
	list<CARGO_INFO> lstCargo;
	HkEnumCargo((const wchar_t*)Players.GetActiveCharacterName(iClientID), lstCargo, iHoldSize);

	foreach(lstCargo, CARGO_INFO, i)
	{
		if (i->bMounted)
		{
			// is it a good id
			list<uint>::iterator iter = idlist.begin();
			while (iter != idlist.end())
			{
				if (*iter == i->iArchID)
				{
					isAdmin = true;
					break;
				}
				iter++;
			}
		}
	}
	if (!isAdmin) {
		PrintUserCmdText(iClientID, L"��������Ϸ����Ա������ʹ�ô�����");
		return false;
	}
	//�ж�
	if (wscFactionEnable == L"����") {
		factionCount = 1;
		while (!GetParam(wscParam, L' ', factionCount).empty()) {
			wscFactionTag = GetParam(wscParam, L' ', factionCount);
			faction[wscFactionTag] = wscFactionTag;
			factionScore[wscFactionTag] = 0;
		}
		/*if (!wscFactionTag1.empty()) {
			faction1 = wscFactionTag1;
		}
		else {
			PrintUserCmdText(iClientID, L"������������1Tag����Ϊ��");
			PrintUserCmdText(iClientID, L"��ȷ�÷���/����ս�� <����>|<�ر�> <����1Tag> <����2Tag>");
			return false;
		}
		if (!wscFactionTag2.empty()) {
			faction2 = wscFactionTag2;
		}
		else {
			PrintUserCmdText(iClientID, L"������������2Tag����Ϊ��");
			PrintUserCmdText(iClientID, L"��ȷ�÷���/����ս�� <����>|<�ر�> <����1Tag> <����2Tag>");
			return false;
		}*/
		wstring wscMsg = L"��ս˫��Ϊ %faction1 �� %faction2 ";
		wscMsg = ReplaceStr(wscMsg, L"%faction1", faction1);
		wscMsg = ReplaceStr(wscMsg, L"%faction2", faction2);
		PrintUserCmdText(iClientID, L"����ս��������");
		PrintUserCmdText(iClientID, wscMsg);
		factionEnable = true;
	}
	else if (wscFactionEnable == L"�ر�") {
		PrintUserCmdText(iClientID, L"����ս���ѹر�");
		factionEnable = false;
		isAdmin = false;
		//faction1Score = 0;
		//faction2Score = 0;
		return false;
	}
	else {
		PrintUserCmdText(iClientID, L"��������");
		PrintUserCmdText(iClientID, L"��ȷ�÷���/����ս�� <����>|<�ر�> <����1Tag> <����2Tag>");
		return false;
	}
	return true;
}

typedef bool(*_UserCmdProc)(uint, const wstring &, const wstring &, const wchar_t*);

struct USERCMD
{
	wchar_t *wszCmd;
	_UserCmdProc proc;
	wchar_t *usage;
};
//��������
USERCMD UserCmds[] =
{
	{ L"/����ս��", UserCmd_SystemTester, L"Usage: /����ս�� <����>|<�ر�> <����1TAG> <����2TAG>" },
	{ L"/����ս��*", UserCmd_SystemTester, L"Usage: /����ս�� <����>|<�ر�> <����1TAG> <����2TAG>" },
};


/*
This function is called by FLHook when a user types a chat string. We look at the
string they've typed and see if it starts with one of the above commands. If it
does we try to process it.
*/

//�����û�ָ��
bool UserCmd_Process(uint iClientID, const wstring &wscCmd)
{
	returncode = DEFAULT_RETURNCODE;

	wstring wscCmdLineLower = ToLower(wscCmd);

	// If the chat string does not match the USER_CMD then we do not handle the
	// command, so let other plugins or FLHook kick in. We require an exact match
	for (uint i = 0; (i < sizeof(UserCmds) / sizeof(USERCMD)); i++)
	{

		if (wscCmdLineLower.find(UserCmds[i].wszCmd) == 0)
		{
			// Extract the parameters string from the chat string. It should
			// be immediately after the command and a space.
			wstring wscParam = L"";
			if (wscCmd.length() > wcslen(UserCmds[i].wszCmd))
			{
				if (wscCmd[wcslen(UserCmds[i].wszCmd)] != ' ')
					continue;
				wscParam = wscCmd.substr(wcslen(UserCmds[i].wszCmd) + 1);
			}

			// Dispatch the command to the appropriate processing function.
			if (UserCmds[i].proc(iClientID, wscCmd, wscParam, UserCmds[i].usage))
			{
				// We handled the command tell FL hook to stop processing this
				// chat string.
				returncode = SKIPPLUGINS_NOFUNCTIONCALL; // we handled the command, return immediatly
				return true;
			}
		}
	}
	return false;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Functions to hook
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT PLUGIN_INFO* Get_PluginInfo()
{
	PLUGIN_INFO* p_PI = new PLUGIN_INFO();
	p_PI->sName = "����ս����� ������ PAD";
	p_PI->sShortName = "����ս��";
	p_PI->bMayPause = true;
	p_PI->bMayUnload = true;
	p_PI->ePluginReturnCode = &returncode;
	p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC*)&UserCmd_Process, PLUGIN_UserCmd_Process, 0));
	//p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC*)&SendDeathMsg, PLUGIN_SendDeathMsg, 0));
	p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC*)&LoadSettings, PLUGIN_LoadSettings, 0));
	p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC*)&HkIServerImpl::JumpInComplete_AFTER, PLUGIN_HkIServerImpl_JumpInComplete_AFTER, 0));
	p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC*)&HkIServerImpl::CharacterSelect_AFTER, PLUGIN_HkIServerImpl_CharacterSelect_AFTER, 0));
	return p_PI;
}
