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
#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/statline.h>
#include <wx/panel.h>
//==============================================================================
#define	ARM_REGISTERS_COUNT	16
#define	COLUMNS_COUNT		 3
//==============================================================================
class	RegisterEditCtrl;
class	CPSREditCtrl;
//==============================================================================
class RegistersPanel: public wxPanel
	{
	//--------------------------------------------------------------------------
	protected:
	//--------------------------------------------------------------------------
		wxPanel				*m_RegistersPanelInternal;
		wxPanel				*m_RegistersPanelInternal2;

		wxStaticText		*m_HeaderTexts[4];
		wxStaticText		*m_HeaderTextsCPSR[6];
		wxStaticText		*m_TextRegisterNames[ARM_REGISTERS_COUNT+2];
		wxStaticLine		*m_StaticLine0;
		wxStaticLine		*m_StaticLine;

		wxPanel				*m_CPSRPanel;

		wxStaticText		*m_StaticTextCPSR0;
		wxStaticText		*m_StaticTextReserved0;
		wxStaticText		*m_StaticTextReserved1;
		wxStaticText		*m_StaticTextCPSR1;
		wxStaticText		*m_StaticTextReserved2;
		wxStaticText		*m_StaticTextReserved3;

		wxPanel				*m_PRIMASKPanel;
		wxStaticText		*m_StaticPRIMASK01;
		wxStaticText		*m_StaticTextReserved01;
		wxStaticText		*m_StaticTextP;
		wxStaticText		*m_StaticTextPRIMASK02;
		wxStaticText		*m_StaticTextReserved21;
		wxTextCtrl			*m_TextCtrlPRIMASK;
		wxPanel				*m_CONTROLPanel;
		wxStaticText		*m_StaticTextCONTROL;
		wxStaticText		*m_StaticTextReserved11;
		wxStaticText		*m_StaticTextS;
		wxStaticText		*m_StaticTextM111;
		wxStaticText		*m_StaticTextCPSR111;
		wxStaticText		*m_StaticTextReserved211;
		wxTextCtrl			*m_TextCtrlS;
		wxStaticText		*m_StaticTextReserved2111;
	//--------------------------------------------------------------------------
	public:
	//--------------------------------------------------------------------------
		RegisterEditCtrl	*m_RegsEditCtrl[ARM_REGISTERS_COUNT+2][COLUMNS_COUNT];
		CPSREditCtrl		*m_BitsEditCtrl[6];

		unsigned			m_SelectedLine, m_SelectedColumn, m_SelectedBit;
	//--------------------------------------------------------------------------
	public:
	//--------------------------------------------------------------------------
							RegistersPanel		( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 356,300 ), long style = wxBORDER_SIMPLE|wxTAB_TRAVERSAL, const wxString& name = wxEmptyString );
							~RegistersPanel		() {};
		void				RedrawRegisters		();
		void				SelectCell			( unsigned line, unsigned column );
		void				SelectBit			( unsigned index );
	//--------------------------------------------------------------------------
	};
//==============================================================================
