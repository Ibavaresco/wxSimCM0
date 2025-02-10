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
#if			!defined __SIMULATOR_H__
#define __SIMULATOR_H__
/*============================================================================*/
#include <stdint.h>
#include "ELF.h"
/*============================================================================*/
#if			defined __cplusplus
extern "C"
	{
#endif	/*	defined __cplusplus */
/*============================================================================*/
typedef	uint32_t			sim_address_t;
typedef uint16_t			sim_nameoffset_t;
/*============================================================================*/
typedef struct
	{
	union
		{
		struct
			{
			uint16_t		Type				: 2;
			uint16_t		ExecutedInThread	: 1;
			uint16_t		ExecutedInInterrupt	: 1;
			uint16_t		BreakpointExecute	: 1;
			uint16_t		BreakpointRead		: 1;
			uint16_t		BreakpointWrite		: 1;
			uint16_t		Read				: 1;
			uint16_t		Written				: 2;
			uint16_t		Changed				: 1;
			};
		struct
			{
			uint16_t							: 2;
			uint16_t		TraceInfo			: 2;
			uint16_t		BreakpointInfo		: 3;
			uint16_t		AccessInfo			: 4;
			};
		struct
			{
			uint16_t							: 4;
			uint16_t		ExecBreakpoint		: 1;
			uint16_t		DataBreakpoint		: 2;
			};
		struct
			{
			uint16_t							: 2;
			uint16_t		TransientInfo		: 9;
			};
		uint16_t			Value;
		};
	sim_nameoffset_t		NameOffset;
	} sim_mapentry_t;
/*============================================================================*/
typedef struct
	{
	uint32_t				ReadOnly;
	uint32_t				Viewable;
	uint32_t				Allocate;
	} sim_memoryareaflags_t;
/*============================================================================*/
typedef struct sim_memoryarea_tag
	{
	sim_address_t			BaseAddress;
	sim_address_t			LastAddress;
	sim_address_t			Size;
	sim_memoryareaflags_t	Flags;
	void					*Cookie;
	int						(*ReadFunc)( struct sim_memoryarea_tag *Area, sim_address_t Address, sim_address_t Size, void *Buffer );
	int						(*WriteFunc)( struct sim_memoryarea_tag *Area, sim_address_t Address, sim_address_t Size, const void *Buffer );
	uint8_t					*Pristine;
	uint8_t					*Previous;
	sim_mapentry_t			*Map;
	char					*SymbolsNames;
	uint8_t					Buffer[];
	} sim_memoryarea_t;
/*============================================================================*/
typedef struct
	{
	unsigned				NumEntries;
	char					*NamesTable;
	elf_symboltableentry_t	Symbols[];
	} symboltable_t;
/*============================================================================*/
int		CreateSimulatorMemoryArea	( sim_memoryarea_t **Area, sim_address_t BaseAddress,
									  sim_address_t Size, int Allocate, int ReadOnly, int Viewable,
									  void *Cookie,
									  int (*ReadFunc)(sim_memoryarea_t*,sim_address_t,sim_address_t,void*),
									  int (*WriteFunc)(sim_memoryarea_t*,sim_address_t,sim_address_t,const void*),
									  symboltable_t *SymbolTable );

int		DeleteSimulatorMemoryArea	( sim_memoryarea_t **Area );
/*============================================================================*/
#if			defined __cplusplus
	}
#endif	/*	defined __cplusplus */
/*============================================================================*/
#endif	/*	!defined __SIMULATOR_H__ */
/*============================================================================*/
