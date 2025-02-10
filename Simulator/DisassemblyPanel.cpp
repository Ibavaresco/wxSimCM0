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
#include "DisassemblyPanel.h"
//==============================================================================
#include "Bkpt.xpm"
#include "Exec.xpm"
#include "Already.xpm"
#include "AlreadyC.xpm"
#include "AlreadyY.xpm"
#include "Empty.xpm"

#include "Thumb.h"
#include "ThumbDisassembler.h"
#include "Simulator.h"
#include "wxThumbApp.h"
//==============================================================================
wxDECLARE_APP( wxThumbApp );
//==============================================================================
class	DisassemblyBitmap : public wxStaticBitmap
	{
	//--------------------------------------------------------------------------
	public:
	//--------------------------------------------------------------------------
							DisassemblyBitmap	( unsigned				line,
												  wxWindow				*parent,
												  wxWindowID			id,
												  const wxBitmapBundle	&label,
												  const wxPoint			&pos	= wxDefaultPosition,
												  const wxSize			&size	= wxDefaultSize,
												  long					style	= 0,
												  const wxString		&name	= wxASCII_STR( wxStaticBitmapNameStr ))
												  : wxStaticBitmap( parent, id, label, pos, size, style, name ), Line( line ) {};
		virtual				~DisassemblyBitmap	() {};
	//--------------------------------------------------------------------------
		unsigned			GetLine				() { return Line; };
	//--------------------------------------------------------------------------
	protected:
	//--------------------------------------------------------------------------
		unsigned			Line;
	//--------------------------------------------------------------------------
	};
//==============================================================================
class	DisassemblyText : public wxStaticText
	{
	//--------------------------------------------------------------------------
	public:
	//--------------------------------------------------------------------------
							DisassemblyText		( unsigned			line,
												  wxWindow			*parent,
												  wxWindowID		id,
												  const wxString	&label,
												  const wxPoint		&pos	= wxDefaultPosition,
												  const wxSize		&size	= wxDefaultSize,
												  long				style	= 0,
												  const wxString	&name	= wxASCII_STR( wxStaticTextNameStr ))
												  : wxStaticText( parent, id, label, pos, size, style, name ), Line( line ) {};
		virtual				~DisassemblyText	() {};
	//--------------------------------------------------------------------------
		unsigned			GetLine				() { return Line; };
	//--------------------------------------------------------------------------
	protected:
	//--------------------------------------------------------------------------
		unsigned			Line;
	//--------------------------------------------------------------------------
	};
//==============================================================================
DisassemblyPanel::DisassemblyPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name ) : wxPanel( parent, id, pos, size, style, name )
	{
	m_ROM				= nullptr;
	m_RAM				= nullptr;

	m_Bitmaps[0]		= new wxBitmap( Empty_xpm );
	m_Bitmaps[1]		= new wxBitmap( Already_xpm );
	m_Bitmaps[2]		= new wxBitmap( AlreadyC_xpm );
	m_Bitmaps[3]		= new wxBitmap( AlreadyY_xpm );
	m_Bitmaps[4]		= new wxBitmap( Bkpt_xpm );
	m_Bitmaps[5]		= new wxBitmap( Exec_xpm );

	wxBoxSizer					*bSizer1;
	bSizer1						= new wxBoxSizer( wxVERTICAL );

	m_DisassemblyPanelInternal	= new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxFlexGridSizer	*fgSizer1;
	fgSizer1					= new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer1->SetFlexibleDirection( wxHORIZONTAL );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	for( int Line = 0; Line < DISASSEMBLY_LINES_COUNT; Line++ )
		{
		m_Lines[Line].Address	= -1;

		m_Lines[Line].Bitmap1	= new DisassemblyBitmap( Line, m_DisassemblyPanelInternal, wxID_ANY, *m_Bitmaps[0], wxDefaultPosition, wxSize( 13, 16 ), 0 );
//		m_Lines[Line].Bitmap1->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_MENU ));
		m_Lines[Line].Bitmap1->SetToolTip( _("Click here to set or clear a breakpoint") );
		fgSizer1->Add( m_Lines[Line].Bitmap1, 0, /*wxLEFT |*/ wxALIGN_CENTER_VERTICAL | wxALIGN_CENTER_HORIZONTAL, 5 );

		m_Lines[Line].Bitmap2	= new DisassemblyBitmap( Line, m_DisassemblyPanelInternal, wxID_ANY, Line == DISASSEMBLY_LINES_COUNT / 2 ? *m_Bitmaps[5] : *m_Bitmaps[0], wxDefaultPosition, wxSize( 13, 16 ), 0 );
//		m_Lines[Line].Bitmap2->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_MENU ));
		m_Lines[Line].Bitmap2->SetToolTip( _("Click here to set or clear a breakpoint") );
		fgSizer1->Add( m_Lines[Line].Bitmap2, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_CENTER_HORIZONTAL, 5 );

		m_Lines[Line].Text		= new DisassemblyText( Line, m_DisassemblyPanelInternal, wxID_ANY, _("00000000: 0000       MOVS   R1,R2"), wxDefaultPosition, wxSize( -1, 16 ), 0 );
		m_Lines[Line].Text->Wrap( -1 );
		m_Lines[Line].Text->SetToolTip( _("Double-click here to set or clear a breakpoint") );
		m_Lines[Line].Text->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT( "Courier New" )));

		fgSizer1->Add( m_Lines[Line].Text, 1, wxALIGN_CENTER_VERTICAL | wxEXPAND | wxRIGHT, 5 );
		}

	m_DisassemblyPanelInternal->SetSizer( fgSizer1 );
	m_DisassemblyPanelInternal->Layout();
	fgSizer1->Fit( m_DisassemblyPanelInternal );
	bSizer1->Add( m_DisassemblyPanelInternal, 1, wxEXPAND|wxALL, 5 );

	this->SetSizer( bSizer1 );
	this->Layout();

	// Connect Events
	for( int Line = 0; Line < DISASSEMBLY_LINES_COUNT; Line++ )
		{
		m_Lines[Line].Bitmap1->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( DisassemblyPanel::OnLeftDownBkpt ), NULL, this );
		m_Lines[Line].Bitmap2->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( DisassemblyPanel::OnLeftDownBkpt ), NULL, this );
		m_Lines[Line].Text->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( DisassemblyPanel::OnLeftDClick ), NULL, this );
		}
	}
//==============================================================================
DisassemblyPanel::~DisassemblyPanel()
	{
	}
//==============================================================================
static int BackTrack( thumbstate_t *State, sim_address_t BaseAddress, sim_address_t *Address, int *Lines )
	{
	sim_mapentry_t	MapEntry[4];

	// We cannot backtrack anymore...
	if( *Address < BaseAddress + 1 )
		return 0;

	MapEntry[3]	= GetMapEntry( State, *Address - 1 );

	// The current address is odd, or the previous address has a label of its own, so the previous address can only be one byte data.
	if(( *Address & 1 ) != 0 || MapEntry[3].NameOffset != (sim_nameoffset_t)-1 )
		{
		// The previous address has a label...
		if( MapEntry[3].NameOffset != (sim_nameoffset_t)-1 )
			// ...so it will occupy two lines.
			*Lines		   += 2;
		// The previous address does not have a label...
		else
			// ...so it will occupy only one line.
			*Lines		   += 1;
		*Address		   -= 1;
		return 1;
		}

	// We cannot backtrack anymore...
	if( *Address < BaseAddress + 2 )
		return 0;

	MapEntry[2]	= GetMapEntry( State, *Address - 2 );

	// The current address is not a multiple of four, or the previous  two addresses have a label of their own...
	if(( *Address & 2 ) != 0 || MapEntry[2].NameOffset != (sim_nameoffset_t)-1 || MapEntry[2].Type == 0 || MapEntry[2].Type == 2 /*&& MapEntry[3].Type == 2*/ )
		{
		uint16_t	OpCode2;

		// The previous address has a label...
		if( MapEntry[2].NameOffset != (sim_nameoffset_t)-1 )
			// ...so it will occupy two lines.
			*Lines		   += 2;
		// The previous address does not have a label...
		else
			// ...so it will occupy only one line.
			*Lines		   += 1;

		LoadHalfWord( State, *Address - 2, &OpCode2 );

		// The previous instruction is a bl/blx suffix...
		if(( MapEntry[2].Type == 0 || MapEntry[2].Type == 2 ) && ( OpCode2 & 0xe800 ) == 0xe800 )
			{
			uint16_t	OpCode1;
			LoadHalfWord( State, *Address - 2, &OpCode1 );
			// ...and the one before it is a bl/blx prefix...
			if(( OpCode1 & 0xf800 ) == 0xf000 )
				{
				*Lines	   += 1;
				*Address   -= 4;
				return 1;
				}
			}

		// The previous instruction is one that changes the PC, and generates a separator line...
		if(( OpCode2 & 0xfd87 ) == 0x4487 || ( OpCode2 & 0xff07 ) == 0x4700 || ( OpCode2 & 0xff00 ) == 0xbe00 || ( OpCode2 & 0xff00 ) == 0xdf00 || ( OpCode2 & 0xe000 ) == 0xe000 )
			*Lines		   += 1;

		*Address		   -= 2;
		return 1;
		}

	// We cannot backtrack anymore...
	if( *Address < BaseAddress + 4 )
		return 0;

	MapEntry[2]	= GetMapEntry( State, *Address - 2 );
	}
//==============================================================================
#define	LENGTH(a)	( sizeof (a) / sizeof (a)[0] )
//==============================================================================
void DisassemblyPanel::ShowDisassembly( thumbstate_t *State )
	{
	typedef struct
		{
		int		Type;
		ssize_t	Address;
		char	Text[256];
		} line_t;

	uint32_t		OurAddress;
	sim_address_t	BaseAddress, Address;
	int				Skip;
	line_t			Lines[DISASSEMBLY_LINES_COUNT+6];
	int				LineIdx, LineCnt, OurLineIdx, k, FirstLineIdx;

	wxGetApp().SimulatorThread->GetRegisterValue( 15, OurAddress );
	OurAddress	   &= ~(sim_address_t)1;
	GetBaseAddress( &wxGetApp().CPUState, OurAddress, &BaseAddress );
	Address			= OurAddress > BaseAddress + 2 * ( DISASSEMBLY_LINES_COUNT / 2 * 2 ) ? OurAddress - 2 * ( DISASSEMBLY_LINES_COUNT / 2 * 2 ) : BaseAddress;

	// Let's disassembly some lines before our address...
	for( LineIdx = 0, LineCnt = 0; Address < OurAddress; )
		{
		sim_address_t	AddressTemp	= Address;
		int Type				= ThumbDisassembler( State, &Address, Address == OurAddress, Lines[LineIdx].Text, &Skip );
		Lines[LineIdx].Type		= Type;
		Lines[LineIdx].Address	= AddressTemp;
		if( ++LineIdx >= (int)LENGTH( Lines ))
			LineIdx	= 0;
		if( LineCnt < (int)LENGTH( Lines ))
			LineCnt++;
		}

	// Something went wrong...
	if( Address > OurAddress )
		{
		// ...let's restart directly from our address.
		Address	= OurAddress;
		LineCnt	= 0;
		LineIdx	= 0;
		}

	// Let's disassembly more lines to fill half of the panel...
	for( OurLineIdx = -1, k = 0; k < DISASSEMBLY_LINES_COUNT / 2; k++ )
		{
		sim_address_t	AddressTemp	= Address;
		int Type				= ThumbDisassembler( State, &Address, Address == OurAddress, Lines[LineIdx].Text, &Skip );

		if( AddressTemp == OurAddress && Type == DISASM_INSTRUCTION )
			{
			OurLineIdx			= LineIdx;
			k					= 0;
			}

		Lines[LineIdx].Type		= Type;
		Lines[LineIdx].Address	= AddressTemp;
		if( ++LineIdx >= (int)LENGTH( Lines ))
			LineIdx	= 0;
		if( LineCnt < (int)LENGTH( Lines ))
			LineCnt++;
		}

	if(( FirstLineIdx = LineCnt >= DISASSEMBLY_LINES_COUNT ? LineIdx - DISASSEMBLY_LINES_COUNT : LineIdx - LineCnt ) < 0 )
		FirstLineIdx   += LENGTH( Lines );

	/* If for some reason we did not get enough lines to fill the panel,
	   let's empty a number of lines at the top to complete the panel. */
	for( LineIdx = 0, k = LineCnt; k < DISASSEMBLY_LINES_COUNT; LineIdx++, k++ )
		SetLine( LineIdx, DISASM_UNKNOWN, -1, 0, "" );

	for( k = FirstLineIdx; LineIdx < DISASSEMBLY_LINES_COUNT; LineIdx++ )
		{
		SetLine( LineIdx, Lines[k].Type, Lines[k].Address, k == OurLineIdx ? 5 : 0, Lines[k].Text );
		if( ++k >= (int)LENGTH( Lines ))
			k	= 0;
		}
#if 0
	if( GetBaseAddress( State, Address, &BaseAddress ) == RESULT_OK )
		{
		int				Lines;
		sim_mapentry_t	MapEntryTemp;
		volatile sim_mapentry_t	MapEntry[4];

		for( Lines = 0; AddressTemp > BaseAddress + 1 && Lines < DISASSEMBLY_LINES_COUNT / 2; )
			{
			MapEntryTemp			= GetMapEntry( State, AddressTemp - 2 );
			MapEntry[2].Type		= MapEntryTemp.Type;
			MapEntry[2].NameOffset	= MapEntryTemp.NameOffset;
			MapEntryTemp			= GetMapEntry( State, AddressTemp - 1 );
			MapEntry[3].Type		= MapEntryTemp.Type;
			MapEntry[3].NameOffset	= MapEntryTemp.NameOffset;
			// We have one Thumb instruction before.
			if(( AddressTemp & 1 ) == 0 && MapEntry[2].Type == 2 && MapEntry[3].Type == 2 )
				{
				AddressTemp	   -= 2;
				Lines++;
				}
			// We have data before.
			else if(( AddressTemp & 1 ) != 0 || MapEntry[3].Type == 1 )
				{
				if(( AddressTemp & 1 ) != 0 || MapEntry[3].NameOffset != (sim_nameoffset_t)-1 || MapEntry[2].Type != 1 )
					{
					AddressTemp	   -= 1;
					Lines++;
					}
				else
					{
					MapEntryTemp			= GetMapEntry( State, AddressTemp - 4 );
					MapEntry[0].Type		= MapEntryTemp.Type;
					MapEntry[0].NameOffset	= MapEntryTemp.NameOffset;
					MapEntryTemp			= GetMapEntry( State, AddressTemp - 3 );
					MapEntry[1].Type		= MapEntryTemp.Type;
					MapEntry[1].NameOffset	= MapEntryTemp.NameOffset;
					if(( AddressTemp & 2 ) != 0 || MapEntry[2].NameOffset != (sim_nameoffset_t)-1 || MapEntry[0].Type != 1 || MapEntry[1].Type != 1 )
						{
						AddressTemp   -= 2;
						Lines++;
						}
					else
						{
						AddressTemp   -= 4;
						Lines++;
						}
					}
				}
			else if(( AddressTemp & 2 ) != 0 )
				{
				AddressTemp   -= 2;
				Lines++;
				}
			else
				{
				AddressTemp   -= 4;
				Lines++;
				}
			}
		}
#endif

#if 0
	Address	= AddressTemp;

	for( Skip = 0, Line = 0; Line < DISASSEMBLY_LINES_COUNT; Line++ )
		{
		sim_address_t	AddressTemp	= Address ;
		int Type	= ThumbDisassembler( State, &Address, Text, &Skip );
		SetLine( Line, Type, AddressTemp, 0, Text );
		}
#endif
	}
//==============================================================================
void DisassemblyPanel::SetLine( size_t line, int type, ssize_t address, unsigned icon2, const wxString &text )
	{
	if( line >= DISASSEMBLY_LINES_COUNT )
		return;

	sim_mapentry_t	*MapEntry	= GetMapEntry2( &wxGetApp().CPUState, address );

	m_Lines[line].Type			= type;
	m_Lines[line].Address		= address;
	m_Lines[line].Text->SetLabel( text );

	SetIcon1( line, m_Lines[line].Type == DISASM_INSTRUCTION && MapEntry != nullptr && MapEntry->BreakpointExecute ? 4 : 0 );
	SetIcon2( line, icon2 != 0 ? icon2 : m_Lines[line].Type != DISASM_INSTRUCTION || MapEntry == nullptr ? 0 : MapEntry->TraceInfo );
	}
//==============================================================================
void DisassemblyPanel::SetIcon1( size_t line, unsigned icon )
	{
	if( line >= DISASSEMBLY_LINES_COUNT || icon >= DISASSEMBLY_ICONS_COUNT )
		return;

	m_Lines[line].Bitmap1->SetBitmap( *m_Bitmaps[icon] );
	}
//==============================================================================
void DisassemblyPanel::SetIcon2( size_t line, unsigned icon )
	{
	if( line >= DISASSEMBLY_LINES_COUNT || icon >= DISASSEMBLY_ICONS_COUNT )
		return;

	m_Lines[line].Bitmap2->SetBitmap( *m_Bitmaps[icon] );
	}
//==============================================================================
void DisassemblyPanel::ToggleBreakpoint( unsigned line )
	{
	if( line >= DISASSEMBLY_LINES_COUNT )
		return;

	sim_mapentry_t	*MapEntry	= GetMapEntry2( &wxGetApp().CPUState, m_Lines[line].Address );

	if( MapEntry == nullptr || m_Lines[line].Type < DISASM_LABEL || ( m_Lines[line].Type == DISASM_LABEL && line >= DISASSEMBLY_LINES_COUNT - 1 ))
		SetIcon1( line, 0 );
	else
		{
		// The clicked line is a label...
		if( m_Lines[line].Type == DISASM_LABEL )
			{
			// ...we will set the breakpoint on the next line.
			MapEntry->BreakpointExecute ^= -1;
			SetIcon1( line + 1, MapEntry != nullptr && MapEntry->BreakpointExecute != 0 ? 4 : 0 );
			}
		// The clicked line is an instruction...
		else
			{
			// ...we will set the breakpoint on it.
			MapEntry->BreakpointExecute ^= -1;
			SetIcon1( line, MapEntry != nullptr && MapEntry->BreakpointExecute != 0 ? 4 : 0 );
			}
		}
	}
//==============================================================================
void DisassemblyPanel::SetAreas( sim_memoryarea_t *rom, sim_memoryarea_t *ram )
	{
	m_ROM	= rom;
	m_RAM	= ram;
	}
//==============================================================================
void DisassemblyPanel::OnLeftDownBkpt( wxMouseEvent &event )
	{
	DisassemblyBitmap	*BitMap	= static_cast<DisassemblyBitmap*>( event.GetEventObject() );
	unsigned			Line	= BitMap->GetLine();

	ToggleBreakpoint( Line );
	}
//==============================================================================
void DisassemblyPanel::OnLeftDClick( wxMouseEvent& event )
	{
	DisassemblyText		*Text	= static_cast<DisassemblyText*>( event.GetEventObject() );
	unsigned			Line	= Text->GetLine();

	ToggleBreakpoint( Line );
	}
//==============================================================================
