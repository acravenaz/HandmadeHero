#ifndef HANDMADE_MAP_H
#define HANDMADE_MAP_H
/*  ========================================================================
	$File: $
	$Date: $
	$Revision: $
	$Creator: Casey Muratori $
	$Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
	======================================================================== */

//
// NOTE(Andrew): Structures and #def's that govern position, tilemaps, world dimensions, etc
//


//NOTE: CHUNK_SHIFT determines the size of chunks in the worldspace!
//      The chunk size is 2^n where n is CHUNK_SHIFT
#define CHUNK_SHIFT 4
#define CHUNK_MASK (1 << CHUNK_SHIFT) - 1

#define CHUNK_WIDTH (1 << CHUNK_SHIFT)
#define CHUNK_HEIGHT (1 << CHUNK_SHIFT)

#define SCREEN_X_OFFSET 0
#define SCREEN_Y_OFFSET RENDER_HEIGHT

struct tile_position {
	uint32 TileX;
	uint32 TileY;
	uint32 TileZ;
	vec2d Offset;
};

struct tile
{
	uint32 TileID;
	uint32 Variant;
	//int32 Attributes[2];
	uint32 X;
	uint32 Y;
	uint32 Z;
	//bool32 Passable;
};

struct chunk
{
	tile *Tiles;
};

struct world_map
{
	memory_region MapMemory;
	chunk *Chunks;
	//NOTE(Andrew): Size of world map in Chunks. Refer to CHUNK_SIZE and CHUNK_SHIFT
	uint32 MapWidth;
	uint32 MapHeight;
	uint32 MapDepth;
	//NOTE(Andrew): Width and height of tiles in Pixels.
	uint32 TileWidth;
	uint32 TileHeight;
};
#endif