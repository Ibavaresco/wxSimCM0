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
#ifdef WX_PRECOMP
#include "wx_pch.h"
#endif
/*============================================================================*/
#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__
/*============================================================================*/
#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include "wxThumbMain.h"
#include "MemoryPanelBase.h"
#include "MemoryDumpPanel.h"
#include "ThumbMemoryPanel.h"
#include "DisassemblyPanel.h"
#include "LoadELFFile.h"
#include "Errors.h"
#include "RegistersPanel.h"
/*============================================================================*/
wxDECLARE_APP( wxThumbApp );
//==============================================================================
enum wxbuildinfoformat
	{
    short_f,
    long_f
    };
/*============================================================================*/
wxString wxbuildinfo( wxbuildinfoformat format )
	{
    wxString wxbuild(wxVERSION_STRING);

    if( format == long_f )
		{
#if defined(__WXMSW__)
        wxbuild << _T("-Windows");
#elif defined(__WXMAC__)
        wxbuild << _T("-Mac");
#elif defined(__UNIX__)
        wxbuild << _T("-Linux");
#endif

#if wxUSE_UNICODE
        wxbuild << _T("-Unicode build");
#else
        wxbuild << _T("-ANSI build");
#endif // wxUSE_UNICODE
		}

    return wxbuild;
	}
/*============================================================================*/
wxThumbDialog::wxThumbDialog( wxDialog *dlg ) : wxThumbFrameBase( dlg )
	{
	App			= &wxGetApp();
	Title		= this->GetLabel();
	m_LastPC	= -1;

	const sim_memoryarea_t	*ROM	= App->CPUState.MemoryAreas[0];
	const sim_memoryarea_t	*RAM	= App->CPUState.MemoryAreas[1];

	m_PanelMemory->SetROMBuffer( ROM->BaseAddress, ROM->LastAddress - ROM->BaseAddress + 1, true, false );
	m_PanelMemory->SetRAMBuffer( RAM->BaseAddress, RAM->LastAddress - RAM->BaseAddress + 1, true, true );
	App->SimulatorThread->Reset( true );
	RedrawAll();
	}
/*============================================================================*/
wxThumbDialog::~wxThumbDialog()
	{
	}
/*============================================================================*/
#include "Thumb.h"

void wxThumbDialog::RefreshLCD()
	{
#if         0 && defined __WXMSW__
	char	Buffer[36*2+1];
	char	Buffer2[36+1];
	int		i, j;

	App->SimulatorThread->GetLCDContents( Buffer2 );

	/* We must replace every '&' with '&&', because the ampersand has special meaning
	   in Windows and in wxWidgets. It is not displayed and causes the next character
	   to be shown underscored. The double ampersand is shown as just one normal ampersand. */
	for( i = 0, j = 0; i < 36; i++ )
		if( Buffer2[i] != '&' )
			Buffer[j++]	= Buffer2[i];
		else
			{
			Buffer[j++]	= '&';
			Buffer[j++]	= '&';
			}
	Buffer[j]	= '\0';
#else   //  defined __WXMSW__
	char	Buffer[36+1];
	App->SimulatorThread->GetLCDContents( Buffer );
#endif  //  defined __WXMSW__

	char	LCDBuffer[33];

	for( int i = 0; i < 32; i++ )
		{
		uint8_t	c;

		if( ThumbLoadByte( &wxGetApp().CPUState, 0x41005000 + i, &c ) < 0 || !isprint( c ))
			c	= ' ';

		LCDBuffer[i]	= c;
		}
	LCDBuffer[32]	= '\0';

	if( strcmp( LCDBuffer, m_LCDBuffer ) == 0 )
		return;

	memcpy( m_LCDBuffer, LCDBuffer, sizeof m_LCDBuffer );

	wxBitmap	BitMap( 878, 30 );

	wxMemoryDC dc( BitMap );
	dc.SetBackground( *wxGREEN_BRUSH );
	dc.SetFont( wxFont( 21, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT( "Courier New" )));
	dc.Clear();
	dc.DrawText( LCDBuffer, 4, 0 );

	m_BitmapLCD->SetBitmap( BitMap );
//	m_BitmapLCD->Refresh( true );
	}
//==============================================================================
void wxThumbDialog::ShowMessage( unsigned MessageId, const wxString &Message, const wxColour *Colour )
	{
	if( m_LastMessageId == MessageId )
		return;

	m_LastMessageId	= MessageId;
	m_StatusBar->SetStatusText( Message, 0 );
	if( Colour == nullptr )
		m_StatusBar->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE  ));
	else
		m_StatusBar->SetBackgroundColour( *Colour );

	m_StatusBar->Refresh();
	}
/*============================================================================*/
#define LENGTH(a) ( sizeof (a) / sizeof (a)[0] )
/*============================================================================*/
void wxThumbDialog::IdleLoop( wxIdleEvent& event )
	{
	static const char		*Messages[]	= { "", "Halted", "Code Execution Breakpoint Reached", "Memory Read Breakpoint Reached", "Memory Write Breakpoint Reached", "Unaligned Memory Access", "Undefined Memory Address Access", "Invalid Instruction OpCode", "Undefined Instruction OpCode", "Invalid Processor Mode" };
	static const wxColour	*Colours[]	= { nullptr, nullptr, wxCYAN, wxCYAN, wxCYAN, wxRED, wxRED, wxRED, wxRED, wxRED };
	int						Status;

//	if( App->SimulatorThread->IsBusy() )
//		return;

	wxMutexLocker	Locker( App->SimulatorMutex );

	RefreshLCD();

	if( App->SimulatorThread->GetSimulatorUpdated() )
		{
		uint32_t	PC;

		App->SimulatorThread->GetRegisterValue( 15, PC );

		if( PC !=  m_LastPC )
			{
			m_PanelDisassembly->ShowDisassembly( &wxGetApp().CPUState );
			m_LastPC	= PC;
			}

		m_PanelRegisters->RedrawRegisters();

		m_PanelMemory->DumpRedraw();

		if(( Status = App->SimulatorThread->GetSimulatorStatus() ) > 0 && Status < (int)LENGTH( Messages ))
			{
			ShowMessage( Status, Messages[Status], Colours[Status] );
			App->SimulatorThread->ClearSimulatorStatus();
			}

		App->SimulatorThread->ClearSimulatorUpdated();
		}

	//event.RequestMore( true );
	}
/*============================================================================*/
void wxThumbDialog::RedrawAll()
	{
	m_PanelDisassembly->ShowDisassembly( &wxGetApp().CPUState );
	m_PanelRegisters->RedrawRegisters();
	m_PanelMemory->DumpRedraw();
	}
/*============================================================================*/
void wxThumbDialog::OnClose( wxCloseEvent &event )
	{
	Destroy();
	}
/*============================================================================*/
void wxThumbDialog::OnQuit( wxCommandEvent &event )
	{
    Destroy();
	}
/*============================================================================*/
void wxThumbDialog::OnAbout( wxCommandEvent &event )
	{
    wxMessageBox( _( "Simulator for a fictitious ARM computer\n"
					 "Created by Isaac Marino Bavaresco\n" "\n"
					 "Based on a previous simulator for another fictitious computer 'Cesar'"
					 "<https://www.inf.ufrgs.br/arq/wiki/doku.php?id=cesar>\n" "\n"
					 "Written using Code::Blocks 20.03 and wxFormBuilder v4.2.1\n"
					 "Compiled with MinGW64 v13.1.0\n"
					 "using the library wxWidgets v3.2.5" ), _( "wxSimCM0" ));
	}
/*============================================================================*/
void wxThumbDialog::New( wxCommandEvent& event )
	{
	wxGetApp().SimulatorThread->New();

	sim_memoryarea_t	*ROM	= wxGetApp().CPUState.MemoryAreas[0];
	sim_memoryarea_t	*RAM	= wxGetApp().CPUState.MemoryAreas[1];

	m_PanelMemory->SetROMBuffer( ROM->BaseAddress, ROM->LastAddress - ROM->BaseAddress + 1, true, false );
	m_PanelMemory->SetRAMBuffer( RAM->BaseAddress, RAM->LastAddress - RAM->BaseAddress + 1, true, true );

	m_PanelDisassembly->SetAreas( ROM, RAM );

	ShowMessage( 0, "", nullptr );

	App->SimulatorThread->Reset( true );

	SetTitle( Title );

	RedrawAll();
	}
/*============================================================================*/
void wxThumbDialog::LoadELFFile( wxCommandEvent& event )
	{
	int	Result;

	wxFileDialog openFileDialog( this, _("Load ELF file"), "", "", "ELF files (*.elf)|*.elf", wxFD_OPEN | wxFD_FILE_MUST_EXIST );

	if( openFileDialog.ShowModal() == wxID_CANCEL )
		return;

	FileName	= openFileDialog.GetPath();

	if(( Result = App->SimulatorThread->LoadFile( FileName )) < 0 )
		{
		wxString	Message = GetErrorMessage( Result );
		wxMessageBox( wxT( "It was not possible to load file " ) + FileName + "\n\n" + Message, wxT( "Error" ), wxICON_ERROR );

		return;
		}

	sim_memoryarea_t	*ROM	= wxGetApp().CPUState.MemoryAreas[0];
	sim_memoryarea_t	*RAM	= wxGetApp().CPUState.MemoryAreas[1];

	m_PanelMemory->SetROMBuffer( ROM->BaseAddress, ROM->LastAddress - ROM->BaseAddress + 1, true, false );
	m_PanelMemory->SetRAMBuffer( RAM->BaseAddress, RAM->LastAddress - RAM->BaseAddress + 1, true, true );

	m_PanelDisassembly->SetAreas( ROM, RAM );

	ShowMessage( 0, "", nullptr );

	App->SimulatorThread->Reset( true );

	SetTitle( Title + " - " + FileName );

	RedrawAll();
	}
/*============================================================================*/
void wxThumbDialog::OnStepInto( wxCommandEvent& event )
	{
	ShowMessage( 0, "", nullptr );

	m_LastPC	= -1;

	wxGetApp().SimulatorThread->StepInto();
	}
/*============================================================================*/
void wxThumbDialog::OnStepOver( wxCommandEvent& event )
	{
	ShowMessage( 0, "", nullptr );

	m_LastPC	= -1;

	wxGetApp().SimulatorThread->StepOver();
	}
/*============================================================================*/
void wxThumbDialog::OnAnimate( wxCommandEvent& event )
	{
	}
/*============================================================================*/
void wxThumbDialog::OnRun( wxCommandEvent& event )
	{
	ShowMessage( 0, "", nullptr );

	m_LastPC	= -1;

	wxGetApp().SimulatorThread->Go();
	}
/*============================================================================*/
void wxThumbDialog::OnMaxSpeed( wxCommandEvent& event )
	{
	}
/*============================================================================*/
void wxThumbDialog::OnPause( wxCommandEvent& event )
	{
	wxGetApp().SimulatorThread->Pause();
	}
/*============================================================================*/
void wxThumbDialog::OnFullReset( wxCommandEvent& event )
	{
	ShowMessage( 0, "", nullptr );

	m_LastPC	= -1;

	wxGetApp().SimulatorThread->Reset( true );
	RedrawAll();
	}
/*============================================================================*/
void wxThumbDialog::OnResetCPU( wxCommandEvent& event )
	{
	ShowMessage( 0, "", nullptr );

	m_LastPC	= -1;

	wxGetApp().SimulatorThread->Reset( false );
	RedrawAll();
	}
/*============================================================================*/
void wxThumbDialog::OnClearTrace( wxCommandEvent& event )
	{
	wxGetApp().SimulatorThread->ClearTrace();
	m_PanelDisassembly->ShowDisassembly( &wxGetApp().CPUState );
	}
/*============================================================================*/
void wxThumbDialog::OnMenuDeleteAllBreakpoints( wxCommandEvent& event )
	{
	wxGetApp().SimulatorThread->DeleteAllBreakpoints();
	m_PanelDisassembly->ShowDisassembly( &wxGetApp().CPUState );
	}
/*============================================================================*/
void wxThumbDialog::OnMenuToggleBreakpoint( wxCommandEvent& event )
	{
	if( wxGetApp().SimulatorThread->IsBusy() )
		return;

	wxGetApp().SimulatorThread->ToggleBreakpoint();
	m_PanelDisassembly->ShowDisassembly( &wxGetApp().CPUState );
	}
/*============================================================================*/
