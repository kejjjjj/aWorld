#include "g_spawn.hpp"

#include "cg/cg_local.hpp"
#include "cg/cg_offsets.hpp"

#include "dvar/dvar.hpp"

#include "utils/engine.hpp"

#include <string.h>

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

