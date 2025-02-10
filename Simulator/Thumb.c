/*===============================================================================
wxSimCM0

Simulator for a generic ARM Cortex-M0 processor with a few onboard peripherals
based on a previous simulator for a fictitious processor also written by me
(<https://github.com/Ibavaresco/wxCesar>).

Written using Code::Blocks 20.03 and
wxFormBuilder v4.2.1 using the library wxWidgets v3.2.5
=================================================================================
Copyright (c) 2024-2025, Isaac Marino Bavaresco
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Neither the name of the author nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.;;

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ''AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
==============================================================================*/
/*============================================================================*/
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "Thumb.h"
//#include "Intrinsics.h"
/*============================================================================*/
#if 0
#define DBG_printf( ... )  fprintf( stderr, __VA_ARGS__ );
#else
#define DBG_printf( ... )
#endif

/*============================================================================*/
#if 1
#define INLINE inline __attribute(( always_inline ))
#else
#define INLINE
#endif
/*============================================================================*/
/*============================================================================*/
/*============================================================================*/
/*============================================================================*/
sim_memoryarea_t *GetArea( const thumbstate_t *State, sim_address_t Address )
	{
	int	i;

	for( i = 0; i < State->NumMemoryAreas; i++ )
		if( State->MemoryAreas[i] != NULL && Address >= State->MemoryAreas[i]->BaseAddress && Address <= State->MemoryAreas[i]->LastAddress )
			return State->MemoryAreas[i];

	return NULL;
	}
/*============================================================================*/
int CheckExecBreakpoints( const thumbstate_t *State, sim_address_t Address, unsigned Length )
	{
	const sim_memoryarea_t	*Area;

	/* Get the memory area to which this address belongs. But it does not belong to any memory area... */
	if(( Area = GetArea( State, Address )) == NULL || Area->Map == NULL )
		/* ...so it cannot have any breakpoints. */
		return 0;

	/* Search for breakpoints in the address range... */
	for( ; Address <= Area->LastAddress && Length > 0; Address++, Length-- )
		if( Area->Map[Address - Area->BaseAddress].BreakpointExecute )
			return 1;

	return 0;
	}
/*============================================================================*/
int CheckReadBreakpoints( const thumbstate_t *State, sim_address_t Address, unsigned Length )
	{
	const sim_memoryarea_t	*Area;

	/* Get the memory area to which this address belongs. But it does not belong to any memory area... */
	if(( Area = GetArea( State, Address )) == NULL || Area->Map == NULL )
		/* ...so it cannot have any breakpoints. */
		return 0;

	/* Search for breakpoints in the address range... */
	for( ; Address <= Area->LastAddress && Length > 0; Address++, Length-- )
		if( Area->Map[Address - Area->BaseAddress].BreakpointRead )
			return 1;

	return 0;
	}
/*============================================================================*/
int CheckWriteBreakpoints( const thumbstate_t *State, sim_address_t Address, unsigned Length )
	{
	const sim_memoryarea_t	*Area;

	/* Get the memory area to which this address belongs. But it does not belong to any memory area... */
	if(( Area = GetArea( State, Address )) == NULL || Area->Map == NULL )
		/* ...so it cannot have any breakpoints. */
		return 0;

	/* Search for breakpoints in the address range... */
	for( ; Address <= Area->LastAddress && Length > 0; Address++, Length-- )
		if( Area->Map[Address - Area->BaseAddress].BreakpointWrite )
			return 1;

	return 0;
	}
/*============================================================================*/
sim_mapentry_t *GetMapEntry2( const thumbstate_t *State, sim_address_t Address )
	{
	int	i;

	for( i = 0; i < State->NumMemoryAreas; i++ )
		if( State->MemoryAreas[i] != NULL && State->MemoryAreas[i]->Map != NULL && Address >= State->MemoryAreas[i]->BaseAddress && Address <= State->MemoryAreas[i]->LastAddress )
			return &State->MemoryAreas[i]->Map[Address - State->MemoryAreas[i]->BaseAddress];

	return NULL;
	}
/*============================================================================*/
execstatus_t GetBaseAddress( const thumbstate_t *State, sim_address_t Address, sim_address_t *BaseAddress )
	{
	int	i;

	for( i = 0; i < State->NumMemoryAreas; i++ )
		if( State->MemoryAreas[i] != NULL && Address >= State->MemoryAreas[i]->BaseAddress && Address <= State->MemoryAreas[i]->LastAddress )
			{
			*BaseAddress	= State->MemoryAreas[i]->BaseAddress;
			return EXEC_STATUS_OK;
			}

	return EXEC_STATUS_ERROR_UNDEFINED_ADDRESS;
	}
/*============================================================================*/
execstatus_t GetLastAddress( const thumbstate_t *State, sim_address_t Address, sim_address_t *LastAddress )
	{
	int	i;

	for( i = 0; i < State->NumMemoryAreas; i++ )
		if( State->MemoryAreas[i] != NULL && Address >= State->MemoryAreas[i]->BaseAddress && Address <= State->MemoryAreas[i]->LastAddress )
			{
			*LastAddress	= State->MemoryAreas[i]->LastAddress;
			return EXEC_STATUS_OK;
			}

	return EXEC_STATUS_ERROR_UNDEFINED_ADDRESS;
	}
/*============================================================================*/
static INLINE execstatus_t StoreByte( thumbstate_t *State, uint32_t Address, uint8_t Value )
	{
	int	i;

	for( i = 0; i < State->NumMemoryAreas; i++ )
		if( State->MemoryAreas[i] != NULL && Address >= State->MemoryAreas[i]->BaseAddress && Address <= State->MemoryAreas[i]->LastAddress )
			{
			if( State->MemoryAreas[i]->WriteFunc != NULL )
				return State->MemoryAreas[i]->WriteFunc( State->MemoryAreas[i], Address, sizeof Value, &Value );

			sim_address_t	Index;

			if( State->MemoryAreas[i]->Flags.ReadOnly )
				return EXEC_STATUS_ERROR_READONLY_ADDRESS;

			Index											= Address - State->MemoryAreas[i]->BaseAddress;

			State->MemoryAreas[i]->Buffer[Index]			= Value;

			if( State->MemoryAreas[i]->Map != NULL )
				State->MemoryAreas[i]->Map[Index].Written	= 1;

			return EXEC_STATUS_OK;
			}

	return EXEC_STATUS_ERROR_UNDEFINED_ADDRESS;
	}
/*============================================================================*/
INLINE execstatus_t LoadByte( const thumbstate_t *State, uint32_t Address, uint8_t *Value )
	{
	int	i;

	for( i = 0; i < State->NumMemoryAreas; i++ )
		if( State->MemoryAreas[i] != NULL && Address >= State->MemoryAreas[i]->BaseAddress && Address <= State->MemoryAreas[i]->LastAddress )
			{
			if( State->MemoryAreas[i]->ReadFunc != NULL )
				return State->MemoryAreas[i]->ReadFunc( State->MemoryAreas[i], Address, sizeof *Value, &Value );

			sim_address_t	Index;

			Index										= Address - State->MemoryAreas[i]->BaseAddress;

			*Value										= State->MemoryAreas[i]->Buffer[Index];

			if( State->MemoryAreas[i]->Map != NULL )
				State->MemoryAreas[i]->Map[Index].Read	= 1;

			return EXEC_STATUS_OK;
			}

	return EXEC_STATUS_ERROR_UNDEFINED_ADDRESS;
	}
/*============================================================================*/
static INLINE execstatus_t StoreHalfWord( thumbstate_t *State, uint32_t Address, uint16_t Value )
	{
	int	i;

	if(( Address & 1 ) != 0 )
		return EXEC_STATUS_ERROR_UNALIGNED_ACCESS;

	for( i = 0; i < State->NumMemoryAreas; i++ )
		if( State->MemoryAreas[i] != NULL && Address >= State->MemoryAreas[i]->BaseAddress && Address <= State->MemoryAreas[i]->LastAddress )
			{
			if( State->MemoryAreas[i]->WriteFunc != NULL )
				return State->MemoryAreas[i]->WriteFunc( State->MemoryAreas[i], Address, sizeof Value, &Value );

			sim_address_t	Index;

			if( State->MemoryAreas[i]->Flags.ReadOnly )
				return EXEC_STATUS_ERROR_READONLY_ADDRESS;

			Index	= Address - State->MemoryAreas[i]->BaseAddress;

			*(uint16_t*)&State->MemoryAreas[i]->Buffer[Index]	= Value;

			if( State->MemoryAreas[i]->Map != NULL )
				{
				State->MemoryAreas[i]->Map[Index + 0].Written	= 1;
				State->MemoryAreas[i]->Map[Index + 1].Written	= 1;
				}

			return EXEC_STATUS_OK;
			}

	return EXEC_STATUS_ERROR_UNDEFINED_ADDRESS;
	}
/*============================================================================*/
INLINE execstatus_t FetchOpCode( const thumbstate_t *State, uint32_t Address, uint16_t *Value )
	{
	int	i;

	if(( Address & 1 ) != 0 )
		return EXEC_STATUS_ERROR_UNALIGNED_ACCESS;

	for( i = 0; i < State->NumMemoryAreas; i++ )
		if( State->MemoryAreas[i] != NULL && Address >= State->MemoryAreas[i]->BaseAddress && Address <= State->MemoryAreas[i]->LastAddress - 1 )
			{
			sim_address_t	Index;

			Index											= Address - State->MemoryAreas[i]->BaseAddress;

			*Value											= *(uint16_t*)&State->MemoryAreas[i]->Buffer[Index];

			if( State->MemoryAreas[i]->Map != NULL )
				{
				State->MemoryAreas[i]->Map[Index + 0].Type	= 2;
				State->MemoryAreas[i]->Map[Index + 1].Type	= 2;

				State->MemoryAreas[i]->Map[Index + 0].Read	= 1;
				State->MemoryAreas[i]->Map[Index + 1].Read	= 1;
				}

			return EXEC_STATUS_OK;
			}

	return EXEC_STATUS_ERROR_UNDEFINED_ADDRESS;
	}
/*============================================================================*/
INLINE execstatus_t LoadHalfWord( const thumbstate_t *State, uint32_t Address, uint16_t *Value )
	{
	int	i;

	if(( Address & 1 ) != 0 )
		return EXEC_STATUS_ERROR_UNALIGNED_ACCESS;

	for( i = 0; i < State->NumMemoryAreas; i++ )
		if( State->MemoryAreas[i] != NULL && Address >= State->MemoryAreas[i]->BaseAddress && Address <= State->MemoryAreas[i]->LastAddress - 1 )
			{
			if( State->MemoryAreas[i]->ReadFunc != NULL )
				return State->MemoryAreas[i]->ReadFunc( State->MemoryAreas[i], Address, sizeof *Value, &Value );

			sim_address_t	Index;

			Index											= Address - State->MemoryAreas[i]->BaseAddress;

			*Value											= *(uint16_t*)&State->MemoryAreas[i]->Buffer[Index];

			if( State->MemoryAreas[i]->Map != NULL )
				{
				State->MemoryAreas[i]->Map[Index + 0].Read	= 1;
				State->MemoryAreas[i]->Map[Index + 1].Read	= 1;
				}

			return EXEC_STATUS_OK;
			}

	return EXEC_STATUS_ERROR_UNDEFINED_ADDRESS;
	}
/*============================================================================*/
static INLINE execstatus_t StoreWord( thumbstate_t *State, uint32_t Address, uint32_t Value )
	{
	int	i;

	if(( Address & 3 ) != 0 )
		return EXEC_STATUS_ERROR_UNALIGNED_ACCESS;

	for( i = 0; i < State->NumMemoryAreas; i++ )
		if( State->MemoryAreas[i] != NULL && Address >= State->MemoryAreas[i]->BaseAddress && Address <= State->MemoryAreas[i]->LastAddress )
			{
			if( State->MemoryAreas[i]->WriteFunc != NULL )
				return State->MemoryAreas[i]->WriteFunc( State->MemoryAreas[i], Address, sizeof Value, &Value );

			sim_address_t	Index;

			if( State->MemoryAreas[i]->Flags.ReadOnly )
				return EXEC_STATUS_ERROR_READONLY_ADDRESS;

			Index												= Address - State->MemoryAreas[i]->BaseAddress;

			*(uint32_t*)&State->MemoryAreas[i]->Buffer[Index]	= Value;

			if( State->MemoryAreas[i]->Map != NULL )
				{
				State->MemoryAreas[i]->Map[Index + 0].Written	= 1;
				State->MemoryAreas[i]->Map[Index + 1].Written	= 1;
				State->MemoryAreas[i]->Map[Index + 2].Written	= 1;
				State->MemoryAreas[i]->Map[Index + 3].Written	= 1;
				}

			return EXEC_STATUS_OK;
			}

	return EXEC_STATUS_ERROR_UNDEFINED_ADDRESS;
	}
/*============================================================================*/
INLINE execstatus_t LoadWord( const thumbstate_t *State, uint32_t Address, uint32_t *Value )
	{
	int	i;

	if(( Address & 3 ) != 0 )
		return EXEC_STATUS_ERROR_UNALIGNED_ACCESS;

	for( i = 0; i < State->NumMemoryAreas; i++ )
		if( State->MemoryAreas[i] != NULL && Address >= State->MemoryAreas[i]->BaseAddress && Address <= State->MemoryAreas[i]->LastAddress - 3 )
			{
			if( State->MemoryAreas[i]->ReadFunc != NULL )
				return State->MemoryAreas[i]->ReadFunc( State->MemoryAreas[i], Address, sizeof *Value, &Value );

			sim_address_t	Index;

			Index	= Address - State->MemoryAreas[i]->BaseAddress;

			*Value	= *(uint32_t*)&State->MemoryAreas[i]->Buffer[Index];

			if( State->MemoryAreas[i]->Map != NULL )
				{
				State->MemoryAreas[i]->Map[Index + 0].Read	= 1;
				State->MemoryAreas[i]->Map[Index + 1].Read	= 1;
				State->MemoryAreas[i]->Map[Index + 2].Read	= 1;
				State->MemoryAreas[i]->Map[Index + 3].Read	= 1;
				}

			return EXEC_STATUS_OK;
			}

	return EXEC_STATUS_ERROR_UNDEFINED_ADDRESS;
	}
/*============================================================================*/
sim_mapentry_t GetMapEntry( thumbstate_t *State, sim_address_t Address )
	{
	int	i;

	for( i = 0; i < State->NumMemoryAreas; i++ )
		if( State->MemoryAreas[i] != NULL && Address >= State->MemoryAreas[i]->BaseAddress && Address <= State->MemoryAreas[i]->LastAddress )
			return State->MemoryAreas[i]->Map[Address - State->MemoryAreas[i]->BaseAddress];

	return (sim_mapentry_t){ 0 };
	}
/*============================================================================*/
const char *GetSymbolName( thumbstate_t *State, sim_address_t Address )
	{
	int	i;

	for( i = 0; i < State->NumMemoryAreas; i++ )
		if( State->MemoryAreas[i] != NULL && Address >= State->MemoryAreas[i]->BaseAddress
			&& Address <= State->MemoryAreas[i]->LastAddress )
			{
			if( State->MemoryAreas[i]->SymbolsNames == NULL || State->MemoryAreas[i]->Map == NULL
				|| State->MemoryAreas[i]->Map[Address - State->MemoryAreas[i]->BaseAddress].NameOffset == (sim_nameoffset_t)-1 )
				return NULL;

			return &State->MemoryAreas[i]->SymbolsNames[State->MemoryAreas[i]->Map[Address - State->MemoryAreas[i]->BaseAddress].NameOffset];
			}

	return NULL;
	}
/*============================================================================*/
/*============================================================================*/
/*============================================================================*/
/*============================================================================*/
static INLINE void OP_Asr( thumbstate_t *State, unsigned int Rd, uint32_t Value, unsigned int Shift )
	{
	if( Shift >= 32 )
		{
		if(( Value & 0x80000000 ) != 0 )
			{
			State->Core.Regs[Rd]	= (uint32_t)-1;
			State->Core.CPSR.C		= 1;
			}
		else
			{
			State->Core.Regs[Rd]	= 0;
			State->Core.CPSR.C		= 0;
			}
		State->Core.Written[CPSR]  |= 0x20000000;
		}
	else if( Shift > 0 )
		{
		State->Core.CPSR.C			= ( Value >> ( Shift - 1 )) & 0x00000001 ? 1 : 0;
		State->Core.Written[CPSR]  |= 0x20000000;
		State->Core.Regs[Rd]		= ((( Value & 0x80000000 ) == 0 ? 0 : (uint32_t)-1 << ( 32 - Shift ))) | ( Value >> Shift );
		}
	State->Core.Written[Rd]			= 1;

	State->Core.CPSR.N				= (int32_t)State->Core.Regs[Rd] < 0 ? 1 : 0;
	State->Core.CPSR.Z				= State->Core.Regs[Rd] == 0 ? 1 : 0;
	State->Core.Written[CPSR]	   |= 0xc0000000;
	}
/*============================================================================*/
/*
 +--+--+--+--+--+--------------+--------+--------+
 |15|14|13|12|11|10 09 08 07 06|05 04 03|02 01 00|
 +--+--+--+--+--+--------------+--------+--------+
 | 0| 0| 0| 0| 0|-----Off5-----|---Rm---|---Rd---|
 +--+--+--+--+--+--------------+--------+--------+
*/
static INLINE void Lsl1( thumbstate_t *State, uint16_t OpCode )
	{
	unsigned int					Rd, Rm, Shift;

	Rd								= ( OpCode >>  0 ) & 0x07;
	Rm								= ( OpCode >>  3 ) & 0x07;
	/* Shift can be 0 to 31 */
	Shift							= ( OpCode >>  6 ) & 0x1f;

	if( Shift != 0 )
		{
		State->Core.CPSR.C			= ( State->Core.Regs[Rm] >> ( 32 - Shift )) & 0x00000001 ? 1 : 0;
		State->Core.Written[CPSR]  |= 0x20000000;
		}

	State->Core.Regs[Rd]			= State->Core.Regs[Rm] << Shift;
	State->Core.Written[Rd]			= 1;

	State->Core.CPSR.Z				= State->Core.Regs[Rd] == 0 ? 1 : 0;
	State->Core.CPSR.N				= (int32_t)State->Core.Regs[Rd] < 0 ? 1 : 0;
	State->Core.Written[CPSR]	   |= 0xc0000000;
	}/*============================================================================*/
/*
 +--+--+--+--+--+--------------+--------+--------+
 |15|14|13|12|11|10 09 08 07 06|05 04 03|02 01 00|
 +--+--+--+--+--+--------------+--------+--------+
 | 0| 0| 0| 0| 1|-----Off5-----|---Rs---|---Rd---|
 +--+--+--+--+--+--------------+--------+--------+
*/
static INLINE void Lsr1( thumbstate_t *State, uint16_t OpCode )
	{
	unsigned int			Rd, Rm, Shift;

	Rd						= ( OpCode >>  0 ) & 0x07;
	Rm						= ( OpCode >>  3 ) & 0x07;
	/* Shift can be 1 to 32. A value of zero means 32 */
	Shift					= ((( OpCode >>  6 ) - 1 ) & 0x1f ) + 1;

	State->Core.CPSR.C		= ( State->Core.Regs[Rm] >> ( Shift - 1 )) & 0x00000001 ? 1 : 0;

	State->Core.Regs[Rd]	= State->Core.Regs[Rm] >> Shift;
	State->Core.Written[Rd]	= 1;
	State->Core.CPSR.Z		= State->Core.Regs[Rd] == 0 ? 1 : 0;
	State->Core.CPSR.N		= (int32_t)State->Core.Regs[Rd] < 0 ? 1 : 0;
	State->Core.Written[CPSR] |= 0xe0000000;
	}/*============================================================================*/
/*
 +--+--+--+--+--+--------------+--------+--------+
 |15|14|13|12|11|10 09 08 07 06|05 04 03|02 01 00|
 +--+--+--+--+--+--------------+--------+--------+
 | 0| 0| 0| 1| 0|-----Off5-----|---Rs---|---Rd---|
 +--+--+--+--+--+--------------+--------+--------+
*/
static INLINE void Asr1( thumbstate_t *State, uint16_t OpCode )
	{
	unsigned int		Rd, Rm, Shift;

	Rd					= ( OpCode >>  0 ) & 0x07;
	Rm					= ( OpCode >>  3 ) & 0x07;
	/* Shift can be 1 to 32. A value of zero means 32 */
	Shift				= ((( OpCode >>  6 ) - 1 ) & 0x1f ) + 1;

	OP_Asr( State, Rd, State->Core.Regs[Rm], Shift );
	}/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--------+--------+--------+
 |15|14|13|12|11|10|09|08 07 06|05 04 03|02 01 00|
 +--+--+--+--+--+--+--+--------+--------+--------+
 | 0| 0| 0| 1| 1| 0| 0|---Rm---|---Rn---|---Rd---|
 +--+--+--+--+--+--+--+--------+--------+--------+
*/
static INLINE void Add3( thumbstate_t *State, uint16_t OpCode )
	{
	unsigned int			Rd, Rn, Rm;

	Rd						= ( OpCode >>  0 ) & 0x07;
	Rn						= ( OpCode >>  3 ) & 0x07;
	Rm						= ( OpCode >>  6 ) & 0x07;

	State->Core.Regs[Rd]	= State->Core.Regs[Rn] + State->Core.Regs[Rm];
	State->Core.Written[Rd]	= 1;
	State->Core.CPSR.N		= (int32_t)State->Core.Regs[Rd] < 0 ? 1 : 0;
	State->Core.CPSR.Z		= State->Core.Regs[Rd] == 0 ? 1 : 0;
	State->Core.CPSR.C		= State->Core.Regs[Rd] < State->Core.Regs[Rn] ? 1 : 0;
	State->Core.CPSR.V		= (( State->Core.Regs[Rn] ^ State->Core.Regs[Rm] ) & 0x80000000 ) == 0 && (( State->Core.Regs[Rd] ^ State->Core.Regs[Rn] ) & 0x80000000 ) != 0 ? 1 : 0;
	State->Core.Written[CPSR] |= 0xf0000000;
	}/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--------+--------+--------+
 |15|14|13|12|11|10|09|08 07 06|05 04 03|02 01 00|
 +--+--+--+--+--+--+--+--------+--------+--------+
 | 0| 0| 0| 1| 1| 0| 0|---Rm---|---Rn---|---Rd---|
 +--+--+--+--+--+--+--+--------+--------+--------+
*/
static INLINE void Sub3( thumbstate_t *State, uint16_t OpCode )
	{
	unsigned int			Rd, Rn, Rm;

	Rd						= ( OpCode >>  0 ) & 0x07;
	Rn						= ( OpCode >>  3 ) & 0x07;
	Rm						= ( OpCode >>  6 ) & 0x07;

	State->Core.Regs[Rd]	= State->Core.Regs[Rn] - State->Core.Regs[Rm];
	State->Core.Written[Rd]	= 1;
	State->Core.CPSR.N		= (int32_t)State->Core.Regs[Rd] < 0 ? 1 : 0;
	State->Core.CPSR.Z		= State->Core.Regs[Rd] == 0 ? 1 : 0;
	State->Core.CPSR.C		= State->Core.Regs[Rd] > State->Core.Regs[Rn] ? 1 : 0;
	State->Core.CPSR.V		= (( State->Core.Regs[Rn] ^ State->Core.Regs[Rm] ) & 0x80000000 ) != 0 && (( State->Core.Regs[Rd] ^ State->Core.Regs[Rn] ) & 0x80000000 ) != 0 ? 1 : 0;
	State->Core.Written[CPSR] |= 0xf0000000;
	}/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--------+--------+--------+
 |15|14|13|12|11|10|09|08 07 06|05 04 03|02 01 00|
 +--+--+--+--+--+--+--+--------+--------+--------+
 | 0| 0| 0| 1| 1| 1| 0|--Imm3--|---Rn---|---Rd---|
 +--+--+--+--+--+--+--+--------+--------+--------+
*/
static INLINE void Add1( thumbstate_t *State, uint16_t OpCode )
	{
	unsigned int			Rd, Rn, Immediate;

	Rd						= ( OpCode >>  0 ) & 0x07;
	Rn						= ( OpCode >>  3 ) & 0x07;
	Immediate				= ( OpCode >>  6 ) & 0x07;

	State->Core.Regs[Rd]	= State->Core.Regs[Rn] + Immediate;
	State->Core.Written[Rd]	= 1;
	State->Core.CPSR.N		= (int32_t)State->Core.Regs[Rd] < 0 ? 1 : 0;
	State->Core.CPSR.Z		= State->Core.Regs[Rd] == 0 ? 1 : 0;
	State->Core.CPSR.C		= State->Core.Regs[Rd] < State->Core.Regs[Rn] ? 1 : 0;
	State->Core.CPSR.V		= (int32_t)State->Core.Regs[Rn] > 0 && (int32_t)State->Core.Regs[Rd] < 0 ? 1 : 0;
	State->Core.Written[CPSR] |= 0xf0000000;
	}/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--------+--------+--------+
 |15|14|13|12|11|10|09|08 07 06|05 04 03|02 01 00|
 +--+--+--+--+--+--+--+--------+--------+--------+
 | 0| 0| 0| 1| 1| 1| 1|--Imm3--|---Rn---|---Rd---|
 +--+--+--+--+--+--+--+--------+--------+--------+
*/
static INLINE void Sub1( thumbstate_t *State, uint16_t OpCode )
	{
	unsigned int			Rd, Rn, Immediate;

	Rd						= ( OpCode >>  0 ) & 0x07;
	Rn						= ( OpCode >>  3 ) & 0x07;
	Immediate				= ( OpCode >>  6 ) & 0x07;

	State->Core.Regs[Rd]	= State->Core.Regs[Rn] - Immediate;
	State->Core.Written[Rd]	= 1;
	State->Core.CPSR.N		= (int32_t)State->Core.Regs[Rd] < 0 ? 1 : 0;
	State->Core.CPSR.Z		= State->Core.Regs[Rd] == 0 ? 1 : 0;
	State->Core.CPSR.C		= State->Core.Regs[Rd] > State->Core.Regs[Rn] ? 1 : 0;
	State->Core.CPSR.V		= (int32_t)State->Core.Regs[Rn] < 0 && (int32_t)State->Core.Regs[Rd] > 0 ? 1 : 0;
	State->Core.Written[CPSR] |= 0xf0000000;
	}/*============================================================================*/
/*
 +--+--+--+--+--+--------+-----------------------+
 |15|14|13|12|11|10 09 08 07 06 05 04 03 02 01 00|
 +--+--+--+--+--+--------+-----------------------+
 | 0| 0| 1| 0| 0|---Rd---|---------Imm8----------|
 +--+--+--+--+--+--------+-----------------------+
*/
static INLINE void Mov1( thumbstate_t *State, uint16_t OpCode )
	{
	unsigned int			Rd, Immediate;

	Rd						= ( OpCode >>  8 ) & 0x07;
	Immediate				= ( OpCode >>  0 ) & 0xff;

	State->Core.Regs[Rd]	= Immediate;
	State->Core.Written[Rd]	= 1;
	State->Core.CPSR.N		= (int32_t)State->Core.Regs[Rd] < 0 ? 1 : 0;
	State->Core.CPSR.Z		= State->Core.Regs[Rd] == 0 ? 1 : 0;
	State->Core.Written[CPSR] |= 0xc0000000;
	}/*============================================================================*/
/*
 +--+--+--+--+--+--------+-----------------------+
 |15|14|13|12|11|10 09 08 07 06 05 04 03 02 01 00|
 +--+--+--+--+--+--------+-----------------------+
 | 0| 0| 1| 0| 1|---Rn---|---------Imm8----------|
 +--+--+--+--+--+--------+-----------------------+
*/
static INLINE void Cmp1( thumbstate_t *State, uint16_t OpCode )
	{
	unsigned int			Rn;
	uint32_t				Immediate, Result;

	Rn						= ( OpCode >>  8 ) & 0x07;
	Immediate				= ( OpCode >>  0 ) & 0xff;

	Result					= State->Core.Regs[Rn] - Immediate;
	State->Core.CPSR.N		= (int32_t)Result < 0 ? 1 : 0;
	State->Core.CPSR.Z		= Result == 0 ? 1 : 0;
	State->Core.CPSR.C		= Result > State->Core.Regs[Rn] ? 1 : 0;
	State->Core.CPSR.V		= (int32_t)State->Core.Regs[Rn] < 0 && (int32_t)Result > 0 ? 1 : 0;
	State->Core.Written[CPSR] |= 0xf0000000;
	}/*============================================================================*/
/*
 +--+--+--+--+--+--------+-----------------------+
 |15|14|13|12|11|10 09 08 07 06 05 04 03 02 01 00|
 +--+--+--+--+--+--------+-----------------------+
 | 0| 0| 1| 1| 0|---Rd---|---------Imm8----------|
 +--+--+--+--+--+--------+-----------------------+
*/
static INLINE void Add2( thumbstate_t *State, uint16_t OpCode )
	{
	unsigned int			Rd;
	uint32_t				Immediate, Original;

	Rd						= ( OpCode >>  8 ) & 0x07;
	Immediate				= ( OpCode >>  0 ) & 0xff;

	Original				= State->Core.Regs[Rd];
	State->Core.Regs[Rd]   += Immediate;
	State->Core.Written[Rd]	= 1;
	State->Core.CPSR.N		= (int32_t)State->Core.Regs[Rd] < 0 ? 1 : 0;
	State->Core.CPSR.Z		= State->Core.Regs[Rd] == 0 ? 1 : 0;
	State->Core.CPSR.C		= State->Core.Regs[Rd] < Original ? 1 : 0;
	State->Core.CPSR.V		= (int32_t)Original > 0 && (int32_t)State->Core.Regs[Rd] < 0 ? 1 : 0;
	State->Core.Written[CPSR]	= 0xf0000000;
	}/*============================================================================*/
/*
 +--+--+--+--+--+--------+-----------------------+
 |15|14|13|12|11|10 09 08 07 06 05 04 03 02 01 00|
 +--+--+--+--+--+--------+-----------------------+
 | 0| 0| 1| 1| 1|---Rd---|---------Imm8----------|
 +--+--+--+--+--+--------+-----------------------+

*/
static INLINE void Sub2( thumbstate_t *State, uint16_t OpCode )
	{
	unsigned int			Rd;
	uint32_t				Immediate, Original;

	Rd						= ( OpCode >>  8 ) & 0x07;
	Immediate				= ( OpCode >>  0 ) & 0xff;

	Original				= State->Core.Regs[Rd];
	State->Core.Regs[Rd]   -= Immediate;
	State->Core.Written[Rd]	= 1;
	State->Core.CPSR.N		= (int32_t)State->Core.Regs[Rd] < 0 ? 1 : 0;
	State->Core.CPSR.Z		= State->Core.Regs[Rd] == 0 ? 1 : 0;
	State->Core.CPSR.C		= State->Core.Regs[Rd] > Original ? 1 : 0;
	State->Core.CPSR.V		= (int32_t)Original < 0 && (int32_t)State->Core.Regs[Rd] > 0 ? 1 : 0;
	State->Core.Written[CPSR]	= 0xf0000000;
	}/*============================================================================*/

/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 |15|14|13|12|11|10|09|08|07|06|05 04 03|02 01 00|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 0| 1| 0| 0| 0| 0| 0| 0| 0| 0|---Rm---|---Rd---|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
*/
static INLINE void AluAnd( thumbstate_t *State, unsigned int Rd, unsigned int Rm )
	{
	State->Core.Regs[Rd]   &= State->Core.Regs[Rm];
	State->Core.CPSR.N		= (int32_t)State->Core.Regs[Rd] < 0 ? 1 : 0;
	State->Core.CPSR.Z		= State->Core.Regs[Rd] == 0 ? 1 : 0;
	State->Core.Written[CPSR]	= 0xc0000000;
	}
/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 |15|14|13|12|11|10|09|08|07|06|05 04 03|02 01 00|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 0| 1| 0| 0| 0| 0| 0| 0| 0| 1|---Rm---|---Rd---|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
*/
static INLINE void AluEor( thumbstate_t *State, unsigned int Rd, unsigned int Rm )
	{
	State->Core.Regs[Rd]   &= State->Core.Regs[Rm];
	State->Core.CPSR.N		= (int32_t)State->Core.Regs[Rd] < 0 ? 1 : 0;
	State->Core.CPSR.Z		= State->Core.Regs[Rd] == 0 ? 1 : 0;
	State->Core.Written[CPSR]	= 0xc0000000;
	}
/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 |15|14|13|12|11|10|09|08|07|06|05 04 03|02 01 00|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 0| 1| 0| 0| 0| 0| 0| 0| 1| 0|---Rs---|---Rd---|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
*/
static INLINE void AluLsl2( thumbstate_t *State, unsigned int Rd, unsigned int Rs )
	{
	unsigned int	Shift			= State->Core.Regs[Rs] & 0xff;

	if( Shift >= 32 )
		{
		if( Shift == 32 )
			State->Core.CPSR.C		= State->Core.Regs[Rd] & 0x00000001 ? 1 : 0;
		else
			State->Core.CPSR.C		= 0;

		State->Core.Written[CPSR]  |= 0x20000000;
		State->Core.Regs[Rd]		= 0;
		State->Core.Written[Rd]		= 1;
		}
	else if( Shift > 0 )
		{
		State->Core.CPSR.C			= ( State->Core.Regs[Rd] >> ( 32 - Shift )) & 0x00000001 ? 1 : 0;
		State->Core.Written[CPSR]  |= 0x20000000;

		State->Core.Regs[Rd]	  <<= Shift;
		State->Core.Written[Rd]		= 1;
		}

	State->Core.CPSR.Z				= State->Core.Regs[Rd] == 0 ? 1 : 0;
	State->Core.CPSR.N				= (int32_t)State->Core.Regs[Rd] < 0 ? 1 : 0;
	State->Core.Written[CPSR]		= 0xc0000000;
	}
/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 |15|14|13|12|11|10|09|08|07|06|05 04 03|02 01 00|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 0| 1| 0| 0| 0| 0| 0| 0| 1| 1|---Rm---|---Rd---|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
*/
static INLINE void AluLsr2( thumbstate_t *State, unsigned int Rd, unsigned int Rm )
	{
	unsigned int	Shift			= State->Core.Regs[Rm] & 0xff;

	if( Shift >= 32 )
		{
		if( Shift == 32 )
			State->Core.CPSR.C		= State->Core.Regs[Rd] & 0x80000000 ? 1 : 0;
		else
			State->Core.CPSR.C		= 0;

		State->Core.Written[CPSR]  |= 0x20000000;
		State->Core.Regs[Rd]		= 0;
		State->Core.Written[Rd]		= 1;
		}
	else if( Shift > 0 )
		{
		State->Core.CPSR.C			= ( State->Core.Regs[Rd] >> ( Shift - 1 )) & 0x00000001 ? 1 : 0;

		State->Core.Written[CPSR]  |= 0x20000000;
		State->Core.Regs[Rd]	  >>= Shift;
		State->Core.Written[Rd]		= 1;
		}

	State->Core.CPSR.Z				= State->Core.Regs[Rd] == 0 ? 1 : 0;
	State->Core.CPSR.N				= (int32_t)State->Core.Regs[Rd] < 0 ? 1 : 0;
	State->Core.Written[CPSR]		= 0xc0000000;
	}
/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 |15|14|13|12|11|10|09|08|07|06|05 04 03|02 01 00|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 0| 1| 0| 0| 0| 0| 0| 1| 0| 0|---Rs---|---Rd---|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
*/
static INLINE void AluAsr2( thumbstate_t *State, unsigned int Rd, unsigned int Rs )
	{
	unsigned int	Shift	= State->Core.Regs[Rs] & 0xff;

	OP_Asr( State, Rd, State->Core.Regs[Rd], Shift );
	}
/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 |15|14|13|12|11|10|09|08|07|06|05 04 03|02 01 00|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 0| 1| 0| 0| 0| 0| 0| 1| 0| 1|---Rm---|---Rd---|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
*/
static INLINE void AluAdc( thumbstate_t *State, unsigned int Rd, unsigned int Rm )
	{
	uint32_t					Original;

	Original					= State->Core.Regs[Rd];
	State->Core.Regs[Rd]	   += State->Core.Regs[Rm] + State->Core.CPSR.C;
	State->Core.CPSR.N			= (int32_t)State->Core.Regs[Rd] < 0 ? 1 : 0;
	State->Core.CPSR.Z			= State->Core.Regs[Rd] == 0 ? 1 : 0;
	State->Core.CPSR.C			= State->Core.Regs[Rd] < Original ? 1 : 0;
	State->Core.CPSR.V			= (( Original ^ State->Core.Regs[Rm] ) & 0x80000000 ) == 0 && (( State->Core.Regs[Rd] ^ Original ) & 0x80000000 ) != 0 ? 1 : 0;
	State->Core.Written[CPSR]	= 0xf0000000;
	}
/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 |15|14|13|12|11|10|09|08|07|06|05 04 03|02 01 00|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 0| 1| 0| 0| 0| 0| 0| 1| 1| 0|---Rm---|---Rd---|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
*/
static INLINE void AluSbc( thumbstate_t *State, unsigned int Rd, unsigned int Rm )
	{
	uint32_t					Original;

	Original					= State->Core.Regs[Rd];
	State->Core.Regs[Rd]	   -= State->Core.Regs[Rm] + State->Core.CPSR.C;
	State->Core.CPSR.N			= (int32_t)State->Core.Regs[Rd] < 0 ? 1 : 0;
	State->Core.CPSR.Z			= State->Core.Regs[Rd] == 0 ? 1 : 0;
	State->Core.CPSR.C			= State->Core.Regs[Rd] > Original ? 1 : 0;
	State->Core.CPSR.V			= (( Original ^ State->Core.Regs[Rm] ) & 0x80000000 ) != 0 && (( State->Core.Regs[Rd] ^ Original ) & 0x80000000 ) != 0 ? 1 : 0;
	State->Core.Written[CPSR]	= 0xf0000000;
	}
/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 |15|14|13|12|11|10|09|08|07|06|05 04 03|02 01 00|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 0| 1| 0| 0| 0| 0| 0| 1| 1| 1|---Rs---|---Rd---|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
*/
static INLINE void AluRor( thumbstate_t *State, unsigned int Rd, unsigned int Rs )
	{
	unsigned int					Rotation;

	Rotation						= State->Core.Regs[Rs] & 0xff;

	if( Rotation > 0 )
		{
		Rotation	   &= 0x1f;

		if( Rotation == 0 )
			State->Core.CPSR.C		= ( State->Core.Regs[Rd] & 0x80000000 ) != 0 ? 1 : 0;
		else
			{
			State->Core.CPSR.C		= (( State->Core.Regs[Rd] >> ( Rotation - 1 )) & 1 ) != 0 ? 1 : 0;
			State->Core.Regs[Rd]	= ( State->Core.Regs[Rd] >> Rotation ) | ( State->Core.Regs[Rd] << ( 32 - Rotation ));
			State->Core.Written[Rd]	= 1;
			}
		State->Core.Written[CPSR]  |= 0x20000000;
		}
	State->Core.CPSR.N				= (int32_t)State->Core.Regs[Rd] < 0 ? 1 : 0;
	State->Core.CPSR.Z				= State->Core.Regs[Rd] == 0 ? 1 : 0;
	State->Core.Written[CPSR]		= 0xc0000000;
	}
/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 |15|14|13|12|11|10|09|08|07|06|05 04 03|02 01 00|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 0| 1| 0| 0| 0| 0| 1| 0| 0| 0|---Rm---|---Rn---|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
*/
static INLINE void AluTst( thumbstate_t *State, unsigned int Rn, unsigned int Rm )
	{
	uint32_t					Result;

	Result						= State->Core.Regs[Rn] & State->Core.Regs[Rm];
	State->Core.CPSR.N			= (int32_t)Result < 0 ? 1 : 0;
	State->Core.CPSR.Z			= Result == 0 ? 1 : 0;
	State->Core.Written[CPSR]	= 0xc0000000;
	}
/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 |15|14|13|12|11|10|09|08|07|06|05 04 03|02 01 00|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 0| 1| 0| 0| 0| 0| 1| 0| 0| 1|---Rm---|---Rd---|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
*/
static INLINE void AluNeg( thumbstate_t *State, unsigned int Rd, unsigned int Rm )
	{
	State->Core.Regs[Rd]		= 0 - State->Core.Regs[Rm];
	State->Core.Written[Rd]		= 1;
	State->Core.CPSR.N			= (int32_t)State->Core.Regs[Rd] < 0 ? 1 : 0;
	State->Core.CPSR.Z			= State->Core.Regs[Rd] == 0 ? 1 : 0;
	State->Core.CPSR.C			= State->Core.Regs[Rd] > 0 ? 1 : 0;
	State->Core.CPSR.V			= (( State->Core.Regs[Rd] ^ State->Core.Regs[Rm] ) & 0x80000000 ) == 0 ? 1 : 0;
	State->Core.Written[CPSR]	= 0xf0000000;
	}
/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 |15|14|13|12|11|10|09|08|07|06|05 04 03|02 01 00|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 0| 1| 0| 0| 0| 0| 1| 0| 1| 0|---Rm---|---Rn---|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
*/
static INLINE void AluCmp2( thumbstate_t *State, unsigned int Rn, unsigned int Rm )
	{
	uint32_t					Result;

	Result						= State->Core.Regs[Rn] - State->Core.Regs[Rm];
	State->Core.CPSR.N			= (int32_t)Result < 0 ? 1 : 0;
	State->Core.CPSR.Z			= Result == 0 ? 1 : 0;
	State->Core.CPSR.C			= Result > State->Core.Regs[Rn] ? 1 : 0;
	State->Core.CPSR.V			= (( State->Core.Regs[Rn] ^ State->Core.Regs[Rm] ) & 0x80000000 ) != 0 && (( Result ^ State->Core.Regs[Rn] ) & 0x80000000 ) != 0 ? 1 : 0;
	State->Core.Written[CPSR]	= 0xf0000000;
	}
/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 |15|14|13|12|11|10|09|08|07|06|05 04 03|02 01 00|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 0| 1| 0| 0| 0| 0| 1| 0| 1| 1|---Rm---|---Rd---|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
*/
static INLINE void AluCmn( thumbstate_t *State, unsigned int Rd, unsigned int Rm )
	{
	uint32_t					Result;

	Result						= State->Core.Regs[Rd] + State->Core.Regs[Rm];
	State->Core.CPSR.N			= (int32_t)Result < 0 ? 1 : 0;
	State->Core.CPSR.Z			= Result == 0 ? 1 : 0;
	State->Core.CPSR.C			= Result < State->Core.Regs[Rd] ? 1 : 0;
	State->Core.CPSR.V			= (( State->Core.Regs[Rd] ^ State->Core.Regs[Rm] ) & 0x80000000 ) == 0 && (( Result ^ State->Core.Regs[Rd] ) & 0x80000000 ) != 0 ? 1 : 0;
	State->Core.Written[CPSR]	= 0xf0000000;
	}
/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 |15|14|13|12|11|10|09|08|07|06|05 04 03|02 01 00|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 0| 1| 0| 0| 0| 0| 1| 1| 0| 0|---Rm---|---Rd---|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
*/
static INLINE void AluOrr( thumbstate_t *State, unsigned int Rd, unsigned int Rm )
	{
	State->Core.Regs[Rd]	   |= State->Core.Regs[Rm];
	State->Core.Written[Rd]		= 1;
	State->Core.CPSR.N			= (int32_t)State->Core.Regs[Rd] < 0 ? 1 : 0;
	State->Core.CPSR.Z			= State->Core.Regs[Rd] == 0 ? 1 : 0;
	State->Core.Written[CPSR]	= 0xc0000000;
	}
/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 |15|14|13|12|11|10|09|08|07|06|05 04 03|02 01 00|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 0| 1| 0| 0| 0| 0| 1| 1| 0| 0|---Rm---|---Rd---|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
*/
static INLINE void AluMul( thumbstate_t *State, unsigned int Rd, unsigned int Rm )
	{
	State->Core.Regs[Rd]	   *= State->Core.Regs[Rm];
	State->Core.Written[Rd]		= 1;
	State->Core.CPSR.N			= (int32_t)State->Core.Regs[Rd] < 0 ? 1 : 0;
	State->Core.CPSR.Z			= State->Core.Regs[Rd] == 0 ? 1 : 0;
	State->Core.Written[CPSR]	= 0xc0000000;
	}
/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 |15|14|13|12|11|10|09|08|07|06|05 04 03|02 01 00|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 0| 1| 0| 0| 0| 0| 1| 1| 1| 0|---Rm---|---Rd---|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
*/
static INLINE void AluBic( thumbstate_t *State, unsigned int Rd, unsigned int Rm )
	{
	State->Core.Regs[Rd]	   &= ~State->Core.Regs[Rm];
	State->Core.Written[Rd]		= 1;
	State->Core.CPSR.N			= (int32_t)State->Core.Regs[Rd] < 0 ? 1 : 0;
	State->Core.CPSR.Z			= State->Core.Regs[Rd] == 0 ? 1 : 0;
	State->Core.Written[CPSR]	= 0xc0000000;
	}
/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 |15|14|13|12|11|10|09|08|07|06|05 04 03|02 01 00|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 0| 1| 0| 0| 0| 0| 1| 1| 1| 1|---Rm---|---Rd---|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
*/
static INLINE void AluMvn( thumbstate_t *State, unsigned int Rd, unsigned int Rm )
	{
	State->Core.Regs[Rd]		= ~State->Core.Regs[Rm];
	State->Core.Written[Rd]		= 1;
	State->Core.CPSR.N			= (int32_t)State->Core.Regs[Rd] < 0 ? 1 : 0;
	State->Core.CPSR.Z			= State->Core.Regs[Rd] == 0 ? 1 : 0;
	State->Core.Written[CPSR]	= 0xc0000000;
	}
/*============================================================================*/
/*
 +--+--+--+--+--+--+-----------+--------+--------+
 |15|14|13|12|11|10|09 08 07 06|05 04 03|02 01 00|
 +--+--+--+--+--+--+-----------+--------+--------+
 | 0| 1| 0| 0| 0| 0|----Op-----|---Rm---|---Rd---|
 +--+--+--+--+--+--+-----------+--------+--------+
*/
static INLINE void ALU( thumbstate_t *State, uint16_t OpCode )
	{
	unsigned int	Rd, Rm, Operation;

	Rd			= ( OpCode >> 0 ) & 0x07;
	Rm			= ( OpCode >> 3 ) & 0x07;
	Operation	= ( OpCode >> 6 ) & 0x0f;

	switch( Operation )
		{
		case 0x00:
			AluAnd( State, Rd, Rm );
			break;
		case 0x01:
			AluEor( State, Rd, Rm );
			break;
		case 0x02:
			AluLsl2( State, Rd, Rm );
			break;
		case 0x03:
			AluLsr2( State, Rd, Rm );
			break;
		case 0x04:
			AluAsr2( State, Rd, Rm );
			break;
		case 0x05:
			AluAdc( State, Rd, Rm );
			break;
		case 0x06:
			AluSbc( State, Rd, Rm );
			break;
		case 0x07:
			AluRor( State, Rd, Rm );
			break;
		case 0x08:
			AluTst( State, Rd, Rm );
			break;
		case 0x09:
			AluNeg( State, Rd, Rm );
			break;
		case 0x0a:
			AluCmp2( State, Rd, Rm );
			break;
		case 0x0b:
			AluCmn( State, Rd, Rm );
			break;
		case 0x0c:
			AluOrr( State, Rd, Rm );
			break;
		case 0x0d:
			AluMul( State, Rd, Rm );
			break;
		case 0x0e:
			AluBic( State, Rd, Rm );
			break;
		case 0x0f:
			AluMvn( State, Rd, Rm );
			break;
		}
	}/*============================================================================*/
/*
; +--+--+--+--+--+--+--+--+--+--+--------+--------+
; |15|14|13|12|11|10|09|08|07|06|05 04 03|02 01 00|
; +--+--+--+--+--+--+--+--+--+--+--------+--------+
; | 0| 1| 0| 0| 0| 1| 0| 0|H1|H2|---Rm---|---Rd---|
; +--+--+--+--+--+--+--+--+--+--+--------+--------+

*/
static INLINE void Add4( thumbstate_t *State, uint16_t OpCode )
	{
	unsigned int			Rd, Rm;

	Rd						= (( OpCode >>  4 ) & 0x08 ) | (( OpCode >>  0 ) & 0x07 );
	Rm						=  ( OpCode >>  3 ) & 0x0f;

	State->Core.Regs[Rd]   += State->Core.Regs[Rm];
	State->Core.Written[Rd]	= 1;
	}/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 |15|14|13|12|11|10|09|08|07|06|05 04 03|02 01 00|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 0| 1| 0| 0| 0| 1| 0| 1|H1|H2|---Rm---|---Rn---|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
*/
static INLINE void Cmp3( thumbstate_t *State, uint16_t OpCode )
	{
	unsigned int				Rm, Rn;
	uint32_t					Result;

	Rn							= (( OpCode >>  4 ) & 0x08 ) | (( OpCode >>  0 ) & 0x07 );
	Rm							=  ( OpCode >>  3 ) & 0x0f;

	Result						= State->Core.Regs[Rn] - State->Core.Regs[Rm];
	State->Core.CPSR.N			= (int32_t)Result < 0 ? 1 : 0;
	State->Core.CPSR.Z			= Result == 0 ? 1 : 0;
	State->Core.CPSR.C			= Result > State->Core.Regs[Rn] ? 1 : 0;
	State->Core.CPSR.V			= (( State->Core.Regs[Rn] ^ State->Core.Regs[Rm] ) & 0x80000000 ) != 0 && (( Result ^ State->Core.Regs[Rn] ) & 0x80000000 ) != 0 ? 1 : 0;
	State->Core.Written[CPSR]	= 0xf0000000;
	}/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 |15|14|13|12|11|10|09|08|07|06|05 04 03|02 01 00|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 0| 1| 0| 0| 0| 1| 1| 0|H1|H2|---Rm---|---Rd---|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
*/
static INLINE void Cpy( thumbstate_t *State, uint16_t OpCode )
	{
	unsigned int			Rd, Rm;

	Rd						= (( OpCode >>  4 ) & 0x08 ) | (( OpCode >>  0 ) & 0x07 );
	Rm						=  ( OpCode >>  3 ) & 0x0f;

	State->Core.Regs[Rd]	= State->Core.Regs[Rm];
	State->Core.Written[Rd]	= 1;
	}/*============================================================================*/
static INLINE execstatus_t CallI( thumbstate_t *State, unsigned Vector )
	{
	uint32_t					Temp;
	execstatus_t				Result2, Result	= EXEC_STATUS_TO_CONTINUE;

	State->Core.SP			   -= 4;
	if(( Result2 = StoreWord( State, State->Core.SP, State->Core.CPSR.Value )) < EXEC_STATUS_BREAKPOINT_WRITE )
		return Result2;
	if( Result2 <= EXEC_STATUS_BREAKPOINT_EXEC )
		Result					= Result2;

	State->Core.SP			   -= 4;
	if(( Result2 = StoreWord( State, State->Core.SP, State->Core.PC )) < EXEC_STATUS_BREAKPOINT_WRITE )
		return Result2;
	if( Result2 <= EXEC_STATUS_BREAKPOINT_EXEC )
		Result					= Result2;

	State->Core.SP			   -= 4;
	if(( Result2 = StoreWord( State, State->Core.SP, State->Core.LR )) < EXEC_STATUS_BREAKPOINT_WRITE )
		return Result2;
	if( Result2 <= EXEC_STATUS_BREAKPOINT_EXEC )
		Result					= Result2;

	State->Core.SP			   -= 4;
	if(( Result2 = StoreWord( State, State->Core.SP, State->Core.R12 )) < EXEC_STATUS_BREAKPOINT_WRITE )
		return Result2;
	if( Result2 <= EXEC_STATUS_BREAKPOINT_EXEC )
		Result					= Result2;

	State->Core.SP			   -= 4;
	if(( Result2 = StoreWord( State, State->Core.SP, State->Core.R3 )) < EXEC_STATUS_BREAKPOINT_WRITE )
		return Result2;
	if( Result2 <= EXEC_STATUS_BREAKPOINT_EXEC )
		Result					= Result2;

	State->Core.SP			   -= 4;
	if(( Result2 = StoreWord( State, State->Core.SP, State->Core.R2 )) < EXEC_STATUS_BREAKPOINT_WRITE )
		return Result2;
	if( Result2 <= EXEC_STATUS_BREAKPOINT_EXEC )
		Result					= Result2;

	State->Core.SP			   -= 4;
	if(( Result2 = StoreWord( State, State->Core.SP, State->Core.R1 )) < EXEC_STATUS_BREAKPOINT_WRITE )
		return Result2;
	if( Result2 <= EXEC_STATUS_BREAKPOINT_EXEC )
		Result					= Result2;

	State->Core.SP			   -= 4;
	if(( Result2 = StoreWord( State, State->Core.SP, State->Core.R0 )) < EXEC_STATUS_BREAKPOINT_WRITE )
		return Result2;
	if( Result2 <= EXEC_STATUS_BREAKPOINT_EXEC )
		Result					= Result2;

	State->Core.CPSR.EN			= CPU_MODE_SVC;
	State->Core.Written[CPSR]	= 0x0000001f;

	Temp						= ( State->MemoryAreas[0] != NULL ? State->MemoryAreas[0]->BaseAddress : State->MemoryAreas[1]->BaseAddress ) + Vector * 4;
	if(( Result2 = LoadWord( State, Temp, &Temp )) < EXEC_STATUS_BREAKPOINT_WRITE )
		return Result2;
	if( Result2 <= EXEC_STATUS_BREAKPOINT_EXEC )
		Result					= Result2;

	if(( Temp & 1 ) == 0 )
		return EXEC_STATUS_ERROR_INVALID_MODE;

	State->Core.LR				= 0xfffffffd;

	State->Core.Written[SP]		= 1;
	State->Core.Written[LR]		= 1;
	State->Core.Written[PC]		= 1;

	return Result;
	}
/*============================================================================*/
static INLINE execstatus_t RetI( thumbstate_t *State, uint32_t Value )
	{
	execstatus_t			Result2, Result	= EXEC_STATUS_TO_CONTINUE;

	if(( Result2 = LoadWord( State, State->Core.SP, &State->Core.R0 )) < EXEC_STATUS_BREAKPOINT_WRITE )
		return Result2;
	State->Core.SP		   += 4;
	if( Result2 <= EXEC_STATUS_BREAKPOINT_EXEC )
		Result				= Result2;

	if(( Result2 = LoadWord( State, State->Core.SP, &State->Core.R1 )) < EXEC_STATUS_BREAKPOINT_WRITE )
		return Result2;
	State->Core.SP		   += 4;
	if( Result2 <= EXEC_STATUS_BREAKPOINT_EXEC )
		Result				= Result2;

	if(( Result2 = LoadWord( State, State->Core.SP, &State->Core.R2 )) < EXEC_STATUS_BREAKPOINT_WRITE )
		return Result2;
	State->Core.SP		   += 4;
	if( Result2 <= EXEC_STATUS_BREAKPOINT_EXEC )
		Result				= Result2;

	if(( Result2 = LoadWord( State, State->Core.SP, &State->Core.R3 )) < EXEC_STATUS_BREAKPOINT_WRITE )
		return Result2;
	State->Core.SP		   += 4;
	if( Result2 <= EXEC_STATUS_BREAKPOINT_EXEC )
		Result				= Result2;

	if(( Result2 = LoadWord( State, State->Core.SP, &State->Core.R12 )) < EXEC_STATUS_BREAKPOINT_WRITE )
		return Result2;
	State->Core.SP		   += 4;
	if( Result2 <= EXEC_STATUS_BREAKPOINT_EXEC )
		Result				= Result2;

	if(( Result2 = LoadWord( State, State->Core.SP, &State->Core.LR )) < EXEC_STATUS_BREAKPOINT_WRITE )
		return Result2;
	State->Core.SP		   += 4;
	if( Result2 <= EXEC_STATUS_BREAKPOINT_EXEC )
		Result				= Result2;

	if(( Result2 = LoadWord( State, State->Core.SP, &State->Core.PC )) < EXEC_STATUS_BREAKPOINT_WRITE )
		return Result2;
	State->Core.SP		   += 4;
	if( Result2 <= EXEC_STATUS_BREAKPOINT_EXEC )
		Result				= Result2;

	if(( Result2 = LoadWord( State, State->Core.SP, &State->Core.CPSR.Value )) < EXEC_STATUS_BREAKPOINT_WRITE )
		return Result2;
	State->Core.SP		   += 4;
	if( Result2 <= EXEC_STATUS_BREAKPOINT_EXEC )
		Result				= Result2;

	State->Core.Written[R0]	= 1;
	State->Core.Written[R1]	= 1;
	State->Core.Written[R2]	= 1;
	State->Core.Written[R3]	= 1;
	State->Core.Written[R12]	= 1;
	State->Core.Written[SP]	= 1;
	State->Core.Written[LR]	= 1;
	State->Core.Written[PC]	= 1;

	return Result;
	}
/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+--+-----------+--+--+--+
 |15|14|13|12|11|10|09|08|07|06 05 04 03|02|01|00|
 +--+--+--+--+--+--+--+--+--+-----------+--+--+--+
 | 0| 1| 0| 0| 0| 1| 1| 1| L|----Rm-----| 0| 0| 0|
 +--+--+--+--+--+--+--+--+--+-----------+--+--+--+
*/
static INLINE execstatus_t BxBlx2( thumbstate_t *State, uint16_t OpCode )
	{
	unsigned int				Rm;
	unsigned int				Link;
	uint32_t					Target;
	int							i;

	Rm							=  ( OpCode >>  3 ) & 0x0f;
	Link						=  ( OpCode >>  7 ) & 0x01;

	if( !Link && State->Core.Regs[Rm] >= 0xfffffff0 && State->Core.CPSR.EN != CPU_MODE_USER )
		return RetI( State, State->Core.Regs[Rm] );

	if(( State->Core.Regs[Rm] & 0x00000001 ) == 0 )
		return EXEC_STATUS_ERROR_INVALID_MODE;

	Target						= State->Core.Regs[Rm] & 0xfffffffe;

	for( i = 0; i < State->NumMemoryAreas; i++ )
		if( State->MemoryAreas[i] != NULL && Target >= State->MemoryAreas[i]->BaseAddress && Target <= State->MemoryAreas[i]->LastAddress )
			break;

	if( i >= State->NumMemoryAreas )
		{
		State->Core.PC		   -= 2;
		return EXEC_STATUS_ERROR_UNDEFINED_ADDRESS;
		}

	if( Link )
		{
		State->Core.LR			= State->Core.PC;
		State->Core.Written[LR]	= 1;
		}

	State->Core.PC				= Target | 1;
	State->Core.Written[PC]		= 1;

    return EXEC_STATUS_TO_CONTINUE;
	}
/*============================================================================*/
/*
 +--+--+--+--+--+--------+-----------------------+
 |15|14|13|12|11|10 09 08|07 06 05 04 03 02 01 00|
 +--+--+--+--+--+--------+-----------------------+
 | 0| 1| 0| 0| 1|---Rd---|---------Imm8----------|
 +--+--+--+--+--+--------+-----------------------+
*/
static INLINE execstatus_t Ldr3( thumbstate_t *State, uint16_t OpCode )
	{
	unsigned int				Rd, Immediate;
	uint32_t					Address;
	int							Result;

	Rd							= ( OpCode >>  8 ) & 0x07;
	Immediate					= ( OpCode <<  2 ) & 0x3fc;

	Address						= (( State->Core.PC + 2 ) & 0xfffffffc ) + Immediate;

	if( (int64_t)State->Core.PC != State->LastDataBkpt && CheckReadBreakpoints( State, Address, 4 ))
		{
		State->Core.PC		   -=  2;
		return EXEC_STATUS_BREAKPOINT_READ;
		}

	if(( Result = LoadWord(State, Address, &State->Core.Regs[Rd] )) >= EXEC_STATUS_OK )
		State->Core.Written[Rd]	=  1;

	return Result;
	}/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--------+--------+--------+
 |15|14|13|12|11|10|09|08 07 06|05 04 03|02 01 00|
 +--+--+--+--+--+--+--+--------+--------+--------+
 | 0| 1| 0| 1| 0| 0| 0|---Rm---|---Rn---|---Rd---|
 +--+--+--+--+--+--+--+--------+--------+--------+
*/
static INLINE execstatus_t Str2( thumbstate_t *State, uint16_t OpCode )
	{
	unsigned int				Rd, Rn, Rm;
	uint32_t					Address;

	Rd							= ( OpCode >>  0 ) & 0x07;
	Rn							= ( OpCode >>  3 ) & 0x07;
	Rm							= ( OpCode >>  6 ) & 0x07;

	Address						= State->Core.Regs[Rn] + State->Core.Regs[Rm];

	if( (int64_t)State->Core.PC != State->LastDataBkpt && CheckWriteBreakpoints( State, Address, 4 ))
		{
		State->Core.PC		   -=  2;
		return EXEC_STATUS_BREAKPOINT_WRITE;
		}

	return StoreWord( State, Address, State->Core.Regs[Rd] );
	}/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--------+--------+--------+
 |15|14|13|12|11|10|09|08 07 06|05 04 03|02 01 00|
 +--+--+--+--+--+--+--+--------+--------+--------+
 | 0| 1| 0| 1| 0| 0| 1|---Rm---|---Rn---|---Rd---|
 +--+--+--+--+--+--+--+--------+--------+--------+
*/
static INLINE execstatus_t Strh2( thumbstate_t *State, uint16_t OpCode )
	{
	unsigned int				Rd, Rn, Rm;
	uint32_t					Address;

	Rd							= ( OpCode >>  0 ) & 0x07;
	Rn							= ( OpCode >>  3 ) & 0x07;
	Rm							= ( OpCode >>  6 ) & 0x07;

	Address						= State->Core.Regs[Rn] + State->Core.Regs[Rm];

	if( (int64_t)State->Core.PC != State->LastDataBkpt && CheckWriteBreakpoints( State, Address, 2 ))
		{
		State->Core.PC		   -=  2;
		return EXEC_STATUS_BREAKPOINT_WRITE;
		}

	return StoreHalfWord( State, Address, (uint16_t)State->Core.Regs[Rd] );
	}/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--------+--------+--------+
 |15|14|13|12|11|10|09|08 07 06|05 04 03|02 01 00|
 +--+--+--+--+--+--+--+--------+--------+--------+
 | 0| 1| 0| 1| 0| 1| 0|---Rm---|---Rn---|---Rd---|
 +--+--+--+--+--+--+--+--------+--------+--------+
*/
static INLINE execstatus_t Strb2( thumbstate_t *State, uint16_t OpCode )
	{
	unsigned int				Rd, Rn, Rm;
	uint32_t					Address;

	Rd							= ( OpCode >>  0 ) & 0x07;
	Rn							= ( OpCode >>  3 ) & 0x07;
	Rm							= ( OpCode >>  6 ) & 0x07;

	Address						= State->Core.Regs[Rn] + State->Core.Regs[Rm];

	if( (int64_t)State->Core.PC != State->LastDataBkpt && CheckWriteBreakpoints( State, Address, 1 ))
		{
		State->Core.PC		   -=  2;
		return EXEC_STATUS_BREAKPOINT_WRITE;
		}

	return StoreByte( State, Address, (uint8_t)State->Core.Regs[Rd] );
	}/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--------+--------+--------+
 |15|14|13|12|11|10|09|08 07 06|05 04 03|02 01 00|
 +--+--+--+--+--+--+--+--------+--------+--------+
 | 0| 1| 0| 1| 0| 1| 1|---Rm---|---Rn---|---Rd---|
 +--+--+--+--+--+--+--+--------+--------+--------+
*/
static INLINE execstatus_t Ldrsb( thumbstate_t *State, uint16_t OpCode )
	{
	unsigned int				Rd, Rn, Rm;
	uint32_t					Address;
	uint8_t						Value;
	execstatus_t				Result;

	Rd							= ( OpCode >>  0 ) & 0x07;
	Rn							= ( OpCode >>  3 ) & 0x07;
	Rm							= ( OpCode >>  6 ) & 0x07;

	Address						= State->Core.Regs[Rn] + State->Core.Regs[Rm];

	if( (int64_t)State->Core.PC != State->LastDataBkpt && CheckReadBreakpoints( State, Address, 1 ))
		{
		State->Core.PC		   -=  2;
		return EXEC_STATUS_BREAKPOINT_READ;
		}

	if(( Result = LoadByte( State, Address, &Value )) >= EXEC_STATUS_BREAKPOINT_WRITE )
		{
		State->Core.Regs[Rd]	= (int32_t)(int8_t)Value;
		State->Core.Written[Rd]	= 1;
		}

	return Result;
	}/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--------+--------+--------+
 |15|14|13|12|11|10|09|08 07 06|05 04 03|02 01 00|
 +--+--+--+--+--+--+--+--------+--------+--------+
 | 0| 1| 0| 1| 1| 0| 0|---Rm---|---Rn---|---Rd---|
 +--+--+--+--+--+--+--+--------+--------+--------+
*/
static INLINE execstatus_t Ldr2( thumbstate_t *State, uint16_t OpCode )
	{
	unsigned int				Rd, Rn, Rm;
	uint32_t					Address;
	int							Result;

	Rd							= ( OpCode >>  0 ) & 0x07;
	Rn							= ( OpCode >>  3 ) & 0x07;
	Rm							= ( OpCode >>  6 ) & 0x07;

	Address						= State->Core.Regs[Rn] + State->Core.Regs[Rm];

	if( (int64_t)State->Core.PC != State->LastDataBkpt && CheckReadBreakpoints( State, Address, 4 ))
		{
		State->Core.PC		   -=  2;
		return EXEC_STATUS_BREAKPOINT_READ;
		}

	if(( Result = LoadWord( State, Address, &State->Core.Regs[Rd] )) >= EXEC_STATUS_OK )
		State->Core.Written[Rd]	= 1;

	return Result;
	}/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--------+--------+--------+
 |15|14|13|12|11|10|09|08 07 06|05 04 03|02 01 00|
 +--+--+--+--+--+--+--+--------+--------+--------+
 | 0| 1| 0| 1| 1| 0| 1|---Rm---|---Rn---|---Rd---|
 +--+--+--+--+--+--+--+--------+--------+--------+
*/
static INLINE execstatus_t Ldrh2( thumbstate_t *State, uint16_t OpCode )
	{
	unsigned int				Rd, Rn, Rm;
	uint32_t					Address;
	uint16_t					Value;
	execstatus_t				Result;

	Rd							= ( OpCode >>  0 ) & 0x07;
	Rn							= ( OpCode >>  3 ) & 0x07;
	Rm							= ( OpCode >>  6 ) & 0x07;

	Address						= State->Core.Regs[Rn] + State->Core.Regs[Rm];

	if( (int64_t)State->Core.PC != State->LastDataBkpt && CheckReadBreakpoints( State, Address, 2 ))
		{
		State->Core.PC		   -=  2;
		return EXEC_STATUS_BREAKPOINT_READ;
		}

	if(( Result = LoadHalfWord( State, Address, &Value )) >= EXEC_STATUS_BREAKPOINT_WRITE )
		{
		State->Core.Regs[Rd]	= (uint32_t)Value;
		State->Core.Written[Rd]	= 1;
		}

	return Result;
	}/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--------+--------+--------+
 |15|14|13|12|11|10|09|08 07 06|05 04 03|02 01 00|
 +--+--+--+--+--+--+--+--------+--------+--------+
 | 0| 1| 0| 1| 1| 1| 0|---Rm---|---Rn---|---Rd---|
 +--+--+--+--+--+--+--+--------+--------+--------+
*/
static INLINE execstatus_t Ldrb2( thumbstate_t *State, uint16_t OpCode )
	{
	unsigned int				Rd, Rn, Rm;
	uint32_t					Address;
	uint8_t						Value;
	execstatus_t				Result;

	Rd							= ( OpCode >>  0 ) & 0x07;
	Rn							= ( OpCode >>  3 ) & 0x07;
	Rm							= ( OpCode >>  6 ) & 0x07;

	Address						= State->Core.Regs[Rn] + State->Core.Regs[Rm];

	if( (int64_t)State->Core.PC != State->LastDataBkpt && CheckReadBreakpoints( State, Address, 1 ))
		{
		State->Core.PC		   -=  2;
		return EXEC_STATUS_BREAKPOINT_READ;
		}

	if(( Result = LoadByte( State, Address, &Value )) >= EXEC_STATUS_BREAKPOINT_WRITE )
		{
		State->Core.Regs[Rd]	= (uint32_t)Value;
		State->Core.Written[Rd]	= 1;
		}

	return Result;
	}/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--------+--------+--------+
 |15|14|13|12|11|10|09|08 07 06|05 04 03|02 01 00|
 +--+--+--+--+--+--+--+--------+--------+--------+
 | 0| 1| 0| 1| 1| 1| 1|---Rm---|---Rn---|---Rd---|
 +--+--+--+--+--+--+--+--------+--------+--------+
*/
static INLINE execstatus_t Ldrsh( thumbstate_t *State, uint16_t OpCode )
	{
	unsigned int				Rd, Rn, Rm;
	uint32_t					Address;
	uint16_t					Value;
	execstatus_t				Result;

	Rd							= ( OpCode >>  0 ) & 0x07;
	Rn							= ( OpCode >>  3 ) & 0x07;
	Rm							= ( OpCode >>  6 ) & 0x07;

	Address						= State->Core.Regs[Rn] + State->Core.Regs[Rm];

	if( (int64_t)State->Core.PC != State->LastDataBkpt && CheckReadBreakpoints( State, Address, 2 ))
		{
		State->Core.PC		   -=  2;
		return EXEC_STATUS_BREAKPOINT_READ;
		}

	if(( Result = LoadHalfWord( State, Address, &Value )) >= EXEC_STATUS_BREAKPOINT_WRITE )
		{
		State->Core.Regs[Rd]	= (int32_t)(int16_t)Value;
		State->Core.Written[Rd]	= 1;
		}

	return Result;
	}/*============================================================================*/
/*
 +--+--+--+--+--+--------------+--------+--------+
 |15|14|13|12|11|10|09|08|07|06|05 04 03|02 01 00|
 +--+--+--+--+--+--------------+--------+--------+
 | 0| 1| 1| 0| 0|-----Imm5-----|---Rn---|---Rd---|
 +--+--+--+--+--+--------------+--------+--------+
*/
static INLINE execstatus_t Str1( thumbstate_t *State, uint16_t OpCode )
	{
	unsigned int				Rd, Rn, Immediate;
	uint32_t					Address;

	Rd							= ( OpCode >>  0 ) & 0x07;
	Rn							= ( OpCode >>  3 ) & 0x07;
	Immediate					= ( OpCode >>  4 ) & 0x7c;

	Address						= State->Core.Regs[Rn] + Immediate;

	if( (int64_t)State->Core.PC != State->LastDataBkpt && CheckWriteBreakpoints( State, Address, 4 ))
		{
		State->Core.PC   -=  2;
		return EXEC_STATUS_BREAKPOINT_WRITE;
		}

	return StoreWord( State, Address, State->Core.Regs[Rd] );
	}/*============================================================================*/
/*
 +--+--+--+--+--+--------------+--------+--------+
 |15|14|13|12|11|10|09|08|07|06|05 04 03|02 01 00|
 +--+--+--+--+--+--------------+--------+--------+
 | 0| 1| 1| 0| 1|-----Imm5-----|---Rn---|---Rd---|
 +--+--+--+--+--+--------------+--------+--------+
*/
static INLINE execstatus_t Ldr1( thumbstate_t *State, uint16_t OpCode )
	{
	unsigned int				Rd, Rn, Immediate;
	uint32_t					Address;
	int							Result;

	Rd							= ( OpCode >>  0 ) & 0x07;
	Rn							= ( OpCode >>  3 ) & 0x07;
	Immediate					= ( OpCode >>  4 ) & 0x7c;

	Address						= State->Core.Regs[Rn] + Immediate;

	if( (int64_t)State->Core.PC != State->LastDataBkpt && CheckReadBreakpoints( State, Address, 4 ))
		{
		State->Core.PC		   -=  2;
		return EXEC_STATUS_BREAKPOINT_READ;
		}

	if(( Result = LoadWord( State, Address, &State->Core.Regs[Rd] )) >= EXEC_STATUS_OK )
		State->Core.Written[Rd]	= 1;

	return Result;
	}/*============================================================================*/
/*
 +--+--+--+--+--+--------------+--------+--------+
 |15|14|13|12|11|10|09|08|07|06|05 04 03|02 01 00|
 +--+--+--+--+--+--------------+--------+--------+
 | 0| 1| 1| 1| 0|-----Imm5-----|---Rn---|---Rd---|
 +--+--+--+--+--+--------------+--------+--------+
*/
static INLINE execstatus_t Strb1( thumbstate_t *State, uint16_t OpCode )
	{
	unsigned int				Rd, Rn, Immediate;
	uint32_t					Address;

	Rd							= ( OpCode >>  0 ) & 0x07;
	Rn							= ( OpCode >>  3 ) & 0x07;
	Immediate					= ( OpCode >>  6 ) & 0x1f;

	Address						= State->Core.Regs[Rn] + Immediate;

	if( (int64_t)State->Core.PC != State->LastDataBkpt && CheckWriteBreakpoints( State, Address, 1 ))
		{
		State->Core.PC		   -=  2;
		return EXEC_STATUS_BREAKPOINT_WRITE;
		}

	return StoreByte( State, Address, (uint8_t)State->Core.Regs[Rd] );
	}/*============================================================================*/
/*
 +--+--+--+--+--+--------------+--------+--------+
 |15|14|13|12|11|10|09|08|07|06|05 04 03|02 01 00|
 +--+--+--+--+--+--------------+--------+--------+
 | 0| 1| 1| 1| 1|-----Imm5-----|---Rn---|---Rd---|
 +--+--+--+--+--+--------------+--------+--------+
*/
static INLINE execstatus_t Ldrb1( thumbstate_t *State, uint16_t OpCode )
	{
	unsigned int				Rd, Rn, Immediate;
	uint32_t					Address;
	uint8_t						Value;
	execstatus_t				Result;

	Rd							= ( OpCode >>  0 ) & 0x07;
	Rn							= ( OpCode >>  3 ) & 0x07;
	Immediate					= ( OpCode >>  6 ) & 0x1f;

	Address						= State->Core.Regs[Rn] + Immediate;

	if( (int64_t)State->Core.PC != State->LastDataBkpt && CheckReadBreakpoints( State, Address, 1 ))
		{
		State->Core.PC		   -=  2;
		return EXEC_STATUS_BREAKPOINT_READ;
		}

	if(( Result = LoadByte( State, Address, &Value )) >= EXEC_STATUS_BREAKPOINT_WRITE )
		{
		State->Core.Regs[Rd]	= (uint32_t)Value;
		State->Core.Written[Rd]	= 1;
		}

	return Result;
	}/*============================================================================*/
/*
 +--+--+--+--+--+--------------+--------+--------+
 |15|14|13|12|11|10|09|08|07|06|05 04 03|02 01 00|
 +--+--+--+--+--+--------------+--------+--------+
 | 1| 0| 0| 0| 0|-----Imm5-----|---Rn---|---Rd---|
 +--+--+--+--+--+--------------+--------+--------+
*/
static INLINE execstatus_t Strh1( thumbstate_t *State, uint16_t OpCode )
	{
	unsigned int				Rd, Rn, Immediate;
	uint32_t					Address;

	Rd							= ( OpCode >>  0 ) & 0x07;
	Rn							= ( OpCode >>  3 ) & 0x07;
	Immediate					= ( OpCode >>  5 ) & 0x3e;

	Address						= State->Core.Regs[Rn] + Immediate;

	if( (int64_t)State->Core.PC != State->LastDataBkpt && CheckWriteBreakpoints( State, Address, 2 ))
		{
		State->Core.PC   -=  2;
		return EXEC_STATUS_BREAKPOINT_WRITE;
		}

	return StoreHalfWord( State, Address, (uint16_t)State->Core.Regs[Rd] );
	}/*============================================================================*/
/*
 +--+--+--+--+--+--------------+--------+--------+
 |15|14|13|12|11|10|09|08|07|06|05 04 03|02 01 00|
 +--+--+--+--+--+--------------+--------+--------+
 | 1| 0| 0| 0| 1|-----Imm5-----|---Rn---|---Rd---|
 +--+--+--+--+--+--------------+--------+--------+
*/
static INLINE execstatus_t Ldrh1( thumbstate_t *State, uint16_t OpCode )
	{
	unsigned int				Rd, Rn, Immediate;
	uint32_t					Address;
	uint16_t					Value;
	execstatus_t				Result;

	Rd							= ( OpCode >>  0 ) & 0x07;
	Rn							= ( OpCode >>  3 ) & 0x07;
	Immediate					= ( OpCode >>  5 ) & 0x3e;

	Address						= State->Core.Regs[Rn] + Immediate;

	if( (int64_t)State->Core.PC != State->LastDataBkpt && CheckReadBreakpoints( State, Address, 2 ))
		{
		State->Core.PC		   -=  2;
		return EXEC_STATUS_BREAKPOINT_READ;
		}

	if(( Result = LoadHalfWord( State, Address, &Value )) >= EXEC_STATUS_BREAKPOINT_WRITE )
		State->Core.Regs[Rd]	= (uint32_t)Value;

	return Result;
	}/*============================================================================*/
/*
 +--+--+--+--+--+--------+-----------------------+
 |15|14|13|12|11|10 09 08|07 06b05 04 03-02 01 00|
 +--+--+--+--+--+--------+-----------------------+
 | 1| 0| 0| 1| 0|---Rd---|---------Imm8----------|
 +--+--+--+--+--+--------+-----------------------+
*/
static INLINE execstatus_t Str3( thumbstate_t *State, uint16_t OpCode )
	{
	unsigned int				Rd, Immediate;
	uint32_t					Address;

	Rd							= ( OpCode >>  8 ) & 0x07;
	Immediate					= ( OpCode <<  2 ) & 0x3fc;

	Address						= State->Core.SP + Immediate;

	if( (int64_t)State->Core.PC != State->LastDataBkpt && CheckWriteBreakpoints( State, Address, 4 ))
		{
		State->Core.PC		   -=  2;
		return EXEC_STATUS_BREAKPOINT_WRITE;
		}

	return StoreWord( State, Address, State->Core.Regs[Rd] );
	}/*============================================================================*/
/*
 +--+--+--+--+--+--------+-----------------------+
 |15|14|13|12|11|10 09 08|07 06b05 04 03-02 01 00|
 +--+--+--+--+--+--------+-----------------------+
 | 1| 0| 0| 1| 0|---Rd---|---------Imm8----------|
 +--+--+--+--+--+--------+-----------------------+
*/
static INLINE execstatus_t Ldr4( thumbstate_t *State, uint16_t OpCode )
	{
	unsigned int				Rd, Immediate;
	uint32_t					Address;
	int							Result;

	Rd							= ( OpCode >>  8 ) & 0x07;
	Immediate					= ( OpCode <<  2 ) & 0x3fc;

	Address						= State->Core.SP + Immediate;

	if( (int64_t)State->Core.PC != State->LastDataBkpt && CheckReadBreakpoints( State, Address, 4 ))
		{
		State->Core.PC		   -=  2;
		return EXEC_STATUS_BREAKPOINT_READ;
		}

	if(( Result = LoadWord( State, Address, &State->Core.Regs[Rd] )) >= EXEC_STATUS_OK )
		State->Core.Written[Rd]	= 1;

	return Result;
	}/*============================================================================*/
/*
 +--+--+--+--+--+--------+-----------------------+
 |15|14|13|12|11|10 09 08|07 06 05 04 03 02 01 00|
 +--+--+--+--+--+--------+-----------------------+
 | 1| 0| 1| 0| 0|---Rd---|---------Imm8----------|
 +--+--+--+--+--+--------+-----------------------+
*/
static INLINE void Add5( thumbstate_t *State, uint16_t OpCode )
	{
	unsigned int			Rd, Immediate;

	Rd						= ( OpCode >>  8 ) & 0x07;
	Immediate				= ( OpCode <<  2 ) & 0x3fc;

	State->Core.Regs[Rd]	= (( State->Core.PC + 2 ) & 0xfffffffc ) + Immediate;
	State->Core.Written[Rd]	= 1;
	}/*============================================================================*/
/*
 +--+--+--+--+--+--------+-----------------------+
 |15|14|13|12|11|10 09 08|07 06 05 04 03 02 01 00|
 +--+--+--+--+--+--------+-----------------------+
 | 1| 0| 1| 0| 0|---Rd---|---------Imm8----------|
 +--+--+--+--+--+--------+-----------------------+
*/
static INLINE void Add6( thumbstate_t *State, uint16_t OpCode )
	{
	unsigned int			Rd, Immediate;

	Rd						= ( OpCode >>  8 ) & 0x07;
	Immediate				= ( OpCode <<  2 ) & 0x3fc;

	State->Core.Regs[Rd]	= State->Core.SP + Immediate;
	State->Core.Written[Rd]	= 1;
	}/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+--+--------------------+
 |15|14|13|12|11|10|09|08|07|06 05 04 03 02 01 00|
 +--+--+--+--+--+--+--+--+--+--------------------+
 | 1| 0| 1| 1| 0| 0| 0| 0| 0|--------Imm7--------|
 +--+--+--+--+--+--+--+--+--+--------------------+
*/
static INLINE void Add7( thumbstate_t *State, uint16_t Immediate )
	{
	State->Core.SP		   += Immediate;
	State->Core.Written[SP]	= 1;
	}
/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+--+--------------------+
 |15|14|13|12|11|10|09|08|07|06 05 04 03 02 01 00|
 +--+--+--+--+--+--+--+--+--+--------------------+
 | 1| 0| 1| 1| 0| 0| 0| 0| 1|--------Imm7--------|
 +--+--+--+--+--+--+--+--+--+--------------------+
*/


static INLINE void Sub4( thumbstate_t *State, uint16_t Immediate )
	{
	State->Core.SP		   -= Immediate;
	State->Core.Written[SP]	= 1;
	}
/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+--+--------------------+
 |15|14|13|12|11|10|09|08|07|06 05 04 03 02 01 00|
 +--+--+--+--+--+--+--+--+--+--------------------+
 | 1| 0| 1| 1| 0| 0| 0| 0|Op|--------Imm7--------|
 +--+--+--+--+--+--+--+--+--+--------------------+
*/
static INLINE void Add7Sub4( thumbstate_t *State, uint16_t OpCode )
	{
	unsigned int	Operation, Immediate;

	Operation	= ( OpCode >>  7 ) & 0x01;
	Immediate	= ( OpCode <<  2 ) & 0x1fc;

	switch( Operation )
		{
		case 0x00:
			Add7( State, Immediate );
			break;
		case 0x01:
			Sub4( State, Immediate );
			break;
		}
	}/*============================================================================*/

/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 |15|14|13|12|11|10|09|08|07|06|05 04 03|02 01 00|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 1| 0| 1| 1| 0| 0| 1| 0| 0| 0|---Rm---|---Rd---|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
*/
static INLINE void Sxth( thumbstate_t *State, unsigned int Rd, unsigned int Rm )
	{
	int32_t					Result;

	Result					= (int32_t)(int16_t)State->Core.Regs[Rm];

	State->Core.Regs[Rd]	= Result;
	State->Core.Written[Rd]	= 1;
	}
/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 |15|14|13|12|11|10|09|08|07|06|05 04 03|02 01 00|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 1| 0| 1| 1| 0| 0| 1| 0| 0| 1|---Rm---|---Rd---|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
*/
static INLINE void Sxtb( thumbstate_t *State, unsigned int Rd, unsigned int Rm )
	{
	int32_t					Result;

	Result					= (int32_t)(int8_t)State->Core.Regs[Rm];

	State->Core.Regs[Rd]	= Result;
	State->Core.Written[Rd]	= 1;
	}
/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 |15|14|13|12|11|10|09|08|07|06|05 04 03|02 01 00|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 1| 0| 1| 1| 0| 0| 1| 0| 1| 0|---Rm---|---Rd---|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
*/
static INLINE void Uxth( thumbstate_t *State, unsigned int Rd, unsigned int Rm )
	{
	uint32_t				Result;

	Result					= (uint32_t)(uint16_t)State->Core.Regs[Rm];

	State->Core.Regs[Rd]	= Result;
	State->Core.Written[Rd]	= 1;
	}
/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 |15|14|13|12|11|10|09|08|07|06|05 04 03|02 01 00|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 1| 0| 1| 1| 0| 0| 1| 0| 1| 1|---Rm---|---Rd---|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
*/
static INLINE void Uxtb( thumbstate_t *State, unsigned int Rd, unsigned int Rm )
	{
	uint32_t				Result;

	Result					= (uint32_t)(uint8_t)State->Core.Regs[Rm];

	State->Core.Regs[Rd]	= Result;
	State->Core.Written[Rd]	= 1;
	}
/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+-----+--------+--------+
 |15|14|13|12|11|10|09|08|07 06|05 04 03|02 01 00|
 +--+--+--+--+--+--+--+--+-----+--------+--------+
 | 1| 0| 1| 1| 0| 0| 1| 0|--Op-|---Rm---|---Rd---|
 +--+--+--+--+--+--+--+--+-----+--------+--------+
*/
static INLINE void XXTX( thumbstate_t *State, uint16_t OpCode )
	{
	unsigned int	Rd, Rm, Operation;

	Rd			= ( OpCode >>  0 ) & 0x07;
	Rm			= ( OpCode >>  3 ) & 0x07;
	Operation	= ( OpCode >>  6 ) & 0x03;

	switch( Operation )
		{
		case 0x00:
			Sxth( State, Rd, Rm );
			break;
		case 0x01:
			Sxtb( State, Rd, Rm );
			break;
		case 0x02:
			Uxth( State, Rd, Rm );
			break;
		case 0x03:
			Uxtb( State, Rd, Rm );
			break;
		}
	}/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+-----------------------+
 |15|14|13|12|11|10|09|08|07 06 05 04 03 02 01 00|
 +--+--+--+--+--+--+--+--+-----------------------+
 | 1| 0| 1| 1| 0| 1| 0|Lr|---------List----------|
 +--+--+--+--+--+--+--+--+-----------------------+
*/
static INLINE execstatus_t Push( thumbstate_t *State, uint16_t OpCode )
	{
	uint8_t					List, Lr, ListAux;
	uint32_t   				Address;
	unsigned int			Rx;
	execstatus_t			Result2, Result	= EXEC_STATUS_TO_CONTINUE;
	int						i;

	List					= ( OpCode >>  0 ) & 0xff;
	Lr						= ( OpCode >>  8 ) & 0x01;
	Address					= State->Core.SP;

   	if( (int64_t)State->Core.PC != State->LastDataBkpt )
		{
		/* Count how many registers are to be pushed. */
		for( i = Lr, ListAux = List; ListAux; ListAux >>= 1 )
			i   += ListAux & 0x01;
		/* There is at least one data read breakpoint in the affected area... */
		if( CheckWriteBreakpoints( State, Address - 4 * i, 4 * i ))
			{
			/* Back-up the PC so this instruction can be re-executed later. */
			State->Core.PC   -=  2;
			/* Return a status telling that the execution must stop. */
			return EXEC_STATUS_BREAKPOINT_WRITE;
			}
		}

	if( Lr )
		{
		Address			   -= 4;
		if(( Result2 = StoreWord( State, Address, State->Core.LR )) < EXEC_STATUS_BREAKPOINT_WRITE )
			return Result2;

		if( Result2 <= EXEC_STATUS_BREAKPOINT_EXEC )
			Result			= Result2;
		}

	for( Rx = 7, ListAux = List; ListAux; Rx--, ListAux <<= 1 )
		{
		if( ListAux & 0x80 )
			{
			Address		   -= 4;
			if(( Result2 = StoreWord( State, Address, State->Core.Regs[Rx] )) < EXEC_STATUS_BREAKPOINT_WRITE )
				return Result2;

			if( Result2 <= EXEC_STATUS_BREAKPOINT_EXEC )
				Result		= Result2;
			}
		}

    State->Core.SP			= Address;
	State->Core.Written[SP]	= 1;

    return Result;
	}/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+-----------------------+
 |15|14|13|12|11|10|09|08|07 06 05 04 03 02 01 00|
 +--+--+--+--+--+--+--+--+-----------------------+
 | 1| 0| 1| 1| 0| 1| 1| 0| 0| 1| 1|im| 0| A| I| F|	CPSxxxx
 +--+--+--+--+--+--+--+--+-----------------------+
 | 1| 0| 1| 1| 0| 1| 1| 0| 0| 1| 0| 1| E| 0| 0| 0|	SETEND
 +--+--+--+--+--+--+--+--+-----------------------+
*/
static INLINE execstatus_t SetEndCPS( thumbstate_t *State, uint16_t OpCode )
	{
	uint16_t	Enable; //, Mask;

	if(( OpCode & 0xffef ) != 0xb662 )
		{
		if( (int64_t)State->Core.PC != State->LastDataBkpt )
			State->Core.PC		   -=  2;

		return EXEC_STATUS_ERROR_INVALID_OPCODE;
		}

	Enable	= ( OpCode & 0x0010 ) != 0;
//	Mask	= OpCode & 0x0007;

	State->Core.PRIMASK.Value	= Enable;

	return EXEC_STATUS_TO_CONTINUE;
	}
/*============================================================================*/
static INLINE void Rev( thumbstate_t *State, unsigned int Rd, unsigned int Rn )
	{
	State->Core.Regs[Rd]	= (( State->Core.Regs[Rn] << 24 ) & 0xff000000uL ) | (( State->Core.Regs[Rn] <<  8 ) & 0x00ff0000uL ) |
					  (( State->Core.Regs[Rn] >>  8 ) & 0x0000ff00uL ) | (( State->Core.Regs[Rn] >> 24 ) & 0x000000ffuL );
	State->Core.Written[Rd]	= 1;
	}
/*============================================================================*/
static INLINE void Rev16( thumbstate_t *State, unsigned int Rd, unsigned int Rn )
	{
	State->Core.Regs[Rd]	= (( State->Core.Regs[Rn] >> 8 ) & 0x00ff00ffuL ) | (( State->Core.Regs[Rn] << 8 ) & 0xff00ff00uL );
	State->Core.Written[Rd]	= 1;
	}
/*============================================================================*/
static INLINE void Revsh( thumbstate_t *State, unsigned int Rd, unsigned int Rn )
	{
	State->Core.Regs[Rd]	= (int32_t)(uint16_t)((( State->Core.Regs[Rn] >> 8 ) & 0x000000ffuL ) | (( State->Core.Regs[Rn] << 8 ) & 0x0000ff00uL ));
	State->Core.Written[Rd]	= 1;
	}
/*============================================================================*/
static INLINE execstatus_t RevXX( thumbstate_t *State, uint16_t OpCode )
	{
	unsigned int	Rd, Rn, Operation;

	Rd			= ( OpCode >> 0 ) & 0x07;
	Rn			= ( OpCode >> 3 ) & 0x07;
	Operation	= ( OpCode >> 6 ) & 0x03;

	switch( Operation )
		{
		case 0x00:
			Rev( State, Rd, Rn );
			break;
		case 0x01:
			Rev16( State, Rd, Rn );
			break;
		case 0x02:
			if( (int64_t)State->Core.PC != State->LastDataBkpt )
				State->Core.PC		   -=  2;

			return EXEC_STATUS_ERROR_UNDEFINED_OPCODE;
		case 0x03:
			Revsh( State, Rd, Rn );
			break;
		}
	return EXEC_STATUS_TO_CONTINUE;
	}
/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+-----------------------+
 |15|14|13|12|11|10|09|08|07 06b05 04 03-02 01 00|
 +--+--+--+--+--+--+--+--+-----------------------+
 | 1| 0| 1| 1| 0| 1| 0|Pc|---------List----------|
 +--+--+--+--+--+--+--+--+-----------------------+
*/
static INLINE execstatus_t Pop( thumbstate_t *State, uint16_t OpCode )
	{
	uint16_t					List, Pc, Rx, ListAux;
	uint32_t					Address;
	execstatus_t				Result	= EXEC_STATUS_TO_CONTINUE;
	int							i;

	List						= ( OpCode >>  0 ) & 0xff;
	Pc							= ( OpCode >>  8 ) & 0x01;
    Address						= State->Core.SP;

   	if( (int64_t)State->Core.PC != State->LastDataBkpt )
		{
		/* Count how many registers are to be popped. */
		for( i = Pc, ListAux = List; ListAux; ListAux >>= 1 )
			i   += ListAux & 0x01;
		/* There is at least one data read breakpoint in the affected area... */
		if( CheckReadBreakpoints( State, Address, 4 * i ))
			{
			/* Back-up the PC so this instruction can be re-executed later. */
			State->Core.PC	   -=  2;
			/* Return a status telling that the execution must stop. */
			return EXEC_STATUS_BREAKPOINT_READ;
			}
		}

	for( Rx = 0, ListAux = List; ListAux; Rx++, ListAux >>= 1 )
		{
		if( ListAux & 0x01 )
			{
			execstatus_t		Result2;

			/* Read one word from the stack. If a real error happened... */
			if(( Result2 = LoadWord( State, Address, &State->Core.Regs[Rx] )) < EXEC_STATUS_BREAKPOINT_WRITE )
				/* ...we must stop now. */
				return Result2;

			State->Core.Written[Rx]	= 1;

			/* We found a data breakpoint... */
			if( Result2 <= EXEC_STATUS_BREAKPOINT_EXEC )
				/* ...let's update the return value. */
				Result				= Result2;

			Address		 		   += 4;
			}
		}

	/* The PC is to be popped also... */
	if( Pc )
		{
		execstatus_t			Result2;
		uint32_t				Temp;

		/* Read one word from the stack. If a real error happened... */
		if(( Result2 = LoadWord( State, Address, &Temp )) < EXEC_STATUS_BREAKPOINT_WRITE )
			/* ...we must stop now. */
			return Result2;

		/* We found a data breakpoint... */
		if( Result2 <= EXEC_STATUS_BREAKPOINT_EXEC )
			/* ...let's update the return value. */
			Result				= Result2;

		Address			 	   += 4;
		if( Temp >= 0xfffffff0 && State->Core.CPSR.EN != CPU_MODE_USER )
			{
			State->Core.SP		= Address;
			return RetI( State, Temp );
			}

		State->Core.PC			= Temp;
		}

    State->Core.SP				= Address;
	State->Core.Written[SP]		= 1;

    return Result;
	}/*============================================================================*/
static INLINE execstatus_t Bkpt( thumbstate_t *State, uint16_t OpCode )
	{
	if( (int64_t)State->Core.PC != State->LastDataBkpt )
		{
		State->Core.PC		   -=  2;
		return EXEC_STATUS_BREAKPOINT_EXEC;
		}

	return EXEC_STATUS_TO_CONTINUE;
	}/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+-----------------------+
 |15|14|13|12|11|10|09|08|07 06 05 04 03 02 01 00|
 +--+--+--+--+--+--+--+--+-----------------------+
 | 1| 0| 1| 1| 1| 1| 1| 1| 0| 0| 1| 1| 0| 0| 0| 0|	WFI
 +--+--+--+--+--+--+--+--+-----------------------+
*/
static INLINE execstatus_t WFI( thumbstate_t *State, uint16_t OpCode )
	{
	/* NOP or YIELD... */
	if(( OpCode & 0xffef ) == 0xbf00 )
		return EXEC_STATUS_TO_CONTINUE;
	/* Not a WFI... s*/
	else if( OpCode != 0xbf30 )
		{
		if( (int64_t)State->Core.PC != State->LastDataBkpt )
			State->Core.PC		   -=  2;

		return EXEC_STATUS_ERROR_INVALID_OPCODE;
		}

	if( !State->InterruptState.I )
		State->Core.PC -= 2;

	return EXEC_STATUS_TO_CONTINUE;
	}
/*============================================================================*/
/*
; +--+--+--+--+--+--------+--+--+--+--+--+--+--+--+
; |15|14|13|12|11|10 09 08|07|06|05|04|03|02|01|00|
; +--+--+--+--+--+--------+--+--+--+--+--+--+--+--+
; | 1| 1| 0| 0| 0|---Rn---|r7|r6|r5|r4|r3|r2|r1|r0|
; +--+--+--+--+--+--------+--+--+--+--+--+--+--+--+
*/
static INLINE execstatus_t Stmia( thumbstate_t *State, uint16_t OpCode )
	{
	uint8_t					List;
	unsigned int			Rn, Rx, ListAux;
	uint32_t				Address;
	execstatus_t			Result2, Result	= EXEC_STATUS_TO_CONTINUE;
	int						i;

	Rn						= ( OpCode >>  8 ) & 0x07;
	List					= ( OpCode >>  0 ) & 0xff;

	Address					= State->Core.Regs[Rn];

   	if( (int64_t)State->Core.PC != State->LastDataBkpt )
		{
		/* Count how many registers are to be pushed. */
		for( i = 0, ListAux = List; ListAux; ListAux >>= 1 )
			i   += ListAux & 0x01;
		/* There is at least one data read breakpoint in the affected area... */
		if( CheckWriteBreakpoints( State, Address - 4 * i, 4 * i ))
			{
			/* Back-up the PC so this instruction can be re-executed later. */
			State->Core.PC -=  2;
			/* Return a status telling that the execution must stop. */
			return EXEC_STATUS_BREAKPOINT_WRITE;
			}
		}

	for( Rx = 0, ListAux = List; ListAux; ListAux >>= 1, Rx++ )
		{
		if( ListAux & 0x01 )
			{
			if(( Result2 = StoreWord( State, Address, State->Core.Regs[Rx] )) < EXEC_STATUS_BREAKPOINT_WRITE )
				return Result2;
			Address		   += 4;

			if( Result2 <= EXEC_STATUS_BREAKPOINT_EXEC )
				Result		= Result2;
			}
		}


	State->Core.Regs[Rn]	= Address;
	State->Core.Written[Rn]	= 1;

	return Result;
	}
/*============================================================================*/
/*
; +--+--+--+--+--+--------+--+--+--+--+--+--+--+--+
; |15|14|13|12|11|10 09 08|07|06|05|04|03|02|01|00|
; +--+--+--+--+--+--------+--+--+--+--+--+--+--+--+
; | 1| 1| 0| 0| 1|---Rn---|r7|r6|r5|r4|r3|r2|r1|r0|
; +--+--+--+--+--+--------+--+--+--+--+--+--+--+--+
*/
static INLINE execstatus_t Ldmia( thumbstate_t *State, uint16_t OpCode )
	{
	unsigned int				Rn, List, Wb, Rx, ListAux;
	uint32_t					Address;
	execstatus_t				Result2, Result	= EXEC_STATUS_TO_CONTINUE;
	int							i;

	Rn							= ( OpCode >>  8 ) & 0x07;
	List						= ( OpCode >>  0 ) & 0xff;
	Wb							= ( ~List >> Rn ) & 0x01;

	Address						= State->Core.Regs[Rn];

   	if( (int64_t)State->Core.PC != State->LastDataBkpt )
		{
		/* Count how many registers are to be popped. */
		for( i = 0, ListAux = List; ListAux; ListAux >>= 1 )
			i   += ListAux & 0x01;
		/* There is at least one data read breakpoint in the affected area... */
		if( CheckReadBreakpoints( State, Address, 4 * i ))
			{
			/* Back-up the PC so this instruction can be re-executed later. */
			State->Core.PC	   -=  2;
			/* Return a status telling that the execution must stop. */
			return EXEC_STATUS_BREAKPOINT_READ;
			}
		}
	for( Rx = 0, ListAux = List; ListAux; ListAux >>= 1, Rx++ )
		{
		if( ListAux & 0x01 )
			{
			if(( Result2 = LoadWord( State, Address, &State->Core.Regs[Rx] )) < EXEC_STATUS_BREAKPOINT_WRITE )
				return Result2;
			State->Core.Written[Rx]	= 1;
			Address			   += 4;

			if( Result2 <= EXEC_STATUS_BREAKPOINT_EXEC )
				Result			= Result2;
			}
		}

	if( Wb )
		{
		State->Core.Regs[Rn]	= Address;
		State->Core.Written[Rn]	= 1;
		}

	return Result;
	}
/*============================================================================*/
/*
 +--+--+--+--+-----------+-----------------------+
 |15|14|13|12|11 10 09 08|07 06 05 04 03 02 01 00|
 +--+--+--+--+-----------+-----------------------+
 | 1| 1| 0| 1|---Cond----|----------Imm8---------|
 +--+--+--+--+-----------+-----------------------+
*/
static INLINE void Beq( thumbstate_t *State, uint16_t OpCode )
	{
	int32_t						Offset;

	if( State->Core.CPSR.Z )
		{
		Offset					= ( (int32_t)(int8_t)OpCode ) << 1;
		State->Core.PC		   += Offset + 2;
		State->Core.Written[PC]	= 1;
		}
	}/*============================================================================*/
/*
 +--+--+--+--+-----------+-----------------------+
 |15|14|13|12|11 10 09 08|07 06 05 04 03 02 01 00|
 +--+--+--+--+-----------+-----------------------+
 | 1| 1| 0| 1|---Cond----|----------Imm8---------|
 +--+--+--+--+-----------+-----------------------+
*/
static INLINE void Bne( thumbstate_t *State, uint16_t OpCode )
	{
	int32_t						Offset;

	if( ! State->Core.CPSR.Z )
		{
		Offset					= ( (int32_t)(int8_t)OpCode ) << 1;
		State->Core.PC		   += Offset + 2;
		State->Core.Written[PC]	= 1;
		}
	}/*============================================================================*/
/*
 +--+--+--+--+-----------+-----------------------+
 |15|14|13|12|11 10 09 08|07 06 05 04 03 02 01 00|
 +--+--+--+--+-----------+-----------------------+
 | 1| 1| 0| 1|---Cond----|----------Imm8---------|
 +--+--+--+--+-----------+-----------------------+
*/
static INLINE void Bcs( thumbstate_t *State, uint16_t OpCode )
	{
	int32_t						Offset;

	if( State->Core.CPSR.C )
		{
		Offset					= ( (int32_t)(int8_t)OpCode ) << 1;
		State->Core.PC		   += Offset + 2;
		State->Core.Written[PC]	= 1;
		}
	}/*============================================================================*/
/*
 +--+--+--+--+-----------+-----------------------+
 |15|14|13|12|11 10 09 08|07 06 05 04 03 02 01 00|
 +--+--+--+--+-----------+-----------------------+
 | 1| 1| 0| 1|---Cond----|----------Imm8---------|
 +--+--+--+--+-----------+-----------------------+
*/
static INLINE void Bcc( thumbstate_t *State, uint16_t OpCode )
	{
	int32_t						Offset;

	if( ! State->Core.CPSR.C )
		{
		Offset					= ( (int32_t)(int8_t)OpCode ) << 1;
		State->Core.PC		   += Offset + 2;
		State->Core.Written[PC]	= 1;
		}
	}/*============================================================================*/
/*
 +--+--+--+--+-----------+-----------------------+
 |15|14|13|12|11 10 09 08|07 06 05 04 03 02 01 00|
 +--+--+--+--+-----------+-----------------------+
 | 1| 1| 0| 1|---Cond----|----------Imm8---------|
 +--+--+--+--+-----------+-----------------------+
*/
static INLINE void Bmi( thumbstate_t *State, uint16_t OpCode )
	{
	int32_t						Offset;

	if( State->Core.CPSR.N )
		{
		Offset					= ( (int32_t)(int8_t)OpCode ) << 1;
		State->Core.PC		   += Offset + 2;
		State->Core.Written[PC]	= 1;
		}
	}/*============================================================================*/
/*
 +--+--+--+--+-----------+-----------------------+
 |15|14|13|12|11 10 09 08|07 06 05 04 03 02 01 00|
 +--+--+--+--+-----------+-----------------------+
 | 1| 1| 0| 1|---Cond----|----------Imm8---------|
 +--+--+--+--+-----------+-----------------------+
*/
static INLINE void Bpl( thumbstate_t *State, uint16_t OpCode )
	{
	int32_t						Offset;

	if( ! State->Core.CPSR.N )
		{
		Offset					= ( (int32_t)(int8_t)OpCode ) << 1;
		State->Core.PC		   += Offset + 2;
		State->Core.Written[PC]	= 1;
		}
	}/*============================================================================*/
/*
 +--+--+--+--+-----------+-----------------------+
 |15|14|13|12|11 10 09 08|07 06 05 04 03 02 01 00|
 +--+--+--+--+-----------+-----------------------+
 | 1| 1| 0| 1|---Cond----|----------Imm8---------|
 +--+--+--+--+-----------+-----------------------+
*/
static INLINE void Bvs( thumbstate_t *State, uint16_t OpCode )
	{
	int32_t						Offset;

	if( State->Core.CPSR.V )
		{
		Offset					= ( (int32_t)(int8_t)OpCode ) << 1;
		State->Core.PC		   += Offset + 2;
		State->Core.Written[PC]	= 1;
		}
	}/*============================================================================*/
/*
 +--+--+--+--+-----------+-----------------------+
 |15|14|13|12|11 10 09 08|07 06 05 04 03 02 01 00|
 +--+--+--+--+-----------+-----------------------+
 | 1| 1| 0| 1|---Cond----|----------Imm8---------|
 +--+--+--+--+-----------+-----------------------+
*/
static INLINE void Bvc( thumbstate_t *State, uint16_t OpCode )
	{
	int32_t						Offset;

	if( ! State->Core.CPSR.V )
		{
		Offset					= ( (int32_t)(int8_t)OpCode ) << 1;
		State->Core.PC		   += Offset + 2;
		State->Core.Written[PC]	= 1;
		}
	}/*============================================================================*/
/*
 +--+--+--+--+-----------+-----------------------+
 |15|14|13|12|11 10 09 08|07 06 05 04 03 02 01 00|
 +--+--+--+--+-----------+-----------------------+
 | 1| 1| 0| 1|---Cond----|----------Imm8---------|
 +--+--+--+--+-----------+-----------------------+
*/
static INLINE void Bhi( thumbstate_t *State, uint16_t OpCode )
	{
	int32_t						Offset;

	if( State->Core.CPSR.C && ! State->Core.CPSR.Z )
		{
		Offset					= ( (int32_t)(int8_t)OpCode ) << 1;
		State->Core.PC		   += Offset + 2;
		State->Core.Written[PC]	= 1;
		}
	}/*============================================================================*/
/*
 +--+--+--+--+-----------+-----------------------+
 |15|14|13|12|11 10 09 08|07 06 05 04 03 02 01 00|
 +--+--+--+--+-----------+-----------------------+
 | 1| 1| 0| 1|---Cond----|----------Imm8---------|
 +--+--+--+--+-----------+-----------------------+
*/
static INLINE void Bls( thumbstate_t *State, uint16_t OpCode )
	{
	int32_t						Offset;

	if( ! State->Core.CPSR.C || State->Core.CPSR.Z )
		{
		Offset					= ( (int32_t)(int8_t)OpCode ) << 1;
		State->Core.PC		   += Offset + 2;
		State->Core.Written[PC]	= 1;
		}
	}/*============================================================================*/
/*
 +--+--+--+--+-----------+-----------------------+
 |15|14|13|12|11 10 09 08|07 06 05 04 03 02 01 00|
 +--+--+--+--+-----------+-----------------------+
 | 1| 1| 0| 1|---Cond----|----------Imm8---------|
 +--+--+--+--+-----------+-----------------------+
*/
static INLINE void Bge( thumbstate_t *State, uint16_t OpCode )
	{
	int32_t						Offset;

	if( State->Core.CPSR.N == State->Core.CPSR.V )
		{
		Offset					= ( (int32_t)(int8_t)OpCode ) << 1;
		State->Core.PC		   += Offset + 2;
		State->Core.Written[PC]	= 1;
		}
	}/*============================================================================*/
/*
 +--+--+--+--+-----------+-----------------------+
 |15|14|13|12|11 10 09 08|07 06 05 04 03 02 01 00|
 +--+--+--+--+-----------+-----------------------+
 | 1| 1| 0| 1|---Cond----|----------Imm8---------|
 +--+--+--+--+-----------+-----------------------+
*/
static INLINE void Blt( thumbstate_t *State, uint16_t OpCode )
	{
	int32_t						Offset;

	if( State->Core.CPSR.N != State->Core.CPSR.V )
		{
		Offset					= ( (int32_t)(int8_t)OpCode ) << 1;
		State->Core.PC		   += Offset + 2;
		State->Core.Written[PC]	= 1;
		}
	}/*============================================================================*/
/*
 +--+--+--+--+-----------+-----------------------+
 |15|14|13|12|11 10 09 08|07 06 05 04 03 02 01 00|
 +--+--+--+--+-----------+-----------------------+
 | 1| 1| 0| 1|---Cond----|----------Imm8---------|
 +--+--+--+--+-----------+-----------------------+
*/
static INLINE void Bgt( thumbstate_t *State, uint16_t OpCode )
	{
	int32_t						Offset;

	if( ! State->Core.CPSR.Z && ( State->Core.CPSR.N == State->Core.CPSR.V ))
		{
		Offset					= ( (int32_t)(int8_t)OpCode ) << 1;
		State->Core.PC		   += Offset + 2;
		State->Core.Written[PC]	= 1;
		}
	}/*============================================================================*/
/*
 +--+--+--+--+-----------+-----------------------+
 |15|14|13|12|11 10 09 08|07 06 05 04 03 02 01 00|
 +--+--+--+--+-----------+-----------------------+
 | 1| 1| 0| 1|---Cond----|----------Imm8---------|
 +--+--+--+--+-----------+-----------------------+
*/
static INLINE void Ble( thumbstate_t *State, uint16_t OpCode )
	{
	int32_t						Offset;

	if( State->Core.CPSR.Z || ( State->Core.CPSR.N != State->Core.CPSR.V ))
		{
		Offset					= ( (int32_t)(int8_t)OpCode ) << 1;
		State->Core.PC		   += Offset + 2;
		State->Core.Written[PC]	= 1;
		}
	}/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+-----------------------+
 |15|14|13|12|11|10|09|08|07 06 05 04 03 02 01 00|
 +--+--+--+--+--+--+--+--+-----------------------+
 | 1| 1| 0| 1| 1| 1| 1| 1|---------Imm8----------|
 +--+--+--+--+--+--+--+--+-----------------------+
*/
static INLINE execstatus_t Swi( thumbstate_t *State, uint16_t OpCode )
	{
	return CallI( State, 11 );
	}/*============================================================================*/
/*
 +--+--+--+--+--+--------------------------------+
 |15|14|13|12|11|10 09 08 07 06 05 04 03 02 01 00|
 +--+--+--+--+--+--------------------------------+
 | 1| 1| 1| 0| 0|--------------Imm11-------------|
 +--+--+--+--+--+--------------------------------+
*/
static INLINE execstatus_t B2( thumbstate_t *State, uint16_t OpCode )
	{
	int32_t					Target;
	int						i;

	Target		 			= State->Core.PC + ((( (int32_t)(int8_t)( OpCode >> 3 )) << 4 ) | ( (uint8_t)OpCode << 1 )) + 2;

	for( i = 0; i < State->NumMemoryAreas; i++ )
		if( State->MemoryAreas[i] != NULL && Target >= State->MemoryAreas[i]->BaseAddress && Target <= State->MemoryAreas[i]->LastAddress )
			break;

	if( i >= State->NumMemoryAreas )
		{
		State->Core.PC		   -= 2;
		return EXEC_STATUS_ERROR_UNDEFINED_ADDRESS;
		}

	State->Core.PC			= Target | 1;
	State->Core.Written[PC]	= 1;

   	return EXEC_STATUS_TO_CONTINUE;
	}/*============================================================================*/
/*
 +--+--+--+--+--+--------------------------------+
 |15|14|13|12|11|10 09 08 07 06 05 04 03 02 01 00|
 +--+--+--+--+--+--------------------------------+
 | 1| 1| 1| 1| 1|--------------Imm11-------------|
 +--+--+--+--+--+--------------------------------+
*/
static INLINE execstatus_t BlSuffix( thumbstate_t *State, uint16_t OpCode )
	{
	int32_t	Target;
	int		i;

	Target					= State->Core.LR + (( OpCode << 1 ) & 0x0ffe );

	for( i = 0; i < State->NumMemoryAreas; i++ )
		if( State->MemoryAreas[i] != NULL && Target >= State->MemoryAreas[i]->BaseAddress && Target <= State->MemoryAreas[i]->LastAddress )
			break;

	if( i >= State->NumMemoryAreas )
		{
		State->Core.PC	   -= 2;
		return EXEC_STATUS_ERROR_UNDEFINED_ADDRESS;
		}

	State->Core.LR			= State->Core.PC | 1;
	State->Core.Written[LR]	= 1;

   	State->Core.PC			= Target | 1;
	State->Core.Written[PC]	= 1;

   	return EXEC_STATUS_TO_CONTINUE;
	}
/*============================================================================*/
static INLINE execstatus_t BlBlx( thumbstate_t *State, uint16_t OpCode, uint16_t OpCode2 )
	{
	execstatus_t			Result;
	int32_t					Offset;

	Offset					= (( (int32_t)(int8_t)( OpCode >> 3 )) << 15 ) | (( OpCode & 7 ) << 12 );

	State->Core.LR			= State->Core.PC + Offset;
	State->Core.Written[LR]	= 1;

	if(( Result = BlSuffix( State, OpCode2 )) < EXEC_STATUS_OK )
		{
		State->Core.PC	   -= 4;
		return Result;
		}

	return EXEC_STATUS_TO_CONTINUE;
	}
/*============================================================================*/
static INLINE execstatus_t Barrier( thumbstate_t *State, uint16_t OpCode2 )
	{
	return EXEC_STATUS_TO_CONTINUE;
	}
/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 | 1| 1| 1| 1| 0| 0| 1| 1| 1| 0| 0| R|    Rd     |   | 1| 0| 0| 0|    Mask   | 0| 0| 0|     Reg      |
 +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*/
static INLINE execstatus_t MSR( thumbstate_t *State, uint16_t OpCode, uint16_t OpCode2 )
	{
	unsigned int	Rd, Reg;

	Rd	= ( OpCode >> 0 ) & 0x000f;

	if( Rd == PC )
		{
		State->Core.PC -= 4;
		return EXEC_STATUS_ERROR_INVALID_OPCODE;
		}

	Reg		= ( OpCode2 >> 0 ) & 0x001f;

	switch( Reg )
		{
		default:
			State->Core.PC -= 4;
			return EXEC_STATUS_ERROR_INVALID_OPCODE;
		case 0x00:
			State->Core.CPSR.Value	= ( State->Core.CPSR.Value & ~0xf0000000 ) | ( State->Core.Regs[Rd] & 0xf0000000 );
			break;
		case 0x01:
			State->Core.CPSR.Value	= ( State->Core.CPSR.Value & ~0xf000003f ) | ( State->Core.Regs[Rd] & 0xf000003f );
			break;
		case 0x02:
			State->Core.CPSR.Value	= ( State->Core.CPSR.Value & ~0xf1000000 ) | ( State->Core.Regs[Rd] & 0xf1000000 );
			break;
		case 0x03:
			State->Core.CPSR.Value	= ( State->Core.CPSR.Value & ~0xf100003f ) | ( State->Core.Regs[Rd] & 0xf100003f );
			break;
		case 0x05:
			State->Core.CPSR.Value	= ( State->Core.CPSR.Value & ~0x0000003f ) | ( State->Core.Regs[Rd] & 0x0000003f );
			break;
		case 0x06:
			State->Core.CPSR.Value	= ( State->Core.CPSR.Value & ~0x01000000 ) | ( State->Core.Regs[Rd] & 0x01000000 );
			break;
		case 0x07:
			State->Core.CPSR.Value	= ( State->Core.CPSR.Value & ~0x0100003f ) | ( State->Core.Regs[Rd] & 0x0100003f );
			break;
		case 0x08:
			State->Core.MSP				= State->Core.Regs[Rd];
			if( State->Core.CONTROL.ASPSEL == 0 )
				State->Core.SP			= State->Core.Regs[Rd];
			break;
		case 0x09:
			State->Core.PSP				= State->Core.Regs[Rd];
			if( State->Core.CONTROL.ASPSEL != 0 )
				State->Core.SP			= State->Core.Regs[Rd];
			break;
		case 0x10:
			State->Core.PRIMASK.Value	= State->Core.Regs[Rd] & 0x00000001;
			break;
		case 0x14:
			State->Core.CONTROL.Value	= State->Core.Regs[Rd] & 0x00000002;
			break;
		}

	return EXEC_STATUS_TO_CONTINUE;
	}
/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 | 1| 1| 1| 1| 0| 0| 1| 1| 1| 1| 1| R| 1| 1| 1| 1|   | 1| 0| 0| 0|     Rd    | 0| 0| 0|     Reg      |
 +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*/
static INLINE execstatus_t MRS( thumbstate_t *State, uint16_t OpCode2 )
	{
	unsigned int	Rd, Reg;

	Rd	= ( OpCode2 >> 8 ) & 0x000f;

	if( Rd == PC )
		{
		State->Core.PC -= 4;
		return EXEC_STATUS_ERROR_INVALID_OPCODE;
		}

	Reg	= ( OpCode2 >> 0 ) & 0x001f;

	switch( Reg )
		{
		default:
			State->Core.PC -= 4;
			return EXEC_STATUS_ERROR_INVALID_OPCODE;
		case 0x00:
			State->Core.Regs[Rd]	= State->Core.CPSR.Value & 0xf0000000;
			break;
		case 0x01:
			State->Core.Regs[Rd]	= State->Core.CPSR.Value & 0xf000003f;
			break;
		case 0x02:
			State->Core.Regs[Rd]	= State->Core.CPSR.Value & 0xf1000000;
			break;
		case 0x03:
			State->Core.Regs[Rd]	= State->Core.CPSR.Value & 0xf100003f;
			break;
		case 0x05:
			State->Core.Regs[Rd]	= State->Core.CPSR.Value & 0x0000003f;
			break;
		case 0x06:
			State->Core.Regs[Rd]	= State->Core.CPSR.Value & 0x01000000;
			break;
		case 0x07:
			State->Core.Regs[Rd]	= State->Core.CPSR.Value & 0x0100003f;
			break;
		case 0x08:
			if( State->Core.CONTROL.ASPSEL == 0 )
				State->Core.MSP		= State->Core.SP;
			State->Core.Regs[Rd]	= State->Core.MSP;
			break;
		case 0x09:
			if( State->Core.CONTROL.ASPSEL != 0 )
				State->Core.PSP		= State->Core.SP;
			State->Core.Regs[Rd]	= State->Core.PSP;
			break;
		case 0x10:
			State->Core.Regs[Rd]	= State->Core.PRIMASK.Value;
			break;
		case 0x14:
			State->Core.Regs[Rd]	= State->Core.CONTROL.Value;
			break;
		}

	return EXEC_STATUS_TO_CONTINUE;
	}
/*============================================================================*/
/*
 +--+--+--+--+--+--------------------------------+
 |15|14|13|12|11|10 09 08 07 06 05 04 03 02 01 00|
 +--+--+--+--+--+--------------------------------+
 | 1| 1| 1| 1| 0|--------------Imm11-------------|
 +--+--+--+--+--+--------------------------------+
*/
static INLINE execstatus_t BlBlxMisc( thumbstate_t *State, uint16_t OpCode )
	{
	execstatus_t			Result;
	uint16_t				OpCode2;

	/* Fetch the second part of the opcode. */
	if(( Result = FetchOpCode( State, State->Core.PC & ~(sim_address_t)1, &OpCode2 )) < EXEC_STATUS_OK )
		return Result;

	State->Core.PC += 2;

	if(( OpCode2 & 0xf800 ) == 0xf800 )
		return BlBlx( State, OpCode, OpCode2 );
	if( OpCode == 0xf3bf && (( OpCode2 & 0xfff0 ) >= 0x8f40 || ( OpCode2 & 0xfff0 ) <= 0x8f60 ))
		return Barrier( State, OpCode2 );
	if(( OpCode & 0xfff0 ) == 0xf380 && ( OpCode2 & 0xffe0 ) == 0x8800 )
		return MSR( State, OpCode, OpCode2 );
	if( OpCode == 0xf3ef && ( OpCode2 & 0xf0e0 ) == 0x8000 )
		return MRS( State, OpCode2 );

	if( (int64_t)State->Core.PC != State->LastDataBkpt )
		State->Core.PC		   -=  4;

	return EXEC_STATUS_ERROR_INVALID_OPCODE;
	}/*============================================================================*/
static execstatus_t ThumbExec( thumbstate_t *State )
	{
	uint16_t		OpCode;
	execstatus_t	Result;
	sim_address_t	Address;
	sim_mapentry_t	*MapEntry;

	if( State == NULL )
		return EXEC_STATUS_ERROR_UNINITIALIZED;

	/* Bit zero of the PC is zero... */
	if(( State->Core.PC & 1 ) == 0 )
		/* ...which means the processor is in ARM mode, which is not supported. */
		return EXEC_STATUS_ERROR_INVALID_MODE;

	Address	= State->Core.PC & ~(sim_address_t)1;

	/* To fetch the opcode we must zero bit zero of the PC. */
	if(( Result = FetchOpCode( State, Address, &OpCode )) < EXEC_STATUS_OK )
		return Result;


	State->Core.PC	   += 2;

	Result				= EXEC_STATUS_TO_CONTINUE;

	switch( (uint8_t)( OpCode >> 8 ))
		{
		/* LSL */
		case 0x00: case 0x01: case 0x02: case 0x03:
		case 0x04: case 0x05: case 0x06: case 0x07:
			Lsl1( State, OpCode );
			break;

		/* LSR */
		case 0x08: case 0x09: case 0x0a: case 0x0b:
		case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			Lsr1( State, OpCode );
			break;

		/* ASR */
		case 0x10: case 0x11: case 0x12: case 0x13:
		case 0x14: case 0x15: case 0x16: case 0x17:
			Asr1( State, OpCode );
			break;

		/* ADD(3) */
		case 0x18: case 0x19:
			Add3( State, OpCode );
			break;

		/* SUB(3) */
		case 0x1a: case 0x1b:
			Sub3( State, OpCode );
			break;

		/* ADD(1) */
		case 0x1c: case 0x1d:
			Add1( State, OpCode );
			break;

		/* SUB(1) */
		case 0x1e: case 0x1f:
			Sub1( State, OpCode );
			break;

		/* MOV(1) */
		case 0x20: case 0x21: case 0x22: case 0x23:
		case 0x24: case 0x25: case 0x26: case 0x27:
			Mov1( State, OpCode );
			break;

		/* CMP(1) */
		case 0x28: case 0x29: case 0x2a: case 0x2b:
		case 0x2c: case 0x2d: case 0x2e: case 0x2f:
			Cmp1( State, OpCode );
			break;

		/* ADD(2) */
		case 0x30: case 0x31: case 0x32: case 0x33:
		case 0x34: case 0x35: case 0x36: case 0x37:
			Add2( State, OpCode );
			break;

		/* SUB(2) */
		case 0x38: case 0x39: case 0x3a: case 0x3b:
		case 0x3c: case 0x3d: case 0x3e: case 0x3f:
			Sub2( State, OpCode );
			break;

		/* AND,EOR,LSL(2),LSR(2) */
		case 0x40:
		/* ASR(2),ADC,SBC,ROR */
		case 0x41:
		/* TST,NEG,CMP(2),CMN */
		case 0x42:
		/* ORR,MUL,BIC,MVN */
		case 0x43:
			ALU( State, OpCode );
			break;

		/* ADD(4) */
		case 0x44:
			Add4( State, OpCode );
			break;

		/* CMP(3) */
		case 0x45:
			Cmp3( State, OpCode );
			break;

		/* CPY */
		case 0x46:
			Cpy( State, OpCode );
			break;

		/* BX,BLX(2) */
		case 0x47:
			Result	= BxBlx2( State, OpCode );
			break;

		/* LDR(3) */
		case 0x48: case 0x49: case 0x4a: case 0x4b:
		case 0x4c: case 0x4d: case 0x4e: case 0x4f:
			Result	= Ldr3( State, OpCode );
			break;

		/* STR(2) */
		case 0x50: case 0x51:
			Result	= Str2( State, OpCode );
			break;

		/* STRH(2) */
		case 0x52: case 0x53:
			Result	= Strh2( State, OpCode );
			break;

		/* STRB(2) */
		case 0x54: case 0x55:
			Result	= Strb2( State, OpCode );
			break;

		/* LDRSB */
		case 0x56: case 0x57:
			Result	= Ldrsb( State, OpCode );
			break;

		/* LDR(2) */
		case 0x58: case 0x59:
			Result	= Ldr2( State, OpCode );
			break;

		/* LDRH(2) */
		case 0x5a: case 0x5b:
			Result	= Ldrh2( State, OpCode );
			break;

		/* LDRB(2) */
		case 0x5c: case 0x5d:
			Result	= Ldrb2( State, OpCode );
			break;

		/* LDRSH */
		case 0x5e: case 0x5f:
			Result	= Ldrsh( State, OpCode );
			break;

		/* STR(1) */
		case 0x60: case 0x61: case 0x62: case 0x63:
		case 0x64: case 0x65: case 0x66: case 0x67:
			Result	= Str1( State, OpCode );
			break;

		/* LDR(1) */
		case 0x68: case 0x69: case 0x6a: case 0x6b:
		case 0x6c: case 0x6d: case 0x6e: case 0x6f:
			Result	= Ldr1( State, OpCode );
			break;

		/* STRB(1) */
		case 0x70: case 0x71: case 0x72: case 0x73:
		case 0x74: case 0x75: case 0x76: case 0x77:
			Result	= Strb1( State, OpCode );
			break;

		/* LDRB(1) */
		case 0x78: case 0x79: case 0x7a: case 0x7b:
		case 0x7c: case 0x7d: case 0x7e: case 0x7f:
			Result	= Ldrb1( State, OpCode );
			break;

		/* STRH(1) */
		case 0x80: case 0x81: case 0x82: case 0x83:
		case 0x84: case 0x85: case 0x86: case 0x87:
			Result	= Strh1( State, OpCode );
			break;

		/* LDRH(1) */
		case 0x88: case 0x89: case 0x8a: case 0x8b:
		case 0x8c: case 0x8d: case 0x8e: case 0x8f:
			Result	= Ldrh1( State, OpCode );
			break;

		/* STR(3) */
		case 0x90: case 0x91: case 0x92: case 0x93:
		case 0x94: case 0x95: case 0x96: case 0x97:
			Result	= Str3( State, OpCode );
			break;

		/* LDR(4) */
		case 0x98: case 0x99: case 0x9a: case 0x9b:
		case 0x9c: case 0x9d: case 0x9e: case 0x9f:
			Result	= Ldr4( State, OpCode );
			break;

		/* ADD(5) */
		case 0xa0: case 0xa1: case 0xa2: case 0xa3:
		case 0xa4: case 0xa5: case 0xa6: case 0xa7:
			Add5( State, OpCode );
			break;

		/* ADD(6) */
		case 0xa8: case 0xa9: case 0xaa: case 0xab:
		case 0xac: case 0xad: case 0xae: case 0xaf:
			Add6( State, OpCode );
			break;

		/* ADD(7),SUB(4) */
		case 0xb0:
			Add7Sub4( State, OpCode );
			break;

		/* <undefined> */
		case 0xb1:
			if( (int64_t)State->Core.PC != State->LastDataBkpt )
				State->Core.PC		   -=  2;

			Result	= EXEC_STATUS_ERROR_UNDEFINED_OPCODE;
			break;

		/* SXTH,SXTB,UXTH,UXTB */
		case 0xb2:
			XXTX( State, OpCode );
			break;

		/* <undefined> */
		case 0xb3:
			if( (int64_t)State->Core.PC != State->LastDataBkpt )
				State->Core.PC		   -=  2;

			Result	= EXEC_STATUS_ERROR_UNDEFINED_OPCODE;
			break;

		/* PUSH */
		case 0xb4: case 0xb5:
			Result	= Push( State, OpCode );
			break;

		/* SETEND,CPS */
		case 0xb6:
			Result	= SetEndCPS( State, OpCode );
			break;

		/* <undefined> */
		case 0xb7:
		case 0xb8: case 0xb9:
			if( (int64_t)State->Core.PC != State->LastDataBkpt )
				State->Core.PC		   -=  2;

			Result	= EXEC_STATUS_ERROR_UNDEFINED_OPCODE;
			break;

		case 0xba:
			Result	= RevXX( State, OpCode );
			break;

		case 0xbb:
			if( (int64_t)State->Core.PC != State->LastDataBkpt )
				State->Core.PC		   -=  2;

			Result	= EXEC_STATUS_ERROR_UNDEFINED_OPCODE;
			break;

		/* POP */
		case 0xbc: case 0xbd:
			Result	= Pop( State, OpCode );
			break;

		/* BKPT */
		case 0xbe:
			Result	= Bkpt( State, OpCode );
			break;

		/* <undefined> */
		case 0xbf:
			Result	= WFI( State, OpCode );
			break;

		/* STMIA */
		case 0xc0: case 0xc1: case 0xc2: case 0xc3:
		case 0xc4: case 0xc5: case 0xc6: case 0xc7:
			Result	= Stmia( State, OpCode );
			break;

		/* LDMIA */
		case 0xc8: case 0xc9: case 0xca: case 0xcb:
		case 0xcc: case 0xcd: case 0xce: case 0xcf:
			Result	= Ldmia( State, OpCode );
			break;

		/* BEQ */
		case 0xd0:
			Beq( State, OpCode );
			break;

		/* BNE */
		case 0xd1:
			Bne( State, OpCode );
			break;

		/* BCS */
		case 0xd2:
			Bcs( State, OpCode );
			break;

		/* BCC */
		case 0xd3:
			Bcc( State, OpCode );
			break;

		/* BMI */
		case 0xd4:
			Bmi( State, OpCode );
			break;

		/* BPL */
		case 0xd5:
			Bpl( State, OpCode );
			break;

		/* BVS */
		case 0xd6:
			Bvs( State, OpCode );
			break;

		/* BVC */
		case 0xd7:
			Bvc( State, OpCode );
			break;

		/* BHI */
		case 0xd8:
			Bhi( State, OpCode );
			break;

		/* BLS */
		case 0xd9:
			Bls( State, OpCode );
			break;

		/* BGE */
		case 0xda:
			Bge( State, OpCode );
			break;

		/* BLT */
		case 0xdb:
			Blt( State, OpCode );
			break;

		/* BGT */
		case 0xdc:
			Bgt( State, OpCode );
			break;

		/* BLE */
		case 0xdd:
			Ble( State, OpCode );
			break;

		/* <undefined> */
		case 0xde:
			if( (int64_t)State->Core.PC != State->LastDataBkpt )
				State->Core.PC		   -=  2;

			Result	= EXEC_STATUS_ERROR_UNDEFINED_OPCODE;
			break;

		/* SWI */
		case 0xdf:
			Result	= Swi( State, OpCode );
			break;

		/* B(2) */
		case 0xe0: case 0xe1: case 0xe2: case 0xe3:
		case 0xe4: case 0xe5: case 0xe6: case 0xe7:
			Result	= B2( State, OpCode );
			break;

		/* BLX suffix [4] */
		case 0xe8: case 0xe9: case 0xea: case 0xeb:
		case 0xec: case 0xed: case 0xee: case 0xef:
			if( (int64_t)State->Core.PC != State->LastDataBkpt )
				State->Core.PC		   -=  2;

			Result	= EXEC_STATUS_ERROR_INVALID_OPCODE;
			break;

		/* BL,BLX prefix */
		case 0xf0: case 0xf1: case 0xf2: case 0xf3:
		case 0xf4: case 0xf5: case 0xf6: case 0xf7:
			Result	= BlBlxMisc( State, OpCode );
			break;

		/* BL suffix [4] */
		case 0xf8: case 0xf9: case 0xfa: case 0xfb:
		case 0xfc: case 0xfd: case 0xfe: case 0xff:
			Result	= BlSuffix( State, OpCode );
			break;
		}

	if(( MapEntry = GetMapEntry2( State, Address )) != NULL )
		{
		if( State->Core.CPSR.EN == 0x00 )
			MapEntry->ExecutedInThread		= 1;
		else
			MapEntry->ExecutedInInterrupt	= 1;
		}
	if( Result >= EXEC_STATUS_OK )
		{
/*@@@@
		if( State->InterruptState.I != 0 && State->Core.CPSR.EN != CPU_MODE_FIQ && State->Core.CPSR.EN != CPU_MODE_IRQ && State->Core.CPSR.I == 0 )
			{
			State->Core.CPSR.I				= 0;
			State->Core.Written[CPSR]	   |= 0x00000080;
			return CallI( State, 8 );
			}
*/
		}

	return Result;
	}
/*============================================================================*/
execstatus_t ThumbLoop( thumbstate_t *State )
	{
	execstatus_t	Result;

	if( State->RunningState == RUNNINGSTATE_STEPOVER )
		asm volatile( "nop" );

	if( State->RunningState == RUNNINGSTATE_STOPPING || State->RunningState == RUNNINGSTATE_STOPPED )
		{
		if( State->Busy )
			State->Updated	= 1;
		State->Busy			= 0;
		State->RunningState	= RUNNINGSTATE_STOPPED;
		return EXEC_STATUS_STOPPED;
		}

	Result					= ThumbExec( State );
	State->LastDataBkpt		= -1;

	State->SimulatorStatus	= Result < 0 ? -Result : 0;

	if( Result < EXEC_STATUS_TO_CONTINUE )
		{
		State->RunningState	= RUNNINGSTATE_STOPPED;
		State->Busy			= 0;
		State->Updated		= 1;
		}
	else if( State->RunningState == RUNNINGSTATE_STEPINTO && Result != EXEC_STATUS_ONE_MORE )
		{
		State->RunningState	= RUNNINGSTATE_STOPPED;
		State->Busy			= 0;
		State->Updated		= 1;
		}
	else if( State->RunningState == RUNNINGSTATE_STEPOVER && State->Core.PC == State->StepOverPC && State->Core.SP == State->StepOverSP )
		{
		State->RunningState	= RUNNINGSTATE_STOPPED;
		State->Busy			= 0;
		State->Updated		= 1;
		}
	else if( CheckExecBreakpoints( State, State->Core.PC & ~(sim_address_t)1, 2 ))
		{
		if( State->SimulatorStatus == 0 )
			State->SimulatorStatus	= 2;
		State->RunningState	= RUNNINGSTATE_STOPPED;
		State->Busy			= 0;
		State->Updated		= 1;
		}

	return Result;
	}
/*============================================================================*/
static int ThumbInitialize2( thumbstate_t *State, uint32_t ROMBase, uint32_t ROMSize, uint32_t RAMBase, uint32_t RAMSize )
	{
	if( ROMSize > 0 )
		{
		if( CreateSimulatorMemoryArea( &State->MemoryAreas[0], ROMBase, ROMSize, 1, 1, 1, NULL, NULL, NULL, NULL ) < 0 )
			return -1;

		State->MemoryAreas[0]->BaseAddress	= ROMBase;
		State->MemoryAreas[0]->LastAddress	= ROMBase + ROMSize - 1;
		State->Core.PC						= ROMBase;
		}
	else
		{
		State->MemoryAreas[0]->BaseAddress	= 1;
		State->MemoryAreas[0]->LastAddress	= 0;
		State->Core.PC						= RAMBase;
		}

	if( CreateSimulatorMemoryArea( &State->MemoryAreas[1], RAMBase, RAMSize, 1, 0, 1, NULL, NULL, NULL, NULL ) < 0 )
		return -1;

	State->MemoryAreas[1]->BaseAddress	= RAMBase;
	State->MemoryAreas[1]->LastAddress	= RAMBase + RAMSize - 1;

	if( CreateSimulatorMemoryArea( &State->MemoryAreas[2], 0x41005000, 32, 1, 0, 1, NULL, NULL, NULL, NULL ) < 0 )
		return -1;

	State->MemoryAreas[2]->BaseAddress	= 0x41005000;
	State->MemoryAreas[2]->LastAddress	= 0x41005000 + 32 - 1;

	State->NumMemoryAreas	= 3;

	return 0;
	}
/*============================================================================*/
int ThumbInitialize( thumbstate_t *State, uint32_t ROMBase, uint32_t ROMSize, uint32_t RAMBase, uint32_t RAMSize )
	{
	if( State == NULL || RAMSize < 1024 )
		return -1;

	memset( State, 0x00, sizeof *State );

	if( ThumbInitialize2( State, ROMBase, ROMSize, RAMBase, RAMSize ) < 0 )
		{
		ThumbFinalize( State );
		return -1;
		}

	return 1;
	}
/*============================================================================*/
int ThumbFinalize( thumbstate_t *State )
	{
	int	i;

	if( State == NULL )
		return -1;

	for( i = 0; i < State->NumMemoryAreas; i++ )
		if( State->MemoryAreas[i] != NULL )
			{
			free( State->MemoryAreas[i] );
			State->MemoryAreas[i]	= NULL;
			}

	State->NumMemoryAreas	= 0;

	return 0;
	}
/*============================================================================*/
static execstatus_t StartState( thumbstate_t *State )
	{
	size_t	i, j;

	if( State == NULL )
		return EXEC_STATUS_ERROR_UNINITIALIZED;

	for( i = 0; i < State->NumMemoryAreas; i++ )
		if( State->MemoryAreas[i] != NULL && State->MemoryAreas[i]->Map != NULL )
			for( j = 0; j < State->MemoryAreas[i]->Size; j++ )
				State->MemoryAreas[i]->Map[j].AccessInfo	= 0;

	memset( State->Core.Written, 0x00, sizeof State->Core.Written );
	State->Core.CPSRWrittenByApp			= 0;
	memcpy( &State->PreviousCore, &State->Core, sizeof State->PreviousCore );
	State->LastDataBkpt	= (uint64_t)State->Core.PC + 2;

	return EXEC_STATUS_OK;
	}
/*============================================================================*/
execstatus_t ThumbReset( thumbstate_t *State, int Full )
	{
	sim_address_t	Base, Limit, SPValue, PCValue;
	execstatus_t	Result;
	unsigned		i;

	if( State == NULL )
		return EXEC_STATUS_ERROR_UNINITIALIZED;

	Base		= State->MemoryAreas[0] != NULL ? State->MemoryAreas[0]->BaseAddress : State->MemoryAreas[1]->BaseAddress;
	Limit		= State->MemoryAreas[0] != NULL ? State->MemoryAreas[0]->LastAddress : State->MemoryAreas[1]->LastAddress;

	if(( Result = LoadWord( State, Base + 0, &SPValue )) < EXEC_STATUS_OK )
		return Result;

	if(( Result = LoadWord( State, Base + 4, &PCValue )) < EXEC_STATUS_OK )
		return Result;

	memset( State->Core.Regs, 0x00, sizeof State->Core.Regs );
	for( i = 0; i < State->NumMemoryAreas; i++ )
		{
		if( Full && State->MemoryAreas[i] != NULL && State->MemoryAreas[i]->Pristine != NULL )
			memcpy( State->MemoryAreas[i]->Buffer, State->MemoryAreas[i]->Pristine, State->MemoryAreas[i]->Size );
		if( State->MemoryAreas[i] != NULL && State->MemoryAreas[i]->Previous != NULL )
			memcpy( State->MemoryAreas[i]->Previous, State->MemoryAreas[i]->Buffer, State->MemoryAreas[i]->Size );
		}

	if(( SPValue & 3 ) == 0 && SPValue >= State->MemoryAreas[1]->BaseAddress && SPValue <= State->MemoryAreas[1]->LastAddress && ( PCValue & 1 ) == 1 && PCValue >= Base && PCValue <= Limit )
		{
		State->Core.SP			= State->Core.MSP = SPValue;
		State->Core.PC			= PCValue;
		}
	else
		{
		State->Core.SP			= State->Core.MSP = ( State->MemoryAreas[1]->LastAddress + 1 ) & ~(sim_address_t)3;
		State->Core.PC			= Base | 1;
		}

	State->Core.LR				= (uint32_t)-1;
	State->Core.PSP				= 0x00000000;
	State->Core.CONTROL.Value	= 0x00000000;
	State->Core.PRIMASK.Value	= 0x00000000;
	State->Core.CPSR.Value		= 0x01000000;
	State->InterruptState.Value	= 0x00000000;
	State->RunningState			= RUNNINGSTATE_STOPPED;
	State->StepOverSP			= 0;
	State->StepOverPC			= 0;
	State->SimulatorStatus		= 0;
	State->Busy					= 0;

	StartState( State );

	return EXEC_STATUS_OK;
	}
/*============================================================================*/
execstatus_t ThumbClearTrace( thumbstate_t *State )
	{
	sim_address_t	i, j;

	if( State == NULL )
		return EXEC_STATUS_ERROR_UNINITIALIZED;

	for( i = 0; i < State->NumMemoryAreas; i++ )
		if( State->MemoryAreas[i] != NULL && State->MemoryAreas[i]->Map != NULL )
			for( j = 0; j < State->MemoryAreas[i]->Size; j++ )
				State->MemoryAreas[i]->Map[j].TraceInfo	= 0;

	return EXEC_STATUS_OK;
	}
/*============================================================================*/
execstatus_t ThumbClearAllBreakpoints( thumbstate_t *State )
	{
	sim_address_t	i, j;

	if( State == NULL )
		return EXEC_STATUS_ERROR_UNINITIALIZED;

	for( i = 0; i < State->NumMemoryAreas; i++ )
		if( State->MemoryAreas[i] != NULL && State->MemoryAreas[i]->Map != NULL )
			for( j = 0; j < State->MemoryAreas[i]->Size; j++ )
				State->MemoryAreas[i]->Map[j].BreakpointInfo	= 0;

	return EXEC_STATUS_OK;
	}
/*============================================================================*/
execstatus_t ThumbToggleBreakpoint( thumbstate_t *State )
	{
	sim_mapentry_t	*MapEntry;

	if( State == NULL )
		return EXEC_STATUS_ERROR_UNINITIALIZED;

	if(( MapEntry = GetMapEntry2( State, State->Core.PC & ~(sim_address_t)1 )) == NULL )
		return EXEC_STATUS_ERROR_UNINITIALIZED;

	MapEntry->BreakpointExecute	   ^= 1;

	return EXEC_STATUS_OK;
	}
/*============================================================================*/
execstatus_t ThumbRun( thumbstate_t *State )
	{
	if( State->Busy	)
		return EXEC_STATUS_STOPPED;

	State->RunningState	= RUNNINGSTATE_RUNNING;
	State->Busy			= 1;

	StartState( State );

	return EXEC_STATUS_OK;
	}
/*============================================================================*/
execstatus_t ThumbPause( thumbstate_t *State )
	{
	State->RunningState	= RUNNINGSTATE_STOPPING;

	return EXEC_STATUS_OK;
	}
/*============================================================================*/
execstatus_t ThumbStepInto( thumbstate_t *State )
	{
	if( State->Busy	)
		return EXEC_STATUS_STOPPED;

	State->RunningState	= RUNNINGSTATE_STEPINTO;
	State->Busy			= 1;

	StartState( State );

	return EXEC_STATUS_OK;
	}
/*============================================================================*/
execstatus_t ThumbStepOver( thumbstate_t *State )
	{
	uint16_t		OpCode;
	execstatus_t	Result;
	sim_address_t	ReturnAddress;

	if( State->Busy	)
		return EXEC_STATUS_STOPPED;

	/* Read one instruction OpCode. If any error happened... */
	if(( Result = FetchOpCode( State, State->Core.PC & ~(sim_address_t)1, &OpCode )) < EXEC_STATUS_OK )
		/* ...return an error status. */
		return Result;

	ReturnAddress	= State->Core.PC + 2;

	/* The OpCode is the first half of a BL/BLX. Read the second OpCode. If any error happened... */
	if(( OpCode & 0xf800 ) == 0xf000 )
		{
		if(( Result = FetchOpCode( State, ( State->Core.PC & ~(sim_address_t)1 ) + 2, &OpCode )) < EXEC_STATUS_OK )
			/* ...return an error status. */
			return Result;
		ReturnAddress	= State->Core.PC + 4;
		}

	/* The OpCode is a "BX Rn" or the second half of a BL/BLX... */
	if(( OpCode &  0xff87 ) == 0x4780 || ( OpCode & 0xe800 ) == 0xe800 )
		{
		/* ...so we can do a "Step Over". */
		State->RunningState	= RUNNINGSTATE_STEPOVER;
		State->StepOverSP	= State->Core.SP;
		State->StepOverPC	= ReturnAddress;
		}
	/* The OpCode is any other instruction... */
	else
		/* ...we can only do a "Step Into". */
		State->RunningState	= RUNNINGSTATE_STEPINTO;

	State->Busy			= 1;

	StartState( State );

	return EXEC_STATUS_OK;
	}
/*============================================================================*/
execstatus_t ThumbIRQ( thumbstate_t *State )
	{
	if( State == NULL )
		return EXEC_STATUS_ERROR_UNINITIALIZED;

	State->InterruptState.I	= 1;

	return EXEC_STATUS_OK;
	}
/*============================================================================*/
int ThumbIsBusy( thumbstate_t *State )
	{
	return State->Busy;
	}
/*============================================================================*/
int ThumbSetMemoryWindow( thumbstate_t *State, sim_address_t startaddress, sim_address_t endaddress )
	{
	if( State == NULL )
		return EXEC_STATUS_ERROR_UNINITIALIZED;

	State->MemMonitorStart	= startaddress;
	State->MemMonitorEnd	= endaddress;

	return EXEC_STATUS_OK;
	}
/*============================================================================*/
int ThumbGetSimulatorUpdated( thumbstate_t *State )
	{
	return State->Updated;
	}
/*============================================================================*/
int ThumbGetSimulatorStatus( thumbstate_t *State )
	{
	return State->SimulatorStatus;
	}
/*============================================================================*/
void ThumbClearSimulatorUpdated( thumbstate_t *State )
	{
	State->Updated			= 0;
	}
/*============================================================================*/
void ThumbClearSimulatorStatus( thumbstate_t *State )
	{
	State->SimulatorStatus	= 0;
	}
/*============================================================================*/
int ThumbGetDataBreakpoint( thumbstate_t *State, sim_address_t address )
	{
	int	i;

	if( State == NULL )
		return -1;

	for( i = 0; i < State->NumMemoryAreas; i++ )
		if( State->MemoryAreas[i] != NULL && State->MemoryAreas[i]->Map != NULL && address >= State->MemoryAreas[i]->BaseAddress && address <= State->MemoryAreas[i]->LastAddress )
			return State->MemoryAreas[i]->Map[address - State->MemoryAreas[i]->BaseAddress].DataBreakpoint;

	return -1;
	}
/*============================================================================*/
int ThumbSetDataBreakpoint( thumbstate_t *State, sim_address_t address, int value )
	{
	int	i;

	if( State == NULL )
		return -1;

	for( i = 0; i < State->NumMemoryAreas; i++ )
		if( State->MemoryAreas[i] != NULL && State->MemoryAreas[i]->Map != NULL && address >= State->MemoryAreas[i]->BaseAddress && address <= State->MemoryAreas[i]->LastAddress )
			{
			State->MemoryAreas[i]->Map[address - State->MemoryAreas[i]->BaseAddress].DataBreakpoint	= value;
			return 1;
			}

	return -1;
	}
/*============================================================================*/
int ThumbGetExecBreakpoint( thumbstate_t *State, sim_address_t address )
	{
	int	i;

	if( State == NULL )
		return -1;

	for( i = 0; i < State->NumMemoryAreas; i++ )
		if( State->MemoryAreas[i] != NULL && State->MemoryAreas[i]->Map != NULL && address >= State->MemoryAreas[i]->BaseAddress && address <= State->MemoryAreas[i]->LastAddress )
			return State->MemoryAreas[i]->Map[address - State->MemoryAreas[i]->BaseAddress].ExecBreakpoint;

	return -1;
	}
/*============================================================================*/
int ThumbSetExecBreakpoint( thumbstate_t *State, sim_address_t address, int value )
	{
	int	i;

	if( State == NULL )
		return -1;

	for( i = 0; i < State->NumMemoryAreas; i++ )
		if( State->MemoryAreas[i] != NULL && State->MemoryAreas[i]->Map != NULL && address >= State->MemoryAreas[i]->BaseAddress && address <= State->MemoryAreas[i]->LastAddress )
			{
			State->MemoryAreas[i]->Map[address - State->MemoryAreas[i]->BaseAddress].ExecBreakpoint	= value;
			return 1;
			}

	return -1;
	}
/*============================================================================*/
int ThumbLoadByte( thumbstate_t *State, sim_address_t address, uint8_t *value )
	{
	int	i;

	if( State == NULL || ( address & 0 ) != 0 )
		return -1;

	for( i = 0; i < State->NumMemoryAreas; i++ )
		if( State->MemoryAreas[i] != NULL && address >= State->MemoryAreas[i]->BaseAddress && address <= State->MemoryAreas[i]->LastAddress )
			{
			if( State->MemoryAreas[i]->ReadFunc != NULL )
				return State->MemoryAreas[i]->ReadFunc( State->MemoryAreas[i], address, sizeof *value, &value );

			*value	= ( (uint8_t)State->MemoryAreas[i]->Buffer[address - State->MemoryAreas[i]->BaseAddress + 0] <<  0 );
			return 1;
			}

	return -1;
	}
/*============================================================================*/
int ThumbLoadHalf( thumbstate_t *State, sim_address_t address, uint16_t *value )
	{
	int	i;

	if( State == NULL || ( address & 1 ) != 0 )
		return -1;

	for( i = 0; i < State->NumMemoryAreas; i++ )
		if( State->MemoryAreas[i] != NULL && address >= State->MemoryAreas[i]->BaseAddress && address <= State->MemoryAreas[i]->LastAddress - 1 )
			{
			if( State->MemoryAreas[i]->ReadFunc != NULL )
				return State->MemoryAreas[i]->ReadFunc( State->MemoryAreas[i], address, sizeof *value, &value );

			*value	= ( (uint16_t)State->MemoryAreas[i]->Buffer[address - State->MemoryAreas[i]->BaseAddress + 1] <<  8 ) |
					  ( (uint16_t)State->MemoryAreas[i]->Buffer[address - State->MemoryAreas[i]->BaseAddress + 0] <<  0 );
			return 1;
			}

	return -1;
	}
/*============================================================================*/
int ThumbLoadWord( thumbstate_t *State, sim_address_t address, uint32_t *value )
	{
	int	i;

	if( State == NULL || ( address & 3 ) != 0 )
		return -1;

	for( i = 0; i < State->NumMemoryAreas; i++ )
		if( State->MemoryAreas[i] != NULL && address >= State->MemoryAreas[i]->BaseAddress && address <= State->MemoryAreas[i]->LastAddress - 1 )
			{
			if( State->MemoryAreas[i]->ReadFunc != NULL )
				return State->MemoryAreas[i]->ReadFunc( State->MemoryAreas[i], address, sizeof *value, &value );

			*value	= ( (uint32_t)State->MemoryAreas[i]->Buffer[address - State->MemoryAreas[i]->BaseAddress + 3] << 24 ) |
					  ( (uint32_t)State->MemoryAreas[i]->Buffer[address - State->MemoryAreas[i]->BaseAddress + 2] << 16 ) |
					  ( (uint32_t)State->MemoryAreas[i]->Buffer[address - State->MemoryAreas[i]->BaseAddress + 1] <<  8 ) |
					  ( (uint32_t)State->MemoryAreas[i]->Buffer[address - State->MemoryAreas[i]->BaseAddress + 0] <<  0 );
			return 1;
			}

	return -1;
	}
/*============================================================================*/
int ThumbLoadMemoryArray( thumbstate_t *State, sim_address_t address, sim_address_t size, uint8_t *buffer )
	{
	int	i;

	if( State == NULL || size == 0 )
		return -1;

	for( i = 0; i < State->NumMemoryAreas; i++ )
		if( State->MemoryAreas[i] != NULL && address >= State->MemoryAreas[i]->BaseAddress && address <= State->MemoryAreas[i]->LastAddress )
			{
			if( address + size - 1 > State->MemoryAreas[i]->LastAddress || address + size < address )
				return -1;

			memcpy( buffer, &State->MemoryAreas[i]->Buffer[address - State->MemoryAreas[i]->BaseAddress + 0], size );

			return 1;
			}

	return -1;
	}
/*============================================================================*/
int ThumbLoadMapSegment( thumbstate_t *State, sim_address_t address, sim_address_t length, sim_mapentry_t *buffer )
	{
	int	i;

	if( State == NULL || length == 0 )
		return -1;

	for( i = 0; i < State->NumMemoryAreas; i++ )
		if( State->MemoryAreas[i] != NULL && address >= State->MemoryAreas[i]->BaseAddress && address <= State->MemoryAreas[i]->LastAddress )
			{
			if( address + length - 1 > State->MemoryAreas[i]->LastAddress || address + length < address )
				return -1;

			memcpy( buffer, &State->MemoryAreas[i]->Map[address - State->MemoryAreas[i]->BaseAddress], length * sizeof( sim_mapentry_t ));

			return 1;
			}

	return -1;
	}
/*============================================================================*/
int ThumbStoreByte( thumbstate_t *State, sim_address_t address, uint8_t value )
	{
	int	i;

	if( State == NULL || ( address & 0 ) != 0 )
		return -1;

	for( i = 0; i < State->NumMemoryAreas; i++ )
		if( State->MemoryAreas[i] != NULL && address >= State->MemoryAreas[i]->BaseAddress && address <= State->MemoryAreas[i]->LastAddress )
			{
			if( State->MemoryAreas[i]->WriteFunc != NULL )
				return State->MemoryAreas[i]->WriteFunc( State->MemoryAreas[i], address, sizeof value, &value );

			sim_address_t	Index	= address - State->MemoryAreas[i]->BaseAddress;

			State->MemoryAreas[i]->Buffer[Index + 0]			= (uint8_t)( value >>  0 );

			if( State->MemoryAreas[i]->Map != NULL )
				State->MemoryAreas[i]->Map[Index + 0].Written	= 3;

			return 1;
			}

	return -1;
	}
/*============================================================================*/
int ThumbStoreHalf( thumbstate_t *State, sim_address_t address, uint16_t value )
	{
	int	i;

	if( State == NULL || ( address & 1 ) != 0 )
		return -1;

	for( i = 0; i < State->NumMemoryAreas; i++ )
		if( State->MemoryAreas[i] != NULL && address >= State->MemoryAreas[i]->BaseAddress && address <= State->MemoryAreas[i]->LastAddress )
			{
			if( State->MemoryAreas[i]->WriteFunc != NULL )
				return State->MemoryAreas[i]->WriteFunc( State->MemoryAreas[i], address, sizeof value, &value );

			sim_address_t	Index	= address - State->MemoryAreas[i]->BaseAddress;

			State->MemoryAreas[i]->Buffer[Index + 0]			= (uint8_t)( value >>  0 );
			State->MemoryAreas[i]->Buffer[Index + 1]			= (uint8_t)( value >>  8 );

			if( State->MemoryAreas[i]->Map != NULL )
				{
				State->MemoryAreas[i]->Map[Index + 0].Written	= 3;
				State->MemoryAreas[i]->Map[Index + 1].Written	= 3;
				}

			return 1;
			}

	return -1;
	}
/*============================================================================*/
int ThumbStoreWord( thumbstate_t *State, sim_address_t address, uint32_t value )
	{
	int	i;

	if( State == NULL || ( address & 3 ) != 0 )
		return -1;

	for( i = 0; i < State->NumMemoryAreas; i++ )
		if( State->MemoryAreas[i] != NULL && address >= State->MemoryAreas[i]->BaseAddress && address <= State->MemoryAreas[i]->LastAddress )
			{
			if( State->MemoryAreas[i]->WriteFunc != NULL )
				return State->MemoryAreas[i]->WriteFunc( State->MemoryAreas[i], address, sizeof value, &value );

			sim_address_t	Index	= address - State->MemoryAreas[i]->BaseAddress;

			State->MemoryAreas[i]->Buffer[Index + 0]			= (uint8_t)( value >>  0 );
			State->MemoryAreas[i]->Buffer[Index + 1]			= (uint8_t)( value >>  8 );
			State->MemoryAreas[i]->Buffer[Index + 2]			= (uint8_t)( value >> 16 );
			State->MemoryAreas[i]->Buffer[Index + 3]			= (uint8_t)( value >> 24 );

			if( State->MemoryAreas[i]->Map != NULL )
				{
				State->MemoryAreas[i]->Map[Index + 0].Written	= 3;
				State->MemoryAreas[i]->Map[Index + 1].Written	= 3;
				State->MemoryAreas[i]->Map[Index + 2].Written	= 3;
				State->MemoryAreas[i]->Map[Index + 3].Written	= 3;
				}

			return 1;
			}

	return -1;
	}
/*============================================================================*/
int ThumbStoreMemoryArray( thumbstate_t *State, sim_address_t address, sim_address_t size, const uint8_t *buffer )
	{
	int	i;

	if( State == NULL || size == 0 )
		return -1;

	for( i = 0; i < State->NumMemoryAreas; i++ )
		if( State->MemoryAreas[i] != NULL && address >= State->MemoryAreas[i]->BaseAddress && address <= State->MemoryAreas[i]->LastAddress )
			{
			if( address + size - 1 > State->MemoryAreas[i]->LastAddress || address + size < address )
				return -1;

			memcpy( &State->MemoryAreas[i]->Buffer[address - State->MemoryAreas[i]->BaseAddress + 0], buffer, size );

			return 1;
			}

	return -1;
	}
/*============================================================================*/
#define	LENGTH(a) ( sizeof (a) / sizeof (a)[0] )
/*============================================================================*/
int ThumbGetCPSRValue( thumbstate_t *State, uint32_t *value )
	{
	if( State == NULL )
		return -1;

	*value	= State->Core.CPSR.Value;

	return 1;
	}
/*============================================================================*/
int ThumbSetCPSRValue( thumbstate_t *State, uint32_t value )
	{
	if( State == NULL )
		return -1;

	if( value != State->Core.CPSR.Value )
		{
		State->Core.CPSRWrittenByApp   |= State->Core.CPSR.Value ^ value;
		State->Core.CPSR.Value			= value;
		}

	return 1;
	}
/*============================================================================*/
int ThumbGetRegisterValue( thumbstate_t *State, unsigned registernumber, uint32_t *value )
	{
	if( State == NULL || registernumber >= LENGTH( State->Core.Regs ))
		return -1;

	if( registernumber == MSP && State->Core.CONTROL.ASPSEL == 0 )
		State->Core.MSP	= State->Core.SP;
	else if( registernumber == PSP && State->Core.CONTROL.ASPSEL != 0 )
		State->Core.PSP	= State->Core.SP;

	*value	= State->Core.Regs[registernumber];

	return 1;
	}
/*============================================================================*/
int ThumbSetRegisterValue( thumbstate_t *State, unsigned registernumber, uint32_t value )
	{
	if( State == NULL || registernumber >= LENGTH( State->Core.Regs ))
		return -1;

	if(( registernumber == MSP && State->Core.CONTROL.ASPSEL == 0 ) || ( registernumber == PSP && State->Core.CONTROL.ASPSEL != 0 ))
		State->Core.SP	= value;
	else if( registernumber == SP )
		State->Core.CONTROL.ASPSEL == 0 ? ( State->Core.MSP = value ) : ( State->Core.PSP = value );

	if( value != State->Core.Regs[registernumber] )
		{
		State->Core.Regs[registernumber]	= value;
		State->Core.Written[registernumber]	= 3;
		}

	return 1;
	}
/*============================================================================*/
int ThumbGetAllRegistersValues( thumbstate_t *State, uint32_t *Registers )
	{
	if( State == NULL || Registers == NULL )
		return -1;

	memcpy( Registers, State->Core.Regs, 16 * sizeof *Registers );

	return 1;
	}
/*============================================================================*/
int ThumbGetCore( thumbstate_t *State, cpucore_t *Core, cpucore_t *PreviousCore )
	{
	if( State == NULL || Core == NULL || PreviousCore == NULL )
		return -1;

	memcpy( Core, &State->Core, sizeof *Core );
	memcpy( PreviousCore, &State->PreviousCore, sizeof *PreviousCore );

	return 1;
	}
/*============================================================================*/
int ThumbPeriphRead( sim_memoryarea_t *Area, sim_address_t Address, sim_address_t Size, void *Buffer )
	{
	sim_address_t	Index;
	uint32_t		Value;

	if( Size != 4 || ( Address & 3 ) != 0 )
		return EXEC_STATUS_ERROR_UNALIGNED_ACCESS;

	if( Address + Size - 1 > Area->LastAddress )
		return EXEC_STATUS_ERROR_UNDEFINED_ADDRESS;

	Index	= Address - Area->BaseAddress;
	Value	= *(uint32_t*)&Area->Buffer[Index];

	if( Index == 8 )
		Area->Buffer[Index]	&= ~0x10;

	*(uint32_t*)Buffer	= Value;

	return EXEC_STATUS_OK;
	}
/*============================================================================*/
int ThumbPeriphWrite( sim_memoryarea_t *Area, sim_address_t Address, sim_address_t Size, const void *Buffer )
	{
	uint32_t	Value;

	if( Size != 4 || ( Address & 3 ) != 0 )
		return EXEC_STATUS_ERROR_UNALIGNED_ACCESS;

	if( Address + Size - 1 > Area->LastAddress )
		return EXEC_STATUS_ERROR_UNDEFINED_ADDRESS;

	Value	= *(uint32_t*)Buffer;

	switch( Address )
		{
		case 0:
			break;
		case 4:
			break;
		case 8:
			break;
		case 12:
			break;
		}

	return EXEC_STATUS_OK;
	}
/*============================================================================*/
