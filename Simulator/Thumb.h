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
#if			!defined __THUMB_H__
#define __THUMB_H__
/*============================================================================*/
#include <stdint.h>
#include <stdio.h>
#include "BitVector.h"
#include "Simulator.h"
/*============================================================================*/
#if			defined __cplusplus
extern "C"
	{
#endif	/*	defined __cplusplus */
/*============================================================================*/
typedef enum
	{
	R0,
	R1,
	R2,
	R3,
	R4,
	R5,
	R6,
	R7,
	R8,
	R9,
	R10,
	R11,
	R12,
	R13, SP = R13,
	R14, LR = R14,
	R15, PC = R15,
	MSP,
	PSP,
	CPSR,
	PRIMASK,
	CONTROL
	} thumbregisters_t;
/*============================================================================*/
typedef enum
	{
	EXEC_STATUS_ERROR_UNINITIALIZED			= -12,
	EXEC_STATUS_ERROR_INTERNAL_ERROR		= -11,
	EXEC_STATUS_ERROR_INVALID_MODE			= -10,
	EXEC_STATUS_ERROR_UNDEFINED_OPCODE		=  -9,
	EXEC_STATUS_ERROR_INVALID_OPCODE		=  -8,
	EXEC_STATUS_ERROR_READONLY_ADDRESS		=  -7,
	EXEC_STATUS_ERROR_UNDEFINED_ADDRESS		=  -6,
	EXEC_STATUS_ERROR_UNALIGNED_ACCESS		=  -5,
	EXEC_STATUS_BREAKPOINT_WRITE			=  -4,
	EXEC_STATUS_BREAKPOINT_READ				=  -3,
	EXEC_STATUS_BREAKPOINT_EXEC				=  -2,
	EXEC_STATUS_HALTED						=  -1,

	EXEC_STATUS_STOPPED,

	EXEC_STATUS_TO_CONTINUE, EXEC_STATUS_OK = EXEC_STATUS_TO_CONTINUE,

	EXEC_STATUS_ONE_MORE
	} execstatus_t;
/*============================================================================*/
typedef enum
	{
	RUNNINGSTATE_STOPPING	= -1,
	RUNNINGSTATE_STOPPED,
	RUNNINGSTATE_STEPINTO,
	RUNNINGSTATE_ANIMATING,
	RUNNINGSTATE_RUNNING,
	RUNNINGSTATE_RUNNINGFAST,
	RUNNINGSTATE_STEPOVER,
	RUNNINGSTATE_PROFILING
	} runningstate_t;
/*============================================================================*/
typedef enum
	{
	CPU_MODE_USER				= 0x10,
	CPU_MODE_FIQ				= 0x11,
	CPU_MODE_IRQ				= 0x12,
	CPU_MODE_SVC				= 0x13,
	CPU_MODE_ABT				= 0x17,
	CPU_MODE_UND				= 0x1b,
	CPU_MODE_SYS				= 0x1f
	} cpumodes_t;
/*============================================================================*/
typedef union
	{
	struct
		{
		uint32_t	EN		:  6;
		uint32_t			: 18;
		uint32_t	T		:  1;
		uint32_t			:  3;
		uint32_t	V		:  1;
		uint32_t	C		:  1;
		uint32_t	Z		:  1;
		uint32_t	N		:  1;
		};
	uint32_t		Value;
	} cpuflags_t;
/*==============================================================================*/
typedef union
	{
	struct
		{
		uint32_t	PRIMASK	:  1;
		uint32_t			: 31;
		};
	uint32_t		Value;
	} cpuprimask_t;
/*==============================================================================*/
typedef union
	{
	struct
		{
		uint32_t			:  1;
		uint32_t	ASPSEL	:  1;
		uint32_t			: 30;
		};
	uint32_t		Value;
	} cpucontrol_t;
/*==============================================================================*/
typedef struct
	{
	union
		{
		struct
			{
			uint32_t		R0;
			uint32_t		R1;
			uint32_t		R2;
			uint32_t		R3;
			uint32_t		R4;
			uint32_t		R5;
			uint32_t		R6;
			uint32_t		R7;
			uint32_t		R8;
			uint32_t		R9;
			uint32_t		R10;
			uint32_t		R11;
			uint32_t		R12;
			uint32_t		R13;
			uint32_t		R14;
			uint32_t		R15;
			uint32_t		MSP;
			uint32_t		PSP;
			cpuflags_t		CPSR;
			cpuprimask_t	PRIMASK;
			cpucontrol_t	CONTROL;
			};
		struct
			{
			uint32_t		_[10];
			uint32_t		SL;
			uint32_t		FP;
			uint32_t		IP;
			uint32_t		SP;
			uint32_t		LR;
			uint32_t		PC;
			};
		uint32_t			Regs[21];
		};
	uint32_t				Written[21];
	uint32_t				CPSRWrittenByApp;
	} cpucore_t;
/*==============================================================================*/
typedef union
	{
	struct
		{
		uint32_t	I	:  1;
		uint32_t		: 31;
		};
	uint32_t		Value;
	} interruptstate_t;
/*==============================================================================*/
typedef struct
	{
	cpucore_t			Core;

	cpucore_t			PreviousCore;

	interruptstate_t	InterruptState;
	runningstate_t		RunningState;
	int					Updated;
	int					Busy;
	int64_t				LastDataBkpt;
	sim_address_t		StepOverSP;
	sim_address_t		StepOverPC;
	unsigned			SimulatorStatus;
	sim_address_t		MemMonitorStart;
	sim_address_t		MemMonitorEnd;

	unsigned			NumMemoryAreas;
	sim_memoryarea_t	*MemoryAreas[4];
	} thumbstate_t;
/*============================================================================*/
sim_mapentry_t	GetMapEntry					( thumbstate_t *State, sim_address_t Address );
sim_mapentry_t	*GetMapEntry2				( const thumbstate_t *State, sim_address_t Address );
const char 		*GetSymbolName				( thumbstate_t *State, sim_address_t Address );
/*============================================================================*/
int				ThumbInitialize				( thumbstate_t *State, uint32_t ROMBase, uint32_t ROMSize, uint32_t RAMBase, uint32_t RAMSize );
int				ThumbFinalize				( thumbstate_t *State );

execstatus_t	ThumbReset					( thumbstate_t *State, int Full );
execstatus_t	ThumbClearTrace				( thumbstate_t *State );
execstatus_t	ThumbClearAllBreakpoints	( thumbstate_t *State );
execstatus_t	ThumbToggleBreakpoint		( thumbstate_t *State );
execstatus_t	ThumbRun					( thumbstate_t *State );
execstatus_t	ThumbPause					( thumbstate_t *State );
execstatus_t	ThumbStepInto				( thumbstate_t *State );
execstatus_t	ThumbStepOver				( thumbstate_t *State );


int				CheckExecBreakpoints		( const thumbstate_t *State, sim_address_t Address, unsigned Length );
int				CheckReadBreakpoints		( const thumbstate_t *State, sim_address_t Address, unsigned Length );
int				CheckWriteBreakpoints		( const thumbstate_t *State, sim_address_t Address, unsigned Length );

execstatus_t	GetBaseAddress				( const thumbstate_t *State, sim_address_t Address, sim_address_t *BaseAddress );
execstatus_t	GetLastAddress				( const thumbstate_t *State, sim_address_t Address, sim_address_t *LastAddress );

execstatus_t	LoadByte					( const thumbstate_t *State, uint32_t Address, uint8_t *Value );
execstatus_t	LoadHalfWord				( const thumbstate_t *State, uint32_t Address, uint16_t *Value );
execstatus_t	LoadWord					( const thumbstate_t *State, uint32_t Address, uint32_t *Value );

execstatus_t	ThumbLoop					( thumbstate_t *State );
execstatus_t	ThumbIRQ					( thumbstate_t *State );
/*============================================================================*/
int				ThumbSetMemoryWindow		( thumbstate_t *State, sim_address_t startaddress, sim_address_t endaddress );
int				ThumbIsBusy					( thumbstate_t *State );
int				ThumbGetSimulatorUpdated	( thumbstate_t *State );
int				ThumbGetSimulatorStatus		( thumbstate_t *State );
void			ThumbClearSimulatorUpdated	( thumbstate_t *State );
void			ThumbClearSimulatorStatus	( thumbstate_t *State );
int				ThumbGetDataBreakpoint		( thumbstate_t *State, sim_address_t address );
int				ThumbSetDataBreakpoint		( thumbstate_t *State, sim_address_t address, int value );
int				ThumbGetExecBreakpoint		( thumbstate_t *State, sim_address_t address );
int				ThumbSetExecBreakpoint		( thumbstate_t *State, sim_address_t address, int value );
int				ThumbLoadByte				( thumbstate_t *State, sim_address_t address, uint8_t *value );
int				ThumbLoadHalf				( thumbstate_t *State, sim_address_t address, uint16_t *value );
int				ThumbLoadWord				( thumbstate_t *State, sim_address_t address, uint32_t *value );
int				ThumbLoadMemoryArray		( thumbstate_t *State, sim_address_t address, sim_address_t size, uint8_t *buffer );
int				ThumbLoadMapSegment			( thumbstate_t *State, sim_address_t address, sim_address_t length, sim_mapentry_t *buffer );
int				ThumbStoreByte				( thumbstate_t *State, sim_address_t address, uint8_t value );
int				ThumbStoreHalf				( thumbstate_t *State, sim_address_t address, uint16_t value );
int				ThumbStoreWord				( thumbstate_t *State, sim_address_t address, uint32_t value );
int				ThumbStoreMemoryArray		( thumbstate_t *State, sim_address_t address, sim_address_t size, const uint8_t *buffer );
int				ThumbGetCPSRValue			( thumbstate_t *State, uint32_t *value );
int				ThumbSetCPSRValue			( thumbstate_t *State, uint32_t value );
int				ThumbGetRegisterValue		( thumbstate_t *State, unsigned registernumber, uint32_t *value );
int				ThumbSetRegisterValue		( thumbstate_t *State, unsigned registernumber, uint32_t value );
int				ThumbGetAllRegistersValues	( thumbstate_t *State, uint32_t *Registers );
int				ThumbGetCore				( thumbstate_t *State, cpucore_t *Core, cpucore_t *PreviousCore );
/*============================================================================*/
typedef struct
	{
	uint32_t	Counter;
	uint32_t	Limit;
	uint32_t	KeyboardBuffer;
	uint32_t	Control;
	} peripherals_t;
/*============================================================================*/
int				ThumbPeriphRead				( sim_memoryarea_t *Area, sim_address_t Address, sim_address_t Size, void *Buffer );
int				ThumbPeriphWrite			( sim_memoryarea_t *Area, sim_address_t Address, sim_address_t Size, const void *Buffer );
/*============================================================================*/
#if			defined __cplusplus
	}
#endif	/*	defined __cplusplus */
/*============================================================================*/
#endif	/*	!defined __THUMB_H__ */
/*============================================================================*/
