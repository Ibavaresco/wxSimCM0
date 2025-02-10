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
//==============================================================================
#include "wxThumbApp.h"
#include "ThumbThread.h"
#include "Simulator.h"
#include "LoadELFFile.h"
#include "Thumb.h"
#include "Errors.h"
//==============================================================================
#define	LENGTH(v)	( sizeof (v) / sizeof (v)[0] )
//==============================================================================
wxDECLARE_APP( wxThumbApp );
/*============================================================================*/
const int	ThumbThread::Delays[17]	=
	{
	2512, 1585, 1000, 631, 398, 251, 158, 100, 63, 40, 25, 16, 10, 6, 4, 2, 1
	};
/*============================================================================*/
ThumbThread::ThumbThread( thumbstate_t *CPUState ) : wxThread()
	{
	ThumbInitialize( CPUState, 0x00080000, 0x00008000, 0x20000000, 0x00004000 );

	ThumbReset( CPUState, 1 );
//	CurrentSpeed			= 2;
//	CPUState->Delay			= Delays[CurrentSpeed];
	}
//==============================================================================
ThumbThread::~ThumbThread()
	{
	//ThumbFinalize( CPUState );
	}
//==============================================================================
wxThread::ExitCode ThumbThread::Entry()
	{
	// Setting thread name helps with debugging, as the debugger
	// may be able to show thread names along with the list of threads.
	SetName( "ThumbThread" );

//	wxLogMessage( "Thread started (priority = %u).", GetPriority() );

	while( 1 )
		{
		runningstate_t	RunningState;
		wxThumbApp		&App	= wxGetApp();

		// This construct may seem weird but it is necessary. We could remove the
		// 'do' and 'while(0)', but the braces must remain, because the local variable
		// 'Locker' must be created and destroyed regularly. It is an 'object' that
		// takes and releases a 'Mutex' in its constructor and destructor. Without
		// that the synchronization between the main application and the thread would
		// freeze.
		do
			{

			wxMutexLocker Locker( App.SimulatorMutex );

			if( App.ShuttingDown || TestDestroy() )
				{
				App.ShuttingDown = false;
				App.SemaphoreAllDone.Post();
				return nullptr;
				}
			if( App.CPUState.RunningState != RUNNINGSTATE_STOPPED )
				ThumbLoop( &App.CPUState );
			RunningState	= App.CPUState.RunningState;
			}
		while( 0 );

		if( RunningState == RUNNINGSTATE_STOPPED )
			Sleep( 50 );
		}

//	wxLogMessage( "Thread finished." );

	return nullptr;
	}
//==============================================================================
int ThumbThread::New()
	{
	wxThumbApp	&App	= wxGetApp();

	wxMutexLocker Locker( App.SimulatorMutex );

	ThumbFinalize( &App.CPUState );

	ThumbInitialize( &App.CPUState, 0x00080000, 0x00008000, 0x20000000, 0x00004000 );

	ThumbReset( &App.CPUState, 1 );

	return 0;
	}
//==============================================================================
int ThumbThread::LoadFile( const wxString &Path )
	{
	wxThumbApp	&App	= wxGetApp();

	wxMutexLocker Locker( App.SimulatorMutex );

	ThumbFinalize( &App.CPUState );

	int	Result	= SimulatorLoadELFFile( &App.CPUState, Path );

	if( Result < 0 )
		return Result;

	if( CreateSimulatorMemoryArea( &App.CPUState.MemoryAreas[App.CPUState.NumMemoryAreas], 0x41005000, 32, 1, 0, 1, NULL, NULL, NULL, NULL ) < 0 )
		return -1;

	App.CPUState.MemoryAreas[App.CPUState.NumMemoryAreas]->BaseAddress	= 0x41005000;
	App.CPUState.MemoryAreas[App.CPUState.NumMemoryAreas]->LastAddress	= 0x41005000 + 32 - 1;

	App.CPUState.NumMemoryAreas++;

	if( CreateSimulatorMemoryArea( &App.CPUState.MemoryAreas[App.CPUState.NumMemoryAreas], 0x41007000, 16, 0, 0, 0, NULL, ThumbPeriphRead, ThumbPeriphWrite, NULL ) < 0 )
		return -1;

	App.CPUState.MemoryAreas[App.CPUState.NumMemoryAreas]->BaseAddress	= 0x41007000;
	App.CPUState.MemoryAreas[App.CPUState.NumMemoryAreas]->LastAddress	= 0x41007000 + 16 - 1;

	App.CPUState.NumMemoryAreas++;

	return 0;
	}
//==============================================================================
int ThumbThread::GetLCDContents( char *Buffer )
	{
	wxMutexLocker Locker( wxGetApp().SimulatorMutex );
/*
	memcpy( Buffer, &CPUState->Memory[ADDRESS_LCD], 36 );
	Buffer[36]	= '\0';

	CPUState->ChangedLCD	= 0;
*/
	return 0;
	}
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
int ThumbThread::Prepare()
	{
	wxThumbApp	&App	= wxGetApp();

	wxMutexLocker Locker( App.SimulatorMutex );

	memcpy( &App.CPUState.PreviousCore, &App.CPUState.Core, sizeof App.CPUState.PreviousCore );

	for( unsigned i = 0; i < App.CPUState.NumMemoryAreas; i++ )
		if( App.CPUState.MemoryAreas[i] != nullptr && App.CPUState.MemoryAreas[i]->Previous != nullptr )
			memcpy( App.CPUState.MemoryAreas[i]->Previous, App.CPUState.MemoryAreas[i]->Buffer, App.CPUState.MemoryAreas[i]->Size );

	return 0;
	}
//==============================================================================
int ThumbThread::Reset( bool full )
	{
	wxMutexLocker Locker( wxGetApp().SimulatorMutex );

	int	Result	= ThumbReset( &wxGetApp().CPUState, full );
	if( Result < 0 )
		return Result;

	Prepare();

	return Result;
	}
//==============================================================================
int ThumbThread::ClearTrace()
	{
	wxMutexLocker Locker( wxGetApp().SimulatorMutex );

	return ThumbClearTrace( &wxGetApp().CPUState );
	}
//==============================================================================
int ThumbThread::DeleteAllBreakpoints()
	{
	wxMutexLocker Locker( wxGetApp().SimulatorMutex );

	return ThumbClearAllBreakpoints( &wxGetApp().CPUState );
	}
//==============================================================================
int ThumbThread::ToggleBreakpoint()
	{
	wxMutexLocker Locker( wxGetApp().SimulatorMutex );

	return ThumbToggleBreakpoint( &wxGetApp().CPUState );
	}
//==============================================================================
int ThumbThread::Go()
	{
	wxMutexLocker Locker( wxGetApp().SimulatorMutex );

	if( wxGetApp().SimulatorThread->IsBusy() )
		return -1;

	Prepare();

	return ThumbRun( &wxGetApp().CPUState );
	}
//==============================================================================
int ThumbThread::Pause()
	{
	wxMutexLocker Locker( wxGetApp().SimulatorMutex );

	return ThumbPause( &wxGetApp().CPUState );
	}
//==============================================================================
int ThumbThread::Animate()
	{
	wxMutexLocker Locker( wxGetApp().SimulatorMutex );

	if( wxGetApp().SimulatorThread->IsBusy() )
		return -1;

	Prepare();

//	return SimulatorAnimate( CPUState );
	return 0;
	}
//==============================================================================
int ThumbThread::MaxSpeed()
	{
	wxMutexLocker Locker( wxGetApp().SimulatorMutex );

	if( wxGetApp().SimulatorThread->IsBusy() )
		return -1;

	Prepare();

//	return SimulatorMaxSpeed( CPUState );
	return 0;
	}
//==============================================================================
int ThumbThread::StepInto()
	{
	wxMutexLocker Locker( wxGetApp().SimulatorMutex );

	if( wxGetApp().SimulatorThread->IsBusy() )
		return -1;

	Prepare();

	return ThumbStepInto( &wxGetApp().CPUState );
	}
//==============================================================================
int ThumbThread::StepOver()
	{
	wxMutexLocker Locker( wxGetApp().SimulatorMutex );

	if( wxGetApp().SimulatorThread->IsBusy() )
		return -1;

	Prepare();

	return ThumbStepOver( &wxGetApp().CPUState );
	}
//==============================================================================
bool ThumbThread::SpeedUp()
	{
	wxMutexLocker Locker( wxGetApp().SimulatorMutex );

	if( ++CurrentSpeed >= (int)LENGTH( Delays ))
		CurrentSpeed	= LENGTH( Delays ) - 1;

//	CPUState->Delay	= Delays[CurrentSpeed];

	return CurrentSpeed < (int)LENGTH( Delays ) - 1;
	}
//==============================================================================
bool ThumbThread::SlowDown()
	{
	wxMutexLocker Locker( wxGetApp().SimulatorMutex );

	if( --CurrentSpeed < 0 )
		CurrentSpeed	= 0;

//	CPUState->Delay	= Delays[CurrentSpeed];

	return CurrentSpeed > 0;
	}
//==============================================================================
void ThumbThread::SetMemoryWindow( uint16_t startaddress, uint16_t endaddress )
	{
	wxMutexLocker Locker( wxGetApp().SimulatorMutex );

	ThumbSetMemoryWindow( &wxGetApp().CPUState, startaddress, endaddress );
	}
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
bool ThumbThread::IsBusy()
	{
	wxMutexLocker Locker( wxGetApp().SimulatorMutex );

	return (bool)ThumbIsBusy( &wxGetApp().CPUState );
	}
//==============================================================================
int ThumbThread::GetSimulatorUpdated()
	{
	wxMutexLocker Locker( wxGetApp().SimulatorMutex );

	return ThumbGetSimulatorUpdated( &wxGetApp().CPUState );
	}
//==============================================================================
int ThumbThread::GetSimulatorStatus()
	{
	wxMutexLocker Locker( wxGetApp().SimulatorMutex );

	return ThumbGetSimulatorStatus( &wxGetApp().CPUState );
	}
//==============================================================================
void ThumbThread::ClearSimulatorUpdated()
	{
	wxMutexLocker Locker( wxGetApp().SimulatorMutex );

	ThumbClearSimulatorUpdated( &wxGetApp().CPUState );
	}
//==============================================================================
void ThumbThread::ClearSimulatorStatus()
	{
	wxMutexLocker Locker( wxGetApp().SimulatorMutex );

	ThumbClearSimulatorStatus( &wxGetApp().CPUState );
	}
//==============================================================================
int ThumbThread::GetDataBreakpoint( sim_address_t address )
	{
	wxMutexLocker Locker( wxGetApp().SimulatorMutex );

	return ThumbGetDataBreakpoint( &wxGetApp().CPUState, address );
	}
//==============================================================================
int ThumbThread::SetDataBreakpoint( sim_address_t address, int value )
	{
	wxMutexLocker Locker( wxGetApp().SimulatorMutex );

	return ThumbSetDataBreakpoint( &wxGetApp().CPUState, address, value );
	}
//==============================================================================
int ThumbThread::GetExecBreakpoint( sim_address_t address )
	{
	wxMutexLocker Locker( wxGetApp().SimulatorMutex );

	return ThumbGetExecBreakpoint( &wxGetApp().CPUState, address );
	}
//==============================================================================
int ThumbThread::SetExecBreakpoint( sim_address_t address, int value )
	{
	wxMutexLocker Locker( wxGetApp().SimulatorMutex );

	return ThumbSetExecBreakpoint( &wxGetApp().CPUState, address, value );
	}
//==============================================================================
int ThumbThread::LoadByte( sim_address_t address, uint8_t &value )
	{
	wxMutexLocker Locker( wxGetApp().SimulatorMutex );

	return ThumbLoadByte( &wxGetApp().CPUState, address, &value );
	}
//==============================================================================
int ThumbThread::LoadHalf( sim_address_t address, uint16_t &value )
	{
	wxMutexLocker Locker( wxGetApp().SimulatorMutex );

	return ThumbLoadHalf( &wxGetApp().CPUState, address, &value );
	}
//==============================================================================
int ThumbThread::LoadWord( sim_address_t address, uint32_t &value )
	{
	wxMutexLocker Locker( wxGetApp().SimulatorMutex );

	return ThumbLoadWord( &wxGetApp().CPUState, address, &value );
	}
//==============================================================================
int ThumbThread::LoadMemoryArray( sim_address_t address, sim_address_t size, uint8_t *buffer )
	{
	wxMutexLocker Locker( wxGetApp().SimulatorMutex );

	return ThumbLoadMemoryArray( &wxGetApp().CPUState, address, size, buffer );
	}
//==============================================================================
int ThumbThread::LoadMapSegment( sim_address_t address, sim_address_t length, sim_mapentry_t *buffer )
	{
	wxMutexLocker Locker( wxGetApp().SimulatorMutex );

	return ThumbLoadMapSegment( &wxGetApp().CPUState, address, length, buffer );
	}
//==============================================================================
int ThumbThread::StoreByte( sim_address_t address, uint8_t value )
	{
	wxMutexLocker Locker( wxGetApp().SimulatorMutex );

	return ThumbStoreByte( &wxGetApp().CPUState, address, value );
	}
//==============================================================================
int ThumbThread::StoreHalf( sim_address_t address, uint16_t value )
	{
	wxMutexLocker Locker( wxGetApp().SimulatorMutex );

	return ThumbStoreHalf( &wxGetApp().CPUState, address, value );
	}
//==============================================================================
int ThumbThread::StoreWord( sim_address_t address, uint32_t value )
	{
	wxMutexLocker Locker( wxGetApp().SimulatorMutex );

	return ThumbStoreWord( &wxGetApp().CPUState, address, value );
	}
//==============================================================================
int ThumbThread::StoreMemoryArray( sim_address_t address, sim_address_t size, const uint8_t *buffer )
	{
	wxMutexLocker Locker( wxGetApp().SimulatorMutex );

	return ThumbStoreMemoryArray( &wxGetApp().CPUState, address, size, buffer );
	}
//==============================================================================
int ThumbThread::GetCPSRValue( uint32_t &value )
	{
	wxMutexLocker Locker( wxGetApp().SimulatorMutex );

	return ThumbGetCPSRValue( &wxGetApp().CPUState, &value );
	}
//==============================================================================
int ThumbThread::SetCPSRValue( uint32_t value )
	{
	wxMutexLocker Locker( wxGetApp().SimulatorMutex );

	return ThumbSetCPSRValue( &wxGetApp().CPUState, value );
	}
//==============================================================================
int ThumbThread::GetRegisterValue( unsigned registernumber, uint32_t &value )
	{
	wxMutexLocker Locker( wxGetApp().SimulatorMutex );

	return ThumbGetRegisterValue( &wxGetApp().CPUState, registernumber, &value );
	}
//==============================================================================
int ThumbThread::SetRegisterValue( unsigned registernumber, uint32_t value )
	{
	wxMutexLocker Locker( wxGetApp().SimulatorMutex );

	return ThumbSetRegisterValue( &wxGetApp().CPUState, registernumber, value );
	}
//==============================================================================
void ThumbThread::GetAllRegistersValues( uint32_t *regs )
	{
	wxMutexLocker Locker( wxGetApp().SimulatorMutex );

	ThumbGetAllRegistersValues( &wxGetApp().CPUState, regs );
	}
//==============================================================================
void ThumbThread::GetCore( cpucore_t *core, cpucore_t *previouscore )
	{
	wxMutexLocker Locker( wxGetApp().SimulatorMutex );

	ThumbGetCore( &wxGetApp().CPUState, core, previouscore );
	}
//==============================================================================
void ThumbThread::GetCoreDiff( cpucore_t &core )
	{
	wxThumbApp	&App		= wxGetApp();

	wxMutexLocker Locker( App.SimulatorMutex );

	App.CPUState.Core.CONTROL.ASPSEL == 0 ? ( App.CPUState.Core.MSP = App.CPUState.Core.SP ) : ( App.CPUState.Core.PSP = App.CPUState.Core.SP );

	for( int i = 0; i < 16 + 2; i++ )
		{
		core.Regs[i]		= App.CPUState.Core.Regs[i];
		core.Written[i]		= App.CPUState.Core.Written[i] == 3 ? 3 : App.CPUState.Core.Regs[i] != App.CPUState.PreviousCore.Regs[i] ? 2 : App.CPUState.Core.Written[i] == 1 ? 1 : 0;
		}

	core.CPSR				= App.CPUState.Core.CPSR;

	core.CPSRWrittenByApp	= App.CPUState.Core.CPSRWrittenByApp | ( App.CPUState.Core.CPSR.Value ^ App.CPUState.PreviousCore.CPSR.Value );
	core.Written[CPSR]		= ( App.CPUState.Core.Written[CPSR] & ~( App.CPUState.Core.CPSR.Value ^ App.CPUState.PreviousCore.CPSR.Value )) | App.CPUState.Core.CPSRWrittenByApp;
	}
//==============================================================================
bool ThumbThread::GetMemoryDiff( sim_address_t address, sim_address_t size, uint8_t *buffer, uint8_t *diff )
	{
	const sim_memoryarea_t	*Area		= nullptr;

	wxThumbApp				&App		= wxGetApp();

	wxMutexLocker Locker( App.SimulatorMutex );

	unsigned i;

	for( i = 0; i < App.CPUState.NumMemoryAreas; i++ )
		if( App.CPUState.MemoryAreas[i] != nullptr && address >= App.CPUState.MemoryAreas[i]->BaseAddress && address <= App.CPUState.MemoryAreas[i]->LastAddress )
			{
			Area		= App.CPUState.MemoryAreas[i];
			break;
			}

	if( i >= App.CPUState.NumMemoryAreas )
		{
		memset( buffer, 0x00, size );
		memset( diff, 4, size );
		return false;
		}

	sim_address_t j;

	for( j = 0; j < size && address <= Area->LastAddress; j++, address++ )
		{
		buffer[j]	= Area->Buffer[address - Area->BaseAddress];

		diff[j]		= Area->Map[address - Area->BaseAddress].Written == 3 ? 3
					: Area->Buffer[address - Area->BaseAddress] != Area->Previous[address - Area->BaseAddress] ? 2
					: Area->Map[address - Area->BaseAddress].Written == 1 ? 1 : 0;
		}

	for( ; j < size; j++ )
		{
		buffer[j]	= 0x00;
		diff[j]		= 4;
		}

	return true;
	}
//==============================================================================
