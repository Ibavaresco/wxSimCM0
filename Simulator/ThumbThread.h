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
#if			!defined __THUMBTHREAD_H__
#define __THUMBTHREAD_H__
//==============================================================================
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/thread.h"
#include "Thumb.h"
//==============================================================================
class ThumbThread : public wxThread
	{
	//--------------------------------------------------------------------------
	public:
							ThumbThread				( thumbstate_t *CPUState );
		virtual				~ThumbThread			();
	//--------------------------------------------------------------------------
	public:
		// thread execution starts here
		virtual void		*Entry					() override;
	//--------------------------------------------------------------------------
	public:
		typedef char		lcd_t[36];
	//--------------------------------------------------------------------------
	public:
		int					LoadFile				( const wxString &Path );
		int					New						();
		int					GetLCDContents			( char *Buffer );
		void				SetMemoryWindow			( uint16_t startaddress, uint16_t endaddress );
	//--------------------------------------------------------------------------
		bool				IsBusy					();
		int					GetSimulatorUpdated		();
		int					GetSimulatorStatus		();
		void				ClearSimulatorUpdated	();
		void				ClearSimulatorStatus	();
		int					GetDataBreakpoint		( sim_address_t address );
		int					SetDataBreakpoint		( sim_address_t address, int value );
		int					GetExecBreakpoint		( sim_address_t address );
		int					SetExecBreakpoint		( sim_address_t address, int value );
		int					LoadByte				( sim_address_t address, uint8_t &value );
		int					LoadHalf				( sim_address_t address, uint16_t &value );
		int					LoadWord				( sim_address_t address, uint32_t &value );
		int					LoadMemoryArray			( sim_address_t address, sim_address_t size, uint8_t *buffer );
		int					LoadMapSegment			( sim_address_t address, sim_address_t length, sim_mapentry_t *buffer );
		int					StoreByte				( sim_address_t address, uint8_t value );
		int					StoreHalf				( sim_address_t address, uint16_t value );
		int					StoreWord				( sim_address_t address, uint32_t value );
		int					StoreMemoryArray		( sim_address_t address, sim_address_t size, const uint8_t *buffer );
		int					GetCPSRValue			( uint32_t &value );
		int					SetCPSRValue			( uint32_t value );
		int					GetRegisterValue		( unsigned registernumber, uint32_t &value );
		int					SetRegisterValue		( unsigned registernumber, uint32_t value );
		void				GetAllRegistersValues	( uint32_t *regs );
		void				GetCore					( cpucore_t *core, cpucore_t *previouscore );
		void				GetCoreDiff				( cpucore_t &core );
		bool				GetMemoryDiff			( sim_address_t address, sim_address_t size, uint8_t *buffer, uint8_t *diff );
	//--------------------------------------------------------------------------
		int					Prepare					();

		int					Reset					( bool full );
		int					ClearTrace				();
		int					DeleteAllBreakpoints	();
		int					ToggleBreakpoint		();
		int					Go						();
		int					Pause					();
		int					Animate					();
		int					MaxSpeed				();
		int					StepInto				();
		int					StepOver				();
		bool				SpeedUp					();
		bool				SlowDown				();
	//--------------------------------------------------------------------------
		int					CurrentSpeed;
		static const int	Delays[17];
	//--------------------------------------------------------------------------
	};
//==============================================================================
#endif	//	!defined __THUMBTHREAD_H__
//==============================================================================
