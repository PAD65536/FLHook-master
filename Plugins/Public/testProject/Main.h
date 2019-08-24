#ifndef __MAIN_H__
#define __MAIN_H__ 1

#include <windows.h>
#include <stdio.h>
#include <string>
#include <time.h>
#include <math.h>
#include <list>
#include <map>
#include <algorithm>
#include <FLHook.h>
#include <plugin.h>
#include <PluginUtilities.h>

using namespace std;

namespace ACI
{
	void LoadSettings();
	void CheckItems(unsigned int iClientID);
	void ClearClientInfo(unsigned int iClientID);
	void CheckOwned(unsigned int iClientID);
	void UpdatePlayerID(unsigned int iClientID);
	bool CanDock(uint iDockTarget, uint iClientID);
	uint GetCustomLastBaseForClient(unsigned int client);
	void MoveClient(unsigned int client, unsigned int targetBase);
	void StoreReturnPointForClient(unsigned int client);
}


#endif
