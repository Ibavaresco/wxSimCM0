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
///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-0-g80c4cb6)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "InertPanel.h"
#include "MemoryDumpPanel.h"

#include "MemoryPanelBase.h"

///////////////////////////////////////////////////////////////////////////

MemoryPanelBase::MemoryPanelBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name ) : wxPanel( parent, id, pos, size, style, name )
{
	this->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNHIGHLIGHT ) );

	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer( wxHORIZONTAL );

	m_panel5 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxSize( -1,18 ), wxTAB_TRAVERSAL );
	m_panel5->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ) );

	wxBoxSizer* bSizer12;
	bSizer12 = new wxBoxSizer( wxHORIZONTAL );

	m_radioBtnByte = new wxRadioButton( m_panel5, wxID_BUTTON_BYTE, _("Byte"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer12->Add( m_radioBtnByte, 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 5 );

	m_radioBtnHalf = new wxRadioButton( m_panel5, wxID_BUTTON_HALF, _("Half"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer12->Add( m_radioBtnHalf, 0, wxALIGN_CENTER_VERTICAL, 5 );

	m_radioBtnWord = new wxRadioButton( m_panel5, wxID_BUTTON_WORD, _("Word"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer12->Add( m_radioBtnWord, 0, wxALIGN_CENTER_VERTICAL, 5 );


	m_panel5->SetSizer( bSizer12 );
	m_panel5->Layout();
	bSizer6->Add( m_panel5, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxLEFT, 5 );

	m_panel4 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxSize( -1,18 ), wxTAB_TRAVERSAL );
	m_panel4->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ) );

	wxBoxSizer* bSizer11;
	bSizer11 = new wxBoxSizer( wxHORIZONTAL );

	m_RadioBtnROM = new wxRadioButton( m_panel4, wxID_BUTTON_ROM, _("ROM"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer11->Add( m_RadioBtnROM, 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 5 );

	m_RadioBtnRAM = new wxRadioButton( m_panel4, wxID_BUTTON_RAM, _("RAM"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer11->Add( m_RadioBtnRAM, 0, wxALIGN_CENTER_VERTICAL, 5 );


	m_panel4->SetSizer( bSizer11 );
	m_panel4->Layout();
	bSizer6->Add( m_panel4, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

	m_panel8 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxSize( -1,18 ), wxTAB_TRAVERSAL );
	m_panel8->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ) );

	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer( wxHORIZONTAL );

	m_RadioBtnBigEndian = new wxRadioButton( m_panel8, wxID_BUTTON_BIG, _("Big Endian"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer5->Add( m_RadioBtnBigEndian, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 5 );

	m_RadioBtnLittleEndian = new wxRadioButton( m_panel8, wxID_BUTTON_LITTLE, _("Little Endian"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer5->Add( m_RadioBtnLittleEndian, 0, wxALIGN_CENTER_VERTICAL, 5 );


	m_panel8->SetSizer( bSizer5 );
	m_panel8->Layout();
	bSizer6->Add( m_panel8, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT, 5 );

	m_panel41 = new InertPanel( this, wxID_ANY, wxDefaultPosition, wxSize( -1,18 ), 0 );
	m_panel41->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ) );

	bSizer6->Add( m_panel41, 1, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT, 5 );


	bSizer4->Add( bSizer6, 0, wxEXPAND, 5 );

	m_PanelByte = new MemoryDumpPanelByte( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	bSizer4->Add( m_PanelByte, 1, wxEXPAND | wxALL, 5 );

	m_PanelHalf = new MemoryDumpPanelHalf( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	bSizer4->Add( m_PanelHalf, 1, wxEXPAND | wxALL, 5 );

	m_PanelWord = new MemoryDumpPanelWord( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	bSizer4->Add( m_PanelWord, 1, wxEXPAND | wxALL, 5 );


	this->SetSizer( bSizer4 );
	this->Layout();

	// Connect Events
	m_radioBtnByte->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( MemoryPanelBase::OnRadioButton ), NULL, this );
	m_radioBtnHalf->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( MemoryPanelBase::OnRadioButton ), NULL, this );
	m_radioBtnWord->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( MemoryPanelBase::OnRadioButton ), NULL, this );
	m_RadioBtnROM->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( MemoryPanelBase::OnRadioButton ), NULL, this );
	m_RadioBtnRAM->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( MemoryPanelBase::OnRadioButton ), NULL, this );
	m_RadioBtnBigEndian->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( MemoryPanelBase::OnRadioButton ), NULL, this );
	m_RadioBtnLittleEndian->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( MemoryPanelBase::OnRadioButton ), NULL, this );
}

MemoryPanelBase::~MemoryPanelBase()
{
}
