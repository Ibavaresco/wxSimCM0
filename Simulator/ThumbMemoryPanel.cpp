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
#include "ThumbMemoryPanel.h"
//==============================================================================
ThumbMemoryPanel::ThumbMemoryPanel( wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, long style, const wxString &name )
									: MemoryPanelBase( parent, id, pos, size, style, name )
	{
	m_RadioBtnROM->Disable();
	m_RadioBtnRAM->Disable();

	m_ROMBufferLength		= 0;
	m_ROMBufferBaseAddress	= 0;
	m_ROMBufferEditable		= false;
	m_RAMBufferLength		= 0;
	m_RAMBufferBaseAddress	= 0;
	m_RAMBufferEditable		= false;

	m_ROMState.Side			= 0;
	m_ROMState.Line			= 0;
	m_ROMState.Column		= 0;
	m_ROMState.StartAddress	= 0;

	m_RAMState.Side			= 0;
	m_RAMState.Line			= 0;
	m_RAMState.Column		= 0;
	m_RAMState.StartAddress	= 0;

	m_ROMCurrentPanel		= 0;
	m_RAMCurrentPanel		= 0;

	m_PanelHalf->Hide();
	m_PanelWord->Hide();
	m_PanelByte->Show();
	this->Layout();

	m_RadioBtnRAM->SetValue( true );
	m_radioBtnByte->SetValue( true );
	m_RadioBtnLittleEndian->SetValue( true );

	m_CurrentPanel			= m_PanelByte;
	m_CurrentArea			= 1;
	}
//==============================================================================
void ThumbMemoryPanel::SelectPanel( unsigned panel )
	{
	switch( panel )
		{
		default:
		case 0:
			m_PanelWord->Hide();
			m_PanelHalf->Hide();
			m_PanelByte->Show();
			this->Layout();
			m_CurrentPanel	= m_PanelByte;
			m_radioBtnByte->SetValue( true );
			break;
		case 1:
			m_PanelByte->Hide();
			m_PanelWord->Hide();
			m_PanelHalf->Show();
			this->Layout();
			m_CurrentPanel	= m_PanelHalf;
			m_radioBtnHalf->SetValue( true );
			break;
		case 2:
			m_PanelHalf->Hide();
			m_PanelByte->Hide();
			m_PanelWord->Show();
			this->Layout();
			m_CurrentPanel	= m_PanelWord;
			m_radioBtnWord->SetValue( true );
			break;
		}
	}
//==============================================================================
void ThumbMemoryPanel::SelectROM()
	{
	m_CurrentArea	= 0;

	m_RadioBtnROM->SetValue( true );

	m_PanelByte->SetBuffer( m_ROMBufferBaseAddress, m_ROMBufferLength, true );
	m_PanelHalf->SetBuffer( m_ROMBufferBaseAddress, m_ROMBufferLength, true );
	m_PanelWord->SetBuffer( m_ROMBufferBaseAddress, m_ROMBufferLength, true );

	SelectPanel( m_ROMCurrentPanel );
	}
//==============================================================================
void ThumbMemoryPanel::SelectRAM()
	{
	m_CurrentArea	= 1;

	m_RadioBtnRAM->SetValue( true );

	m_PanelByte->SetBuffer( m_RAMBufferBaseAddress, m_RAMBufferLength, true );
	m_PanelHalf->SetBuffer( m_RAMBufferBaseAddress, m_RAMBufferLength, true );
	m_PanelWord->SetBuffer( m_RAMBufferBaseAddress, m_RAMBufferLength, true );

	SelectPanel( m_RAMCurrentPanel );
	}
//==============================================================================
void ThumbMemoryPanel::OnRadioButton( wxCommandEvent& event )
	{
	int	Id	= event.GetId();
	switch( Id )
		{
		case wxID_BUTTON_BIG:
			if( m_CurrentPanel->GetEndianness() )
				break;

			m_PanelByte->SetEndianness( true );
			m_PanelHalf->SetEndianness( true );
			m_PanelWord->SetEndianness( true );

			m_RadioBtnBigEndian->SetValue( true );
			break;

		case wxID_BUTTON_LITTLE:
			if( !m_CurrentPanel->GetEndianness() )
				break;

			m_PanelByte->SetEndianness( false );
			m_PanelHalf->SetEndianness( false );
			m_PanelWord->SetEndianness( false );

			m_RadioBtnLittleEndian->SetValue( true );
			break;

		case wxID_BUTTON_ROM:
			if( m_CurrentArea == 0 )
				break;

			m_CurrentPanel->GetState( m_RAMState );
			SelectROM();
			m_CurrentPanel->SetState( m_ROMState );
			if( m_ROMState.BigEndian )
				m_RadioBtnBigEndian->SetValue( true );
			else
				m_RadioBtnLittleEndian->SetValue( true );
			break;

		case wxID_BUTTON_RAM:
			if( m_CurrentArea != 0 )
				break;

			m_CurrentPanel->GetState( m_ROMState );
			SelectRAM();
			m_CurrentPanel->SetState( m_RAMState );
			if( m_RAMState.BigEndian )
				m_RadioBtnBigEndian->SetValue( true );
			else
				m_RadioBtnLittleEndian->SetValue( true );
			break;

		case wxID_BUTTON_BYTE:
			if( m_CurrentPanel == m_PanelByte )
				break;

			m_CurrentArea == 0 ? m_ROMCurrentPanel = 0 : m_RAMCurrentPanel = 0;
			m_CurrentPanel->GetState( m_CurrentArea == 0 ? m_ROMState : m_RAMState );
			m_PanelByte->SetState( m_CurrentArea == 0 ? m_ROMState : m_RAMState );
			m_PanelByte->DumpRedraw();
			SelectPanel( 0 );
			break;

		case wxID_BUTTON_HALF:
			if( m_CurrentPanel == m_PanelHalf )
				break;
			m_CurrentArea == 0 ? m_ROMCurrentPanel = 1 : m_RAMCurrentPanel = 1;
			m_CurrentPanel->GetState( m_CurrentArea == 0 ? m_ROMState : m_RAMState );
			m_PanelHalf->SetState( m_CurrentArea == 0 ? m_ROMState : m_RAMState );
			m_PanelHalf->DumpRedraw();
			SelectPanel( 1 );
			break;

		case wxID_BUTTON_WORD:
			if( m_CurrentPanel == m_PanelWord )
				break;
			m_CurrentArea == 0 ? m_ROMCurrentPanel = 2 : m_RAMCurrentPanel = 2;
			m_CurrentPanel->GetState( m_CurrentArea == 0 ? m_ROMState : m_RAMState );
			m_PanelWord->SetState( m_CurrentArea == 0 ? m_ROMState : m_RAMState );
			m_PanelWord->DumpRedraw();
			SelectPanel( 2 );
			break;
		}
	}
//==============================================================================
bool ThumbMemoryPanel::SetRAMBuffer( uint32_t bufferbaseaddress, uint32_t bufferlength, bool editable, bool select )
	{
	m_RAMBufferLength		= bufferlength;
	m_RAMBufferBaseAddress	= bufferbaseaddress;
	m_RAMBufferEditable		= editable;

	if( bufferlength == 0 )
		m_RadioBtnRAM->Disable();
	else
		{
		m_RadioBtnRAM->Enable();
		if( select )
			{
			m_RAMState	= { .Side = 0, .Line = 0, .Column = 0, .StartAddress = bufferbaseaddress };
			m_PanelByte->SetBuffer( bufferbaseaddress, bufferlength, editable );
			m_PanelHalf->SetBuffer( bufferbaseaddress, bufferlength, editable );
			m_PanelWord->SetBuffer( bufferbaseaddress, bufferlength, editable );

			return true;
			}
		}
	return false;
	}
//==============================================================================
bool ThumbMemoryPanel::SetROMBuffer( uint32_t bufferbaseaddress, uint32_t bufferlength, bool editable, bool select )
	{
	m_ROMBufferLength		= bufferlength;
	m_ROMBufferBaseAddress	= bufferbaseaddress;
	m_ROMBufferEditable		= editable;

	if( bufferlength == 0 )
		m_RadioBtnROM->Disable();
	else
		{
		m_RadioBtnROM->Enable();
		if( select )
			{
			m_ROMState	= { .Side = 0, .Line = 0, .Column = 0, .StartAddress = bufferbaseaddress };
			m_PanelByte->SetBuffer( bufferbaseaddress, bufferlength, editable );
			m_PanelHalf->SetBuffer( bufferbaseaddress, bufferlength, editable );
			m_PanelWord->SetBuffer( bufferbaseaddress, bufferlength, editable );

			return true;
			}
		}
	return false;
	}
//==============================================================================
void ThumbMemoryPanel::SetRAMChangedCallBack( void (*callback)( uint32_t, uint32_t ) )
	{
	}
//==============================================================================
void ThumbMemoryPanel::DumpRedraw()
	{
	m_CurrentPanel->DumpRedraw();
	}
//==============================================================================
