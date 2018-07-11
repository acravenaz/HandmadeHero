#ifndef HANDMADE_H
#define HANDMADE_H
/*  ========================================================================
	$File: $
	$Date: $
	$Revision: $
	$Creator: Casey Muratori $
	$Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. 
	======================================================================== */


// TODO(casey): Implement sine ourselves

#include "handmade_platform.h"
#include "handmade_intrinsics.h"
#include "handmade_math.h"
#include "handmade_map.h"
#include "handmade_entity.h"

#define DEBUG_SPAM_ENTITIES_COUNT 10

#pragma pack(push, 1)
struct bitmap_header
{
	uint16 FileType;
	uint32 FileSize;
	uint16 Reserved1;
	uint16 Reserved2;
	uint32 BitmapOffset;
	uint32 HeaderSize;
	int32 Width;
	int32 Height;
	uint16 Planes;
	uint16 BitsPerPixel;
	uint32 Compression;
	uint32 SizeOfBitmap;
	int32 HorzResolution;
	int32 VertResolution;
	uint32 ColorsUsed;
	uint32 ColorsImportant;

	uint32 RedMask;
	uint32 GreenMask;
	uint32 BlueMask;
};
#pragma pack(pop)

enum alpha_mode
{
	Alpha_None,
	Alpha_Test,
	Alpha_Blend
};

struct bitmap
{
	uint32 *Pixels;
	bitmap_header *Header;
	alpha_mode DrawMode;
};

struct color 
{
	real32 R;
	real32 G;
	real32 B;
};

struct game_state
{
	uint32 ControllerPlayerIndex[ArrayCount(((game_input *)0)->Controllers)];

	entity_space EntitySpaces[2048];
	entity_info EntityInfos[2048];
	entity_low LowEntities[2048];
	entity_high HighEntities[2048];

	uint32 EntityCount;

	ai_controller AIControllers[DEBUG_SPAM_ENTITIES_COUNT - 1];

	//TODO(Andrew): Support multiple cameras(Splitscreen?)
	camera Camera;
	
	bitmap EmptyTile;
	bitmap TestBitmap;
	bitmap Grass[3];
	bitmap Dirt[3];
	bitmap Gravel[3];
	//bitmap Wall;
	bitmap StairUp;
	bitmap StairDown;
	bitmap EntityBitmaps[5][4];
	world_map *WorldMap;
	real32 tSine;
};
#include "handmade_entity.cpp"
#include "handmade_map.cpp"
#endif