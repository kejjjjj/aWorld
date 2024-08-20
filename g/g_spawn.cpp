#include "g_spawn.hpp"

#include "cg/cg_local.hpp"
#include "cg/cg_offsets.hpp"

#include "dvar/dvar.hpp"
#include "scr/scr_functions.hpp"

#include "utils/engine.hpp"



#include <string.h>
#include <ranges>
#include "com/com_channel.hpp"

void G_InitGame()
{
	if (Dvar_FindMalleableVar("sv_running")->current.enabled)
		return;

	Engine::call<void>(0x4BD860); //G_RegisterDvars
	Engine::Tools::write_bytes(0x4DF9C0, "\xC3");

	level->initializing = 1;
	level->time = 0;
	level->startTime = level->time;
	level->currentEntityThink = -1;
	level->scriptPrintChannel = 24;

	level->currentScriptIOLineMark[0].lines = 0;
	level->currentScriptIOLineMark[0].text = 0;
	level->currentScriptIOLineMark[0].ungetToken = 0;
	level->currentScriptIOLineMark[0].backup_lines = 0;
	level->currentScriptIOLineMark[0].backup_text = 0;
	level->openScriptIOFileHandles[0] = 0;
	level->openScriptIOFileBuffers[0] = 0;

	memset(g_entities, 0, 0x9D000u);
	level->gentities = g_entities;
	level->maxclients = 64;
	memset(g_clients, 0, 0x0C6100);
	level->clients = g_clients;

	for (auto i = 0; i < level->maxclients; ++i)
		g_entities[i].client = &level->clients[i];

	level->num_entities = 72;
	level->firstFreeEnt = 0;
	level->lastFreeEnt = 0;

	sv->gentities = g_entities;
	sv->gentitySize = 628;
	sv->num_entities = 72;
	sv->gameClients = &g_clients->ps;
	sv->gameClientSize = 12676;

	G_SpawnEntitiesFromString();

}

void G_SpawnEntitiesFromString()
{
	G_ResetEntityParsePoint();

	__asm
	{
		mov esi, 0x4E0990;
		call esi;
	}

}
void G_ResetEntityParsePoint()
{
	*g_entityBeginParsePoint = cm->mapEnts->entityString;
	*g_entityEndParsePoint = *g_entityBeginParsePoint;
}

int G_ParseSpawnVars(SpawnVar* var)
{
	constexpr static auto vG_ParseSpawnVars = 0x4BD580;
	__asm { 
		mov eax, var; 
		call vG_ParseSpawnVars; 
	}
}

int G_GetItemForClassname(const char* classname)
{
	constexpr static auto f = 0x4DFE30;

	__asm
	{
		mov esi, classname;
		call f;
	}

}
void(__cdecl* __cdecl G_FindSpawnFunc(const SpawnFuncEntry* spawnFuncArray, const char* classname, int spawnFuncCount))(gentity_s*)
{
	for (auto spawnFuncIter = 0; spawnFuncIter < spawnFuncCount; ++spawnFuncIter)
	{
		if (!strcmp(classname, spawnFuncArray[spawnFuncIter].classname))
			return spawnFuncArray[spawnFuncIter].callback;
	}
	return 0;
}

bool G_GetSpawnItemIndex(const gentity_s* gent)
{

	const char* classname = 0;
	const auto G_SpawnString = [&](const SpawnVar* spawnVar, const char* key, const char* defaultString, const char** out) {
		return Engine::call<int>(0x4BD700, spawnVar, key, defaultString, out); };
	
	//
	//SpawnFuncEntry* s_bspOrDynamicSpawns = reinterpret_cast<SpawnFuncEntry*>(0x6BC1D8);
	//SpawnFuncEntry* s_bspOnlySpawns = reinterpret_cast<SpawnFuncEntry*>(0x6BC168);


	G_SpawnString(&level->spawnVar, "classname", "", &classname);

	return classname && !strcmp(classname, Scr_GetString(gent->classname));

	//if (!classname || strncmp(classname, "dyn_", 4)) {
	//	continue;
	//}

	//if (G_GetItemForClassname(classname)) {
	//	continue;
	//}

	//auto spawnFunc = G_FindSpawnFunc(s_bspOrDynamicSpawns, classname, 6);
	//if (!spawnFunc)
	//	spawnFunc = G_FindSpawnFunc(s_bspOnlySpawns, classname, 14);
	//	
	//if (spawnFunc != decltype(spawnFunc)(0x4E3A50)) {
	//	currentNumber++;
	//}



	//return currentNumber;

}

SpawnVar* G_GetGentitySpawnVars(const gentity_s* gent)
{
	SpawnVar* var = &level->spawnVar;
	auto parsed = false;

	while (parsed = G_ParseSpawnVars(var), parsed){
		if (G_GetSpawnItemIndex(gent))
			break;
	};

	return !parsed ? nullptr : var;
}
