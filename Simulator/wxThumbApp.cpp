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
#include "wxThumbApp.h"
#include "wxThumbMain.h"
#include "ThumbThread.h"
/*============================================================================*/
IMPLEMENT_APP( wxThumbApp );
/*============================================================================*/
bool wxThumbApp::OnInit()
	{
	CPUState.NumMemoryAreas		= 0;

	memset( &CPUState, 0x00, sizeof CPUState );

	ShuttingDown			= false;
	SimulatorThread			= new ThumbThread( &CPUState );

	if( SimulatorThread->Create() != wxTHREAD_NO_ERROR || SimulatorThread->Run() != wxTHREAD_NO_ERROR )
		{
		delete SimulatorThread;
		SimulatorThread		= nullptr;

	    wxMessageBox( _( "Could not create the simulator thread." ), _( "Error" ));

		return false;
		}

    wxThumbDialog	*dlg	= new wxThumbDialog( 0L );
    dlg->SetIcon( wxICON( aaaa ));
    dlg->Show();

    return true;
	}
/*============================================================================*/
int wxThumbApp::OnExit()
	{
	do
		{
		wxMutexLocker Locker( SimulatorMutex );
		ShuttingDown		= true;
		}
	while( 0 );

	SemaphoreAllDone.Wait();

	ThumbFinalize( &CPUState );

	return 0;
	}
/*============================================================================*/
#if 0
wxThumbApp::wxThumbApp() : wxApp()
	{
	CPUState			= nullptr;
	ShuttingDown		= false;
	SimulatorThread		= new SimulatorThread( &CPUState );

	if( SimulatorThread->Create() != wxTHREAD_NO_ERROR || SimulatorThread->Run() != wxTHREAD_NO_ERROR )
		{
		delete SimulatorThread;
		SimulatorThread	= nullptr;
		}
	}
/*============================================================================*/
wxThumbApp::~wxThumbApp()
	{
//	wxMutexLocker Locker( SimulatorMutex );
	ShuttingDown		= true;
	SemaphoreAllDone.Wait();

	ThumbDelete( CPUState );
	CPUState			= nullptr;
	};
#endif
/*============================================================================*/
