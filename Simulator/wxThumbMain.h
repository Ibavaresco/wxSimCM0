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
#ifndef WXTHUMBMAIN_H
#define WXTHUMBMAIN_H
//==============================================================================
#include "wxThumbApp.h"
#include "wxThumbFrameBase.h"
#include "Thumb.h"
//==============================================================================
class wxThumbDialog: public wxThumbFrameBase
	{
    public:
		//----------------------------------------------------------------------
							wxThumbDialog				( wxDialog *dlg );
							~wxThumbDialog				();
		//----------------------------------------------------------------------
    protected:
		//----------------------------------------------------------------------
        virtual void		OnClose						( wxCloseEvent &event );
        virtual void		OnQuit						( wxCommandEvent &event ) override;
        virtual void		OnAbout						( wxCommandEvent &event ) override;

		virtual void		New							( wxCommandEvent &event ) override;
		virtual void		LoadELFFile					( wxCommandEvent &event ) override;
		virtual void		OnStepInto					( wxCommandEvent &event ) override;
		virtual void		OnStepOver					( wxCommandEvent &event ) override;
		virtual void		OnAnimate					( wxCommandEvent &event ) override;
		virtual void		OnRun						( wxCommandEvent &event ) override;
		virtual void		OnMaxSpeed					( wxCommandEvent &event ) override;
		virtual void		OnPause						( wxCommandEvent &event ) override;
		virtual void		OnResetCPU					( wxCommandEvent &event ) override;
		virtual void		OnFullReset					( wxCommandEvent &event ) override;
		virtual void		OnClearTrace				( wxCommandEvent &event ) override;
		virtual void		OnMenuDeleteAllBreakpoints	( wxCommandEvent &event ) override;
		virtual void		OnMenuToggleBreakpoint		( wxCommandEvent &event ) override;
		//----------------------------------------------------------------------
		virtual void		IdleLoop					( wxIdleEvent &event ) override;
		void				RedrawAll					();
		//----------------------------------------------------------------------
		char				m_LCDBuffer[33];
		wxThumbApp			*App;
		wxString			Title;
		wxString			FileName;
		unsigned			m_LastMessageId;

		uint32_t			m_LastPC;
		//----------------------------------------------------------------------
	public:
		//----------------------------------------------------------------------
		void 				ShowMessage					( unsigned MessageId, const wxString &Message, const wxColour *Colour );
		void				RefreshLCD					();
		//----------------------------------------------------------------------
	};
//==============================================================================
#endif // WXTHUMBMAIN_H
//==============================================================================
