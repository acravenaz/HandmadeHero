#ifndef HANDMADE_MEMORY_H
#define HANDMADE_MEMORY_H
/*  ========================================================================
	$File: $
	$Date: $
	$Revision: $
	$Creator: Casey Muratori $
	$Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
	======================================================================== */

typedef struct thread_context
{
	int Placeholder;
} thread_context;

#if HANDMADE_INTERNAL
/* NOTE(casey):

These are NOT for doing anything in the shipping game - they are
blocking and the write doesn't protect against lost data!
*/
struct debug_read_file_result
{
	uint32 ContentsSize;
	void *Contents;
};

#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(thread_context *Thread, void *Memory)
typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platform_free_file_memory);

#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) debug_read_file_result name(thread_context *Thread, char *Filename)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);

#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) bool32 name(thread_context *Thread, char *Filename, uint32 MemorySize, void *Memory)
typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platform_write_entire_file);

#endif

typedef size_t memory_index;

struct memory_region
{
	memory_index RegionSize;
	memory_index UsedBytes;
	uint8 *Base;
};

struct game_memory
{
	bool32 IsInitialized;

	uint64 PermanentStorageSize;
	void *PermanentStorage; // NOTE(casey): REQUIRED to be cleared to zero at startup

	uint64 TransientStorageSize;
	void *TransientStorage; // NOTE(casey): REQUIRED to be cleared to zero at startup

	debug_platform_free_file_memory *DEBUGPlatformFreeFileMemory;
	debug_platform_read_entire_file *DEBUGPlatformReadEntireFile;
	debug_platform_write_entire_file *DEBUGPlatformWriteEntireFile;
};
#endif