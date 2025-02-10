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
#pragma once
//==============================================================================
#include <stdint.h>
#include "MemoryPanelBase.h"
#include "MemoryDumpPanel.h"
//==============================================================================
class ThumbMemoryPanel : public MemoryPanelBase
	{
	//--------------------------------------------------------------------------
	private:
		uint32_t			m_ROMBufferLength;
		uint32_t			m_ROMBufferBaseAddress;
		bool				m_ROMBufferEditable;
		uint32_t			m_RAMBufferLength;
		uint32_t			m_RAMBufferBaseAddress;
		bool				m_RAMBufferEditable;

		MemoryDumpPanel		*m_CurrentPanel;
		unsigned			m_CurrentArea;
		MemoryDumpPanel::panelstate_t m_ROMState;
		MemoryDumpPanel::panelstate_t m_RAMState;
		unsigned			m_ROMCurrentPanel;
		unsigned			m_RAMCurrentPanel;
	//--------------------------------------------------------------------------
	public:
	//--------------------------------------------------------------------------
							ThumbMemoryPanel		( wxWindow			*parent,
													  wxWindowID		id = wxID_ANY,
													  const wxPoint		&pos = wxDefaultPosition,
													  const wxSize		&size = wxSize( 532,300 ),
													  long				style = 0,
													  const wxString	&name = wxEmptyString );
							~ThumbMemoryPanel		() {};
	//--------------------------------------------------------------------------
		virtual void		OnRadioButton			( wxCommandEvent& event ) override;
	//--------------------------------------------------------------------------
		bool				SetRAMBuffer			( uint32_t bufferbaseaddress, uint32_t bufferlength, bool editable, bool select );
		bool				SetROMBuffer			( uint32_t bufferbaseaddress, uint32_t bufferlength, bool editable, bool select );
		void				SetRAMChangedCallBack	( void (*callback)( uint32_t, uint32_t ) );
		void				SelectPanel				( unsigned panel );
		void				SelectROM				();
		void				SelectRAM				();
		void				DumpRedraw				();
	//--------------------------------------------------------------------------
	};
//==============================================================================
