/*  ========================================================================
    $File: $
    $Date: $
    $Revision: $
    $Creator: Casey Muratori $
    $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
    ======================================================================== */

#include "handmade_memory.h"

internal void 
InitializeMemoryRegion(memory_region *Region, memory_index RegionSize, uint8 *Base)
{
	Region->RegionSize = RegionSize;
	Region->Base = Base;
	Region->UsedBytes = 0;
}

#define ReserveMemory(Region, type) (type *)ReserveMemory_(Region, sizeof(type))
#define ReserveArray(Region, Count, type) (type *)ReserveMemory_(Region, (Count)*sizeof(type))
void *ReserveMemory_(memory_region *Region, memory_index RegionSize)
{
	ASSERT(Region->UsedBytes + RegionSize <= Region->RegionSize);
	void *Result = Region->Base + Region->UsedBytes;
	Region->UsedBytes += RegionSize;

	return (Result);
}