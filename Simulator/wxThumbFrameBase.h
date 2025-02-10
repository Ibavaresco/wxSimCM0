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

#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
class DisassemblyPanel;
class RegistersPanel;
class ThumbMemoryPanel;

#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/menu.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/statusbr.h>
#include <wx/frame.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class wxThumbFrameBase
///////////////////////////////////////////////////////////////////////////////
class wxThumbFrameBase : public wxFrame
{
	private:

	protected:
		wxMenuBar* m_MenuBar;
		wxMenu* m_MenuFile;
		wxMenu* m_MenuRun;
		wxMenu* m_MenuHelp;
		wxPanel* m_PanelMain;
		DisassemblyPanel* m_PanelDisassembly;
		ThumbMemoryPanel* m_PanelMemory;
		RegistersPanel* m_PanelRegisters;
		wxBitmapButton* m_BitmapLCD;
		wxStatusBar* m_StatusBar;

		// Virtual event handlers, override them in your derived class
		virtual void IdleLoop( wxIdleEvent& event ) { event.Skip(); }
		virtual void New( wxCommandEvent& event ) { event.Skip(); }
		virtual void LoadELFFile( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnQuit( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnStepInto( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnStepOver( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnAnimate( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnRun( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnMaxSpeed( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnPause( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnResetCPU( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnFullReset( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnClearTrace( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnMenuDeleteAllBreakpoints( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnMenuToggleBreakpoint( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnAbout( wxCommandEvent& event ) { event.Skip(); }


	public:

		wxThumbFrameBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("wxSimCM0 - A Simulator for a Fictitious ARM Thumb Computer"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_FRAME_STYLE|wxCLIP_CHILDREN|wxTAB_TRAVERSAL );

		~wxThumbFrameBase();

};

