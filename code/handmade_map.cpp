/*  ========================================================================
    $File: $
    $Date: $
    $Revision: $
    $Creator: Casey Muratori $
    $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
    ======================================================================== */

#include "handmade_map.h"

internal void 
FillChunk(game_state *GameState, chunk *Chunk, int XPos, int YPos, int ZPos)
{
	world_map *Map = GameState->WorldMap;
	Chunk->Tiles = ReserveArray(&Map->MapMemory, CHUNK_WIDTH * CHUNK_HEIGHT, tile);
	for (int Row = 0; Row < CHUNK_HEIGHT; Row++)
	{
		for (int Column = 0; Column < CHUNK_WIDTH; Column++)
		{
			uint32 DefaultID = 0;
			if (ZPos == 0)
			{
				DefaultID = 2;
			}
			else if (ZPos == 1)
			{
				DefaultID = 3;
			}
			else if (ZPos == 2)
			{
				DefaultID = 4;
			}
			tile *Tile = &Chunk->Tiles[Row*CHUNK_WIDTH + Column];
			Tile->X = Column + (XPos * CHUNK_WIDTH);
			Tile->Y = Row + (YPos * CHUNK_HEIGHT);
			Tile->Z = ZPos;
			if (Row == 0 || Column == 0 || Row == CHUNK_HEIGHT - 1 || Column == CHUNK_WIDTH - 1)
			{
				Tile->TileID = DefaultID;
				if (Row == CHUNK_HEIGHT / 2 || Row == (CHUNK_HEIGHT / 2) - 1 || 
					Column == CHUNK_WIDTH / 2 || Column == (CHUNK_WIDTH / 2) - 1)
				{
					if (Tile->X == 0 || Tile->Y == 0 || 
						Tile->X == (Map->MapWidth * CHUNK_WIDTH) - 1 ||
						Tile->Y == (Map->MapHeight * CHUNK_HEIGHT) - 1)
					{
						//Tile->TileID = 1;
						uint32 NewWallID = InitializeEntity(GameState);
						InitializeWall(GameState, NewWallID, Tile->X, Tile->Y, Tile->Z);
					}
				}
				else {
					//Tile->TileID = 1;
					uint32 NewWallID = InitializeEntity(GameState);
					InitializeWall(GameState, NewWallID, Tile->X, Tile->Y, Tile->Z);
				}
			}
			else {
				Tile->TileID = DefaultID;
			}
			if (Row == CHUNK_HEIGHT - 2 && Column == CHUNK_WIDTH - 2)
			{
				//stairs up
				//Tile->TileID = 5;
				uint32 NewStairID = InitializeEntity(GameState);
				InitializeStair(GameState, NewStairID, Tile->X, Tile->Y, Tile->Z, true);
			}
			if (Row == CHUNK_HEIGHT - 2 && Column == 1)
			{
				//stairs down
				//Tile->TileID = 6;
				uint32 NewStairID = InitializeEntity(GameState);
				InitializeStair(GameState, NewStairID, Tile->X, Tile->Y, Tile->Z, false);
			}
			if (Tile->TileID == 2 || Tile->TileID == 3 || Tile->TileID == 4)
			{
				Tile->Variant = rand() % 3;
			}
		}
	}
}

internal chunk*
GetChunk(game_state *GameState, uint32 ChunkX, uint32 ChunkY, uint32 ChunkZ)
{
	world_map *Map = GameState->WorldMap;
	chunk *Chunk = &Map->Chunks[ChunkZ*Map->MapHeight*Map->MapHeight + ChunkY*Map->MapHeight + ChunkX];
	if (Chunk->Tiles)
	{
		return (Chunk);
	}
	else {
		FillChunk(GameState, Chunk, ChunkX, ChunkY, ChunkZ);
		return (Chunk);
		//return (NULL);
	}
}

internal tile* 
GetTile(game_state *GameState, uint32 TileX, uint32 TileY, uint32 TileZ)
{
	world_map *Map = GameState->WorldMap;
	if (TileX < Map->MapHeight * CHUNK_WIDTH &&
		TileY < Map->MapHeight * CHUNK_HEIGHT &&
		TileZ < Map->MapDepth)
	{
		uint32 ChunkX = TileX >> CHUNK_SHIFT;
		uint32 ChunkY = TileY >> CHUNK_SHIFT;
		chunk *Chunk = GetChunk(GameState, ChunkX, ChunkY, TileZ);
		if (Chunk)
		{
			uint32 ChunkTileX = TileX & CHUNK_MASK;
			uint32 ChunkTileY = TileY & CHUNK_MASK;
			tile *Tile = {};
			Tile = &Chunk->Tiles[ChunkTileY*CHUNK_WIDTH + ChunkTileX];
			return(Tile);
		}
		else
		{
			return (NULL);
		}
	}
	else
	{
		return(NULL);
	}
}

internal tile*
GetTile(game_state *GameState, tile_position *P)
{
	tile *Result = GetTile(GameState, P->TileX, P->TileY, P->TileZ);
	return (Result);
}

inline bool32 
PositionsEqual(tile_position *A, tile_position *B)
{
	bool32 PositionsEqual = false;
	if (A->TileX == B->TileX &&
		A->TileY == B->TileY &&
		A->TileZ == B->TileZ)
	{
		PositionsEqual = true;
	}
	return (PositionsEqual);
}

internal bool32 
IsTilePassable(tile *Tile)
{
	bool32 Result = false;
	if (Tile != NULL)
	{
		if (Tile->TileID != 1)
		{
			Result = true;
		}
	}
	return (Result);
}

internal bool32 
IsTilePassable(game_state *GameState, tile_position *P)
{
	bool32 Result = false;
	tile *Tile = GetTile(GameState, P);
	Result = IsTilePassable(Tile);
	return (Result);
}

internal vec2d 
TileDiff(tile_position *A, tile_position *B)
{
	vec2d Result = {(real32)((int32)(A->TileX - B->TileX)), 
					(real32)((int32)(A->TileY - B->TileY))};
	Result += (A->Offset - B->Offset);
	return (Result);
}

inline tile_position 
CreatePosition(uint32 TileX, uint32 TileY, uint32 TileZ)
{
	tile_position Result = {};
	Result.TileX = TileX;
	Result.TileY = TileY;
	Result.TileZ = TileZ;
	Result.Offset = {0.0f, 0.0f};
	return(Result);
}

inline void
RecanonicalizeCoord(uint32 *Tile, real32 *TileRel)
{
    // TODO(casey): Need to do something that doesn't use the divide/multiply method
    // for recanonicalizing because this can end up rounding back on to the tile
    // you just came from.

    // NOTE(casey): TileMap is assumed to be toroidal topology, if you
    // step off one end you come back on the other!
    int32 Offset = RoundReal32ToInt32(*TileRel);
    *Tile += Offset;
    *TileRel -= Offset;

    // TODO(casey): Fix floating point math so this can be exact?
    ASSERT(*TileRel > -0.5001f);
    ASSERT(*TileRel < 0.5001f);
}

inline tile_position
MapIntoTileSpace(tile_position Pos, vec2d Offset, uint32 TileWidth, uint32 TileHeight)
{
    tile_position Result = Pos;

	Offset.X = Offset.X / TileWidth;
	Offset.Y = Offset.Y / TileHeight;

	Result.Offset += Offset;
    RecanonicalizeCoord(&Result.TileX, &Result.Offset.X);
    RecanonicalizeCoord(&Result.TileY, &Result.Offset.Y);
    
    return(Result);
}
