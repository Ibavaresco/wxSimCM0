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
#include <wx/menu.h>
#include "MemoryDumpPanel.h"
#include "wxThumbApp.h"
//==============================================================================
#define	HEADER_HEIGHT	16
#define	ROW_HEIGHT		16
//==============================================================================
wxDECLARE_APP( wxThumbApp );
/*============================================================================*/
class	DumpEditCtrl: public wxTextCtrl
	{
	//--------------------------------------------------------------------------
	public:
	//--------------------------------------------------------------------------
							DumpEditCtrl		( unsigned			side,
												  unsigned			line,
												  unsigned			column,
												  unsigned			columnwidth,
												  MemoryDumpPanel	*parent,
												  wxWindowID		id,
												  const wxString	&value		= wxEmptyString,
												  const wxPoint		&pos		= wxDefaultPosition,
												  const wxSize		&size		= wxDefaultSize,
												  long				style		= 0,
												  const wxValidator	&validator	= wxDefaultValidator,
												  const wxString	&name		= wxASCII_STR( wxTextCtrlNameStr ))
												: wxTextCtrl( parent, id, value, pos, size, style, validator, name ),
												  m_Parent( parent ), m_Side( side ), m_Line( line ), m_Column( column ),
												  m_ColumnWidth( columnwidth ), m_BkColour( nullptr ) {};

		virtual				~DumpEditCtrl		() {};
	//--------------------------------------------------------------------------
		virtual void		OnChar				( wxKeyEvent &event ) = 0;
		virtual void		OnKillFocus			( wxFocusEvent &event );
		virtual void		OnLeftDown			( wxMouseEvent &event );
		virtual void		OnSetFocus			( wxFocusEvent &event );
		virtual void		DummyMouseEvent		( wxMouseEvent& event ) {};
		virtual void		OnRightDown			( wxMouseEvent& event );
	//--------------------------------------------------------------------------
		unsigned			GetLine				() { return m_Line; };
		virtual unsigned	GetColumn			() { return m_Column * m_ColumnWidth; };
	//--------------------------------------------------------------------------
		virtual bool		StartEditing		() { return !wxGetApp().SimulatorThread->IsBusy(); };
		virtual bool		CancelEditing		() { return true; };
		virtual bool		FinishEditing		() { return true; };
	//--------------------------------------------------------------------------
	protected:
	//--------------------------------------------------------------------------
		MemoryDumpPanel		*m_Parent;
		unsigned			m_Side, m_Line, m_Column;
		unsigned			m_ColumnWidth;
	//--------------------------------------------------------------------------
	public:
	//--------------------------------------------------------------------------
		const wxColour		*m_BkColour;
	//--------------------------------------------------------------------------
	};
//==============================================================================
void DumpEditCtrl::OnLeftDown( wxMouseEvent &event )
	{
	wxWindow	*FocusOwner	= FindFocus();

	if( FocusOwner == event.GetEventObject() )
		return;

	if( dynamic_cast<DumpEditCtrl*>( FocusOwner ) != nullptr )
		m_Parent->ClearCellBackGround();

	m_Parent->SetCurrentCell( m_Side, m_Line, m_Column * m_ColumnWidth );
	m_Parent->SetCellFocus();
	}
//==============================================================================
#define	LENGTH(a)	( sizeof (a) /sizeof (a)[0] )
//==============================================================================
void DumpEditCtrl::OnRightDown( wxMouseEvent& event )
	{
	sim_address_t	Address		= m_Parent->GetCellAddress( m_Side, m_Line, m_Column );

	int				Selected	= wxGetApp().SimulatorThread->GetDataBreakpoint( Address );

	if( Selected < 0 )
		return;

	OnLeftDown( event );

	wxString	Text;
	Text.Printf( "Data Breakpoint At 0x%08X", Address );
    wxMenu menu( Text );

    wxMenuItem	*Items[4];

    Items[0]	= menu.AppendCheckItem( 0, wxT( "Off" ), wxT( "" ));
    Items[1]	= menu.AppendCheckItem( 1, wxT( "Read" ), wxT( "" ));
    Items[2]	= menu.AppendCheckItem( 2, wxT( "Write" ), wxT( "" ));
    Items[3]	= menu.AppendCheckItem( 3, wxT( "Read/Write" ), wxT( "" ));

    menu.Check( Selected, true );

    PopupMenu( &menu, event.GetPosition() );

    for( int i = 0; i < (int)LENGTH( Items ); i++ )
		if( i != Selected && Items[i]->IsChecked() )
			{
			Selected	= i;
			break;
			}

	wxGetApp().SimulatorThread->SetDataBreakpoint( Address, Selected );
    }
//==============================================================================
void DumpEditCtrl::OnSetFocus( wxFocusEvent &event )
	{
	if( m_Parent->GetCurrentSide() != m_Side || m_Parent->GetCurrentLine() != m_Line || m_Parent->GetCurrentColumn() / m_ColumnWidth != m_Column )
		{
		m_Parent->SetCellFocus();
		return;
		}

	m_Parent->SetActive( true );
	m_Parent->SelectCell( m_Side, m_Line, m_Column * m_ColumnWidth, false );

	m_Parent->SetCellBackGround();
	}
//==============================================================================
void DumpEditCtrl::OnKillFocus( wxFocusEvent &event )
	{
	FinishEditing();
	m_Parent->SetActive( false );
	m_Parent->ClearCellBackGround();
	}
//==============================================================================
class	HexEdit: public DumpEditCtrl
	{
	//--------------------------------------------------------------------------
	public:
	//--------------------------------------------------------------------------
							HexEdit				( unsigned			side,
												  unsigned			line,
												  unsigned			column,
												  unsigned			columnwidth,
												  MemoryDumpPanel	*parent,
												  wxWindowID		id,
												  const wxString	&value		= wxEmptyString,
												  const wxPoint		&pos		= wxDefaultPosition,
												  const wxSize		&size		= wxDefaultSize,
												  long				style		= 0,
												  const wxValidator	&validator	= wxDefaultValidator,
												  const wxString	&name		= wxASCII_STR( wxTextCtrlNameStr ))
												: DumpEditCtrl( side, line, column, columnwidth, parent, id, value, pos, size, style, validator, name ),
												  m_Editing( false ) {};

		virtual				~HexEdit			() {};
	//--------------------------------------------------------------------------
		virtual void		OnChar				( wxKeyEvent &event );
	//--------------------------------------------------------------------------
	protected:
		virtual bool		StartEditing		() override;
		virtual bool		CancelEditing		() override;
		virtual bool		FinishEditing		() override;
	//--------------------------------------------------------------------------
		bool				m_Editing;
		wxString			m_OriginalValue;
	//--------------------------------------------------------------------------
	};
//==============================================================================
class	CharEdit: public DumpEditCtrl
	{
	//--------------------------------------------------------------------------
	public:
	//--------------------------------------------------------------------------
							CharEdit			( unsigned			side,
												  unsigned			line,
												  unsigned			column,
												  MemoryDumpPanel	*parent,
												  wxWindowID		id,
												  const wxString	&value		= wxEmptyString,
												  const wxPoint		&pos		= wxDefaultPosition,
												  const wxSize		&size		= wxDefaultSize,
												  long				style		= 0,
												  const wxValidator	&validator	= wxDefaultValidator,
												  const wxString	&name		= wxASCII_STR( wxTextCtrlNameStr ))
												: DumpEditCtrl( side, line, column, 1, parent, id, value, pos, size, style, validator, name ) {};

		virtual				~CharEdit			() {};
	//--------------------------------------------------------------------------
		virtual void		OnChar				( wxKeyEvent &event );
	//--------------------------------------------------------------------------
	};
//==============================================================================
bool HexEdit::StartEditing()
	{
	if( wxGetApp().SimulatorThread->IsBusy() || !m_Parent->IsEditable() || m_Editing )
		return false;

	m_OriginalValue	= GetValue();
	m_Editing		= true;

	return true;
	}
//==============================================================================
bool HexEdit::CancelEditing()
	{
	if( !m_Editing )
		return false;

	SetValue( m_OriginalValue );
	Refresh();
	m_Editing	= false;

	return m_Parent->IsEditable();
	}
//==============================================================================
bool HexEdit::FinishEditing()
	{
	if( !m_Editing )
		return false;

	if( !m_Parent->IsEditable() || GetValue().Length() == 0  )
		{
		SetValue( m_OriginalValue );
		Refresh();
		m_Editing	= false;
		return false;
		}

	unsigned long int	Value;
	wxString	Text	= GetValue();
	sscanf( Text, "%lx", &Value );
	m_Parent->SetCellValueHex( m_Line, m_Column, Value );
	m_Editing	= false;

	return true;
	}
//==============================================================================
void HexEdit::OnChar( wxKeyEvent& event )
	{
	int			KeyCode		= event.GetKeyCode();

	if( !isprint( KeyCode ))
		{
		int	Modifiers	= event.GetModifiers();

		switch( KeyCode )
			{
			case WXK_HOME:
				if( Modifiers & wxMOD_CONTROL )
					m_Parent->ControlHome( this );
				else
					m_Parent->Home( this );
				return;
			case WXK_PAGEUP:
				m_Parent->PageUp( this );
				return;
			case WXK_PAGEDOWN:
				m_Parent->PageDown( this );
				return;
			case WXK_UP:
				m_Parent->MoveUp( this );
				return;
			case WXK_DOWN:
				m_Parent->MoveDown( this );
				return;
			case WXK_LEFT:
				m_Parent->MoveLeft( this, m_ColumnWidth );
				return;
			case WXK_RIGHT:
				m_Parent->MoveRight( this, m_ColumnWidth );
				return;
			case WXK_END:
				if( Modifiers & wxMOD_CONTROL )
					m_Parent->ControlEnd( this, m_ColumnWidth );
				else
					m_Parent->End( this, m_ColumnWidth );
				return;
			case WXK_RETURN:
				if( !m_Editing || FinishEditing() )
					m_Parent->MoveRight( this, m_ColumnWidth );
				return;
			case WXK_DELETE:
				if( wxGetApp().SimulatorThread->IsBusy() )
					return;
				if( m_Parent->IsHexCellValid( m_Line, m_Column ) && StartEditing() )
					{
					SetValue( "" );
					Refresh();
					}
				return;
			case WXK_BACK:
				{
				if( wxGetApp().SimulatorThread->IsBusy() )
					return;
				if( !m_Parent->IsEditable() || !m_Parent->IsHexCellValid( m_Line, m_Column ))
					return;
				StartEditing();
				wxString	Text	= GetValue();
				Text.Truncate( Text.Length() - 1 );
				SetValue( Text );
				Refresh();
				return;
				}
			case WXK_ESCAPE:
				CancelEditing();
				return;
			case WXK_TAB:
				FinishEditing();
				if( !( Modifiers & wxMOD_SHIFT ))
					{
					m_Parent->SwitchSide();
					return;
					}
				else
					m_Parent->ClearCellBackGround();
			/* Fall-through. */
			default:
				m_Parent->OnChar( event );
				return;
			}
		}

	if( wxGetApp().SimulatorThread->IsBusy() )
		return;
	if( !isxdigit( KeyCode ) || !m_Parent->IsEditable() || !m_Parent->IsHexCellValid( m_Line, m_Column ))
		return;

	if( StartEditing() )
		{
		wxString		Text;
		Text.Printf( "%c", toupper( KeyCode ));
		SetValue( Text );
		Refresh();
		}
	else
		{
		wxString	Text	= GetValue();
		Text.Append( (wchar_t)toupper( KeyCode ));
		SetValue( Text );
		Refresh();
		if( Text.Length() >= 2 * m_ColumnWidth && FinishEditing() )
			{
			m_Parent->MoveRight( this, m_ColumnWidth );
			return;
			}
		}
	SetInsertionPoint( -1 );
	}
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
void CharEdit::OnChar( wxKeyEvent &event )
	{
	int			KeyCode		= event.GetKeyCode();

	if( !isprint( KeyCode ))
		{
		int	Modifiers	= event.GetModifiers();

		switch( KeyCode )
			{
			case WXK_HOME:
				if( Modifiers & wxMOD_CONTROL )
					m_Parent->ControlHome( this );
				else
					m_Parent->Home( this );
				return;
			case WXK_PAGEUP:
				m_Parent->PageUp( this );
				return;
			case WXK_PAGEDOWN:
				m_Parent->PageDown( this );
				return;
			case WXK_UP:
				m_Parent->MoveUp( this );
				return;
			case WXK_DOWN:
				m_Parent->MoveDown( this );
				return;
			case WXK_LEFT:
				m_Parent->MoveLeft( this, 1 );
				return;
			case WXK_RIGHT:
				m_Parent->MoveRight( this, 1 );
				return;
			case WXK_END:
				if( Modifiers & wxMOD_CONTROL )
					m_Parent->ControlEnd( this, 1 );
				else
					m_Parent->End( this, 1 );
				return;
			case WXK_RETURN:
				m_Parent->MoveRight( this, 1 );
				return;
			case WXK_DELETE:
				return;
			case WXK_BACK:
				return;
			case WXK_ESCAPE:
				return;
			case WXK_TAB:
				FinishEditing();
				if( Modifiers & wxMOD_SHIFT )
					{
					m_Parent->SwitchSide();
					return;
					}
				else
					m_Parent->ClearCellBackGround();
			/* Fall-through. */
			default:
				m_Parent->OnChar( event );
				return;
			}
		}

	if( wxGetApp().SimulatorThread->IsBusy() )
		return;

	if( !m_Parent->IsEditable() || !m_Parent->IsCharCellValid( m_Line, m_Column ))
		return;

	m_Parent->SetCellValueChar( m_Line, m_Column, KeyCode );
	m_Parent->MoveRight( this, 1 );
	}
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
MemoryDumpPanel::MemoryDumpPanel( unsigned addressbits, unsigned widthbytes, unsigned columnwidth, wchar_t placeholder, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name ) : wxPanel( parent, id, pos, size, style, name )
	{
//	static const char	*ColumnHeaderTexts[]	= { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F" };

	m_PanelChangedCallBack	= nullptr;

	m_WidthBytes			= widthbytes;
	m_ColumnWidth			= columnwidth;
	m_BigEndian				= false;
	m_AddressNibbles		= ( addressbits + 3 ) >> 2;
	m_Placeholder			= placeholder;
	m_Editable				= false;
	m_StartAddress			=  0;
	m_BufferBaseAddress		=  0;
	m_FirstPageStartAddress	=  0;
	m_LastPageStartAddress	=  0;
	m_LinesCount			= MEMORY_DUMP_LINES;
	m_ColumnsCount			=  m_WidthBytes / m_ColumnWidth;
	m_CurrentSide			=  0;
	m_CurrentLine			=  0;
	m_CurrentColumn			=  0;
	m_IsActive				= false;

	this->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNHIGHLIGHT ));

	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxHORIZONTAL );

	wxFlexGridSizer* fgSizer3;
	fgSizer3 = new wxFlexGridSizer( 0, 1 + m_ColumnsCount + 1 + m_WidthBytes, 0, 0 );
	fgSizer3->SetFlexibleDirection( wxBOTH );
	fgSizer3->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_Header.Address = new wxStaticText( this, wxID_ANY, _("Address"), wxDefaultPosition, wxSize( 69, HEADER_HEIGHT ), wxALIGN_CENTER_HORIZONTAL );
	m_Header.Address->Wrap( -1 );
	m_Header.Address->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ) );
	fgSizer3->Add( m_Header.Address, 0, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL /*| wxBOTTOM*/, 5 );

	for( unsigned Column = 0; Column < m_ColumnsCount; Column++ )
		{
		wxString	Text;
		if( m_ColumnWidth == 1 )
			Text.Printf( m_WidthBytes <= 16 ? "%X" : "%02X", Column );
		else if( m_BigEndian )
			Text.Printf( m_WidthBytes <= 16 ? "%X~%0X" : "%02X~%02X", Column * m_ColumnWidth, ( Column + 1 ) * m_ColumnWidth - 1 );
		else
			Text.Printf( m_WidthBytes <= 16 ? "%X~%0X" : "%02X~%02X", ( Column + 1 ) * m_ColumnWidth - 1, Column * m_ColumnWidth );
		m_Header.Hexadecimal[Column] = new wxStaticText( this, wxID_ANY, Text, wxDefaultPosition, wxSize( 16 * m_ColumnWidth, HEADER_HEIGHT ), wxALIGN_CENTER_HORIZONTAL );
		m_Header.Hexadecimal[Column]->Wrap( -1 );
		m_Header.Hexadecimal[Column]->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Courier New")) );
		m_Header.Hexadecimal[Column]->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ) );
		fgSizer3->Add( m_Header.Hexadecimal[Column], 0, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL /*| wxBOTTOM*/, 5 );
		}

	m_Header.Spacer = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 8, HEADER_HEIGHT ), wxALIGN_CENTER_HORIZONTAL );
	m_Header.Spacer->Wrap( -1 );
	m_Header.Spacer->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ) );
	fgSizer3->Add( m_Header.Spacer, 0, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL /*| wxBOTTOM*/, 5 );

	for( unsigned Column = 0; Column < m_WidthBytes; Column++ )
		{
		wxString	Text;
		if( m_ColumnsCount <= 16 )
			Text.Printf( "%X", Column );
		else
			Text	= wxEmptyString;
		m_Header.Characters[Column] = new wxStaticText( this, wxID_ANY, Text, wxDefaultPosition, wxSize( 8, HEADER_HEIGHT ), wxALIGN_CENTER_HORIZONTAL );
		m_Header.Characters[Column]->Wrap( -1 );
		m_Header.Characters[Column]->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Courier New")) );
		m_Header.Characters[Column]->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ) );

		fgSizer3->Add( m_Header.Characters[Column], 0, 0 /*| wxBOTTOM*/, 5 );
		}

	for( unsigned Line = 0; Line < m_LinesCount; Line++ )
		{
		wxString	Text;
		if( m_Placeholder == 0 )
			Text.Printf( "%0*X:", m_AddressNibbles, Line * m_WidthBytes );
		else
			Text.Printf( "%0*X%c:", m_AddressNibbles - 1, ( Line * m_WidthBytes ) >> 4, m_Placeholder );
		m_Lines[Line].Address = new wxStaticText( this, wxID_ANY, Text, wxDefaultPosition, wxSize( -1, ROW_HEIGHT ), wxALIGN_CENTER_HORIZONTAL );
		m_Lines[Line].Address->Wrap( -1 );
		m_Lines[Line].Address->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ) );
		m_Lines[Line].Address->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Courier New")) );

		fgSizer3->Add( m_Lines[Line].Address, 0, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL | wxRIGHT, 5 );

		Text.Printf( "%.*s", 2 * m_ColumnWidth, "????????????????" );
		for( unsigned Column = 0; Column < m_ColumnsCount; Column++ )
			{
			m_Lines[Line].Hexadecimal[Column] = new HexEdit( 0, Line, Column, m_ColumnWidth, this, wxID_ANY, Text, wxDefaultPosition, wxSize( 14 * m_ColumnWidth + 2, ROW_HEIGHT ), wxTE_PROCESS_ENTER | wxTE_RIGHT | wxBORDER_NONE );
#ifdef __WXGTK__
			if ( !m_Lines[Line].Hexadecimal[Column]->HasFlag( wxTE_MULTILINE ))
#endif
				m_Lines[Line].Hexadecimal[Column]->SetMaxLength( 2 * m_ColumnWidth );
			m_Lines[Line].Hexadecimal[Column]->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Courier New")) );

			fgSizer3->Add( m_Lines[Line].Hexadecimal[Column], 0, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5 );

			m_Lines[Line].Hexadecimal[Column]->Connect( wxEVT_CHAR, wxKeyEventHandler( HexEdit::OnChar ));
			m_Lines[Line].Hexadecimal[Column]->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler( DumpEditCtrl::OnKillFocus ));
			m_Lines[Line].Hexadecimal[Column]->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( DumpEditCtrl::OnLeftDown ));
			m_Lines[Line].Hexadecimal[Column]->Connect( wxEVT_SET_FOCUS, wxFocusEventHandler( DumpEditCtrl::OnSetFocus ));
			m_Lines[Line].Hexadecimal[Column]->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( DumpEditCtrl::DummyMouseEvent ));
			m_Lines[Line].Hexadecimal[Column]->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( DumpEditCtrl::OnRightDown ));
			}

		m_Lines[Line].Spacer = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 8, ROW_HEIGHT ), wxALIGN_CENTER_HORIZONTAL );
		m_Lines[Line].Spacer->Wrap( -1 );

		fgSizer3->Add( m_Lines[Line].Spacer, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

		for( unsigned Column = 0; Column < m_WidthBytes; Column++ )
			{
			m_Lines[Line].Characters[Column] = new CharEdit( 1, Line, Column, this, wxID_ANY, _("?"), wxDefaultPosition, wxSize( 8, ROW_HEIGHT ), wxTE_PROCESS_ENTER | wxTE_RIGHT | wxBORDER_NONE );
#ifdef __WXGTK__
			if ( !m_Lines[Line].Characters[Column]->HasFlag( wxTE_MULTILINE ) )
#endif
				m_Lines[Line].Characters[Column]->SetMaxLength( 1 );
			m_Lines[Line].Characters[Column]->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Courier New") ) );

			fgSizer3->Add( m_Lines[Line].Characters[Column], 0, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5 );

			m_Lines[Line].Characters[Column]->Connect( wxEVT_CHAR, wxKeyEventHandler( CharEdit::OnChar ));
			m_Lines[Line].Characters[Column]->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler( DumpEditCtrl::OnKillFocus ));
			m_Lines[Line].Characters[Column]->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( DumpEditCtrl::OnLeftDown ));
			m_Lines[Line].Characters[Column]->Connect( wxEVT_SET_FOCUS, wxFocusEventHandler( DumpEditCtrl::OnSetFocus ));
			m_Lines[Line].Characters[Column]->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( DumpEditCtrl::DummyMouseEvent ));
			m_Lines[Line].Characters[Column]->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( DumpEditCtrl::OnRightDown ));
			}
		}

	bSizer3->Add( fgSizer3, 1, wxEXPAND, 5 );

	m_ScrollBar	= new wxScrollBar( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL );
	m_ScrollBar->SetPageSize( m_LinesCount - 1 );
	m_ScrollBar->SetThumbSize( m_LinesCount );
	m_ScrollBar->SetRange(( m_LastPageStartAddress - m_FirstPageStartAddress ) / m_ColumnsCount );
	m_ScrollBar->Disable();
	bSizer3->Add( m_ScrollBar, 0, wxLEFT | wxEXPAND, 5 );

	this->SetSizer( bSizer3 );
	this->Layout();

	m_CurrentControl		= m_CurrentSide == 0 ? static_cast<DumpEditCtrl*>( m_Lines[m_CurrentLine].Hexadecimal[m_CurrentColumn/m_ColumnWidth] ) : static_cast<DumpEditCtrl*>( m_Lines[m_CurrentLine].Characters[m_CurrentColumn] );

	this->Connect( wxEVT_MOUSEWHEEL, wxMouseEventHandler( MemoryDumpPanel::OnMouseWheel ));
	m_ScrollBar->Connect( wxEVT_SET_FOCUS, wxFocusEventHandler( MemoryDumpPanel::OnSetFocusScrollbar ), NULL, this );
	m_ScrollBar->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( MemoryDumpPanel::OnScrollChanged ), NULL, this );
	m_ScrollBar->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( MemoryDumpPanel::OnScrollChanged ), NULL, this );
	m_ScrollBar->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( MemoryDumpPanel::OnScrollChanged ), NULL, this );
	m_ScrollBar->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( MemoryDumpPanel::OnScrollChanged ), NULL, this );
	m_ScrollBar->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( MemoryDumpPanel::OnScrollChanged ), NULL, this );
	m_ScrollBar->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( MemoryDumpPanel::OnScrollChanged ), NULL, this );
	}
//==============================================================================
void MemoryDumpPanel::GetState( panelstate_t &state )
	{
	state.Side			= m_CurrentSide;
	state.Line			= m_CurrentLine;
	state.Column		= m_CurrentColumn;
	state.BigEndian		= m_BigEndian;
	state.StartAddress	= m_StartAddress;
	}
//==============================================================================
void MemoryDumpPanel::SetState( const panelstate_t &state )
	{
	ChangeStartAddress( state.StartAddress );
	SetEndianness( state.BigEndian );
	SelectCell( state.Side, state.Line, state.Column, true );
	}
//==============================================================================
void MemoryDumpPanel::SetPanelChangedCallBack( void (*callback)( uint32_t, uint32_t ) )
	{
	m_PanelChangedCallBack	= callback;
	}
//==============================================================================
bool MemoryDumpPanel::ChangeStartAddress( uint32_t newaddress )
	{
	if( newaddress == m_StartAddress || newaddress < m_FirstPageStartAddress || newaddress > m_LastPageStartAddress )
		return false;

	m_StartAddress	= newaddress;
	m_ScrollBar->SetThumbPosition(( m_StartAddress - m_FirstPageStartAddress ) / m_WidthBytes );

	DumpRedraw();

	if( m_PanelChangedCallBack != nullptr )
		m_PanelChangedCallBack( m_StartAddress, m_StartAddress + m_BufferLength - 1 );

	return true;
	}
//==============================================================================
bool MemoryDumpPanel::CenterAddress( uint32_t address )
	{
	if( address < m_FirstPageStartAddress || address >= m_BufferBaseAddress + m_BufferLength )
		return false;

	address			= address / m_WidthBytes * m_WidthBytes;

	if( address < m_FirstPageStartAddress + m_LinesCount / 2 * m_WidthBytes )
		address		= m_FirstPageStartAddress;
	else
		address	   -= m_LinesCount / 2 * m_WidthBytes;

	ChangeStartAddress( address );

	return true;
	}
//==============================================================================
bool MemoryDumpPanel::IsHexCellValid( unsigned line, unsigned column )
	{
	if( line >= m_LinesCount || column >= m_ColumnsCount )
		return false;

	uint32_t	Address	= m_StartAddress + line * m_WidthBytes + column * m_ColumnWidth;

	return Address >= m_BufferBaseAddress && Address + m_ColumnWidth <= m_BufferBaseAddress + m_BufferLength;
	}
//==============================================================================
bool MemoryDumpPanel::IsCharCellValid( unsigned line, unsigned column )
	{
	if( line >= m_LinesCount || column >= m_WidthBytes )
		return false;

	uint32_t	Address	= m_StartAddress + line * m_WidthBytes + column;

	return Address >= m_BufferBaseAddress && Address < m_BufferBaseAddress + m_BufferLength;
	}
//==============================================================================
void MemoryDumpPanel::OnChar( wxKeyEvent &event )
	{
	int			KeyCode		= event.GetKeyCode();

	if( !isprint( KeyCode ))
		{
		int	Modifiers	= event.GetModifiers();

		switch( KeyCode )
			{
			case WXK_TAB:
				Navigate( ( Modifiers & wxMOD_SHIFT ) ? wxNavigationKeyEvent::IsBackward : wxNavigationKeyEvent::IsForward );
				return;
			}
		}
	event.Skip();
	}
//==============================================================================
void MemoryDumpPanel::OnMouseWheel( wxMouseEvent& event )
	{
	int		Rotation	= event.GetWheelRotation();

	if( Rotation > 0 )
		{
		if( m_StartAddress > m_FirstPageStartAddress )
			{
			m_CurrentControl->FinishEditing();
			ChangeStartAddress( m_StartAddress - m_WidthBytes );
			}
		}
	else if( Rotation < 0 )
		{
		if( m_StartAddress < m_LastPageStartAddress )
			{
			m_CurrentControl->FinishEditing();
			ChangeStartAddress( m_StartAddress + m_WidthBytes );
			}
		}
	}
//==============================================================================
void MemoryDumpPanel::OnSetFocusScrollbar( wxFocusEvent &event )
	{
	wxWindow	*FocusOwner	= dynamic_cast<wxWindow*>( event.GetEventObject() );

	if( dynamic_cast<HexEdit*>( FocusOwner ) == nullptr && dynamic_cast<CharEdit*>( FocusOwner ) == nullptr )
		SetCellFocus();
	}
//==============================================================================
void MemoryDumpPanel::OnScrollChanged( wxScrollEvent& event )
	{
	int			Position				= event.GetPosition();
	uint32_t	NewStartAddress			= m_FirstPageStartAddress + Position * m_WidthBytes;

	if( NewStartAddress != m_StartAddress )
		{
		m_CurrentControl->FinishEditing();
		ChangeStartAddress( NewStartAddress );
		}
	}
//==============================================================================
sim_address_t MemoryDumpPanel::GetCellAddress( unsigned side, unsigned line, unsigned column )
	{
	return m_StartAddress + line * m_WidthBytes + column * ( side == 0 ? m_ColumnWidth : 1 );
	}
//==============================================================================
bool MemoryDumpPanel::SetBuffer( uint32_t bufferbaseaddress, uint32_t bufferlength, bool editable )
	{
	if( bufferlength == 0 || bufferbaseaddress + bufferlength < bufferbaseaddress )
		{
		m_BufferLength			= 0;
		m_BufferBaseAddress		= 0;
		m_StartAddress			= 0;
		m_FirstPageStartAddress	= 0;
		m_LastPageStartAddress	= 0;
		m_Editable				= false;
		m_ScrollBar->Disable();
		DumpRedraw();
		return false;
		}
	else
		{
		m_BufferLength			= bufferlength;
		m_BufferBaseAddress		= bufferbaseaddress;

		m_FirstPageStartAddress	= m_BufferBaseAddress / m_WidthBytes * m_WidthBytes;
		uint32_t	LastAddress	= ( m_BufferBaseAddress + m_BufferLength + m_WidthBytes - 1 ) / m_WidthBytes * m_WidthBytes;
		if( LastAddress - m_FirstPageStartAddress < m_LinesCount * m_WidthBytes )
			LastAddress	= m_FirstPageStartAddress + m_LinesCount * m_WidthBytes;

		m_LastPageStartAddress	= LastAddress - m_LinesCount * m_WidthBytes;

		m_StartAddress			= m_FirstPageStartAddress;

		m_Editable				= editable;
		m_ScrollBar->SetPageSize( m_LinesCount - 1 );
		m_ScrollBar->SetThumbSize( m_LinesCount );
		m_ScrollBar->SetRange(( m_LastPageStartAddress - m_FirstPageStartAddress + m_LinesCount * m_WidthBytes ) / m_WidthBytes );
		m_ScrollBar->SetThumbPosition( 0 );
		m_ScrollBar->Enable();
		DumpRedraw();
		return true;
		}
	}
//==============================================================================
void MemoryDumpPanel::UpdateHeader()
	{
	for( unsigned Column = 0; Column < m_ColumnsCount; Column++ )
		{
		wxString	Text;
		if( m_ColumnWidth == 1 )
			Text.Printf( m_WidthBytes <= 16 ? "%X" : "%02X", Column );
		else if( m_BigEndian )
			Text.Printf( m_WidthBytes <= 16 ? "%X~%0X" : "%02X~%02X", Column * m_ColumnWidth, ( Column + 1 ) * m_ColumnWidth - 1 );
		else
			Text.Printf( m_WidthBytes <= 16 ? "%X~%0X" : "%02X~%02X", ( Column + 1 ) * m_ColumnWidth - 1, Column * m_ColumnWidth );
		m_Header.Hexadecimal[Column]->SetLabel( Text );
		}
	}
//==============================================================================
bool MemoryDumpPanel::UpdateAddress( unsigned line, uint32_t address )
	{
	if( line >= m_LinesCount )
		return false;

	wxString	Text;

	if( address < m_FirstPageStartAddress || address >= m_BufferBaseAddress + m_BufferLength )
		{
		if( m_Placeholder == 0 )
			Text.Printf( "%.*s:", m_AddressNibbles, "????????????????" );
		else
			Text.Printf( "%.*s%c:", m_AddressNibbles - 1, "????????????????" , m_Placeholder );
		m_Lines[line].Address->SetLabel( Text );

		return false;
		}
	else
		{
		if( m_Placeholder == 0 )
			Text.Printf( "%0*X:", m_AddressNibbles, address );
		else
			Text.Printf( "%0*X%c:", m_AddressNibbles - 1, address >> 4, m_Placeholder );
		m_Lines[line].Address->SetLabel( Text );

		return true;
		}
	}
//==============================================================================
bool MemoryDumpPanel::UpdateHexCell( unsigned line, unsigned column, uint32_t address, const wxColour *colour )
	{
	if( line >= m_LinesCount || column >= m_ColumnsCount )
		return false;

	wxString	Text;

	if( m_ColumnWidth == 1 )
		{
		uint8_t	Value;

		//if( wxGetApp().SimulatorThread->LoadByte( address, Value ) < 0 )
		if( address < m_StartAddress || address > m_StartAddress + sizeof m_Buffer - 1 || address > m_BufferBaseAddress + m_BufferLength - 1 )
			Text.Printf( "??" );
		else
			{
			Value	= m_Buffer[address - m_StartAddress];
			Text.Printf( "%02hhX", Value );
			}
		}
	else if( m_ColumnWidth == 2 )
		{
		uint16_t	Value;

		//if( wxGetApp().SimulatorThread->LoadHalf( address, Value ) < 0 )
		if( address < m_StartAddress || address > m_StartAddress + sizeof m_Buffer - 2 || address > m_BufferBaseAddress + m_BufferLength - 2 )
			Text.Printf( "????" );
		else
			{
			if( m_BigEndian )
				//Value	= ( Value << 8 & 0xff00 ) | ( Value >> 8 & 0x00ff );
				Value	= ( (uint16_t)m_Buffer[address - m_StartAddress + 0] << 8 ) | ( (uint16_t)m_Buffer[address - m_StartAddress + 1] << 0 );
			else
				Value	= ( (uint16_t)m_Buffer[address - m_StartAddress + 1] << 8 ) | ( (uint16_t)m_Buffer[address - m_StartAddress + 0] << 0 );

			Text.Printf( "%04hX", Value );
			}
		}
	else if( m_ColumnWidth == 4 )
		{
		uint32_t	Value;

		//if( wxGetApp().SimulatorThread->LoadWord( address, Value ) < 0 )
		if( address < m_StartAddress || address > m_StartAddress + sizeof m_Buffer - 4 || address > m_BufferBaseAddress + m_BufferLength - 4 )
			Text.Printf( "????????" );
		else
			{
			if( m_BigEndian )
				//Value	= ( Value << 24 & 0xff000000 ) | ( Value << 8 & 0x00ff0000 ) | ( Value >> 8 & 0x0000ff00 ) | ( Value >> 24 & 0x000000ff );
				Value	= ( (uint32_t)m_Buffer[address - m_StartAddress + 0] << 24 ) | ( (uint32_t)m_Buffer[address - m_StartAddress + 1] << 16 ) | ( (uint32_t)m_Buffer[address - m_StartAddress + 2] << 8 ) | ( (uint32_t)m_Buffer[address - m_StartAddress + 3] << 0 );
			else
				Value	= ( (uint32_t)m_Buffer[address - m_StartAddress + 3] << 24 ) | ( (uint32_t)m_Buffer[address - m_StartAddress + 2] << 16 ) | ( (uint32_t)m_Buffer[address - m_StartAddress + 1] << 8 ) | ( (uint32_t)m_Buffer[address - m_StartAddress + 0] << 0 );

			Text.Printf( "%08lX", Value );
			}
		}
	else
		Text.Printf( "%.*s", 2 * m_ColumnWidth, "????????????????" );

	uint32_t	Reg;
	wxGetApp().SimulatorThread->GetRegisterValue( SP, Reg );
	bool		IsAtSP			= ( address & ~3 ) == Reg;

	bool		HasBreakpoint	= false;
	for( unsigned i = 0; i < m_ColumnWidth; i++ )
		{
		sim_mapentry_t m	= GetMapEntry( &wxGetApp().CPUState, address + i );
		if( m.DataBreakpoint != 0 )
			{
			HasBreakpoint	= true;
			break;
			}
		}

	bool		IsTarget		= false;
	for( int i = 0; i < 16; i++ )
		{
		wxGetApp().SimulatorThread->GetRegisterValue( i, Reg );
		if( Reg >= address && Reg < address + m_ColumnWidth )
			{
			IsTarget	= true;
			break;
			}
		}

	m_Lines[line].Hexadecimal[column]->SetValue( Text );
	m_Lines[line].Hexadecimal[column]->m_BkColour	= colour;
	m_Lines[line].Hexadecimal[column]->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, HasBreakpoint ? wxFONTSTYLE_ITALIC : wxFONTSTYLE_NORMAL, IsTarget ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL, IsAtSP, wxT("Courier New")) );
	if( !m_IsActive || (line != m_CurrentLine || column != m_CurrentColumn / m_ColumnWidth ))
		m_Lines[line].Hexadecimal[column]->SetBackgroundColour( colour != nullptr ? *colour : *wxWHITE );
	m_Lines[line].Hexadecimal[column]->Refresh();

	return true;
	}
//==============================================================================
bool MemoryDumpPanel::UpdateCharCell( unsigned line, unsigned column, uint32_t address, const wxColour *colour )
	{
	if( line >= m_LinesCount || column >= m_WidthBytes )
		return false;

	wxString	Text;
	uint8_t		Value;

	//if( wxGetApp().SimulatorThread->LoadByte( address, Value ) < 0 )
	if( address < m_StartAddress || address > m_StartAddress + sizeof m_Buffer - 1 || address > m_BufferBaseAddress + m_BufferLength - 1 )
		Text.Printf( "?" );
	else
		{
		Value	= m_Buffer[address - m_StartAddress];
		Text.Printf( "%c", isprint( Value ) ? Value : '.' );
		}

	m_Lines[line].Characters[column]->SetValue( Text );
	m_Lines[line].Characters[column]->m_BkColour	= colour;
	if( !m_IsActive || line != m_CurrentLine || ( m_CurrentSide != 0 ? ( column != m_CurrentColumn ) : ( column / m_ColumnWidth * m_ColumnWidth != m_CurrentColumn / m_ColumnWidth * m_ColumnWidth )))
		m_Lines[line].Characters[column]->SetBackgroundColour( colour != nullptr ? *colour : *wxWHITE );
	m_Lines[line].Characters[column]->Refresh();

	return true;
	}
//==============================================================================
void MemoryDumpPanel::DumpRedraw()
	{
	static const wxColour	*Colours[]	= { nullptr, wxCYAN, wxRED, wxYELLOW };

	uint8_t	Diff[MEMORY_DUMP_LINES*MEMORY_DUMP_BYTE_COLUMNS];

	if( m_BufferLength == 0 )
		return;

	if( !wxGetApp().SimulatorThread->GetMemoryDiff( m_StartAddress, sizeof m_Buffer, m_Buffer, Diff ))
		{
		//TODO
		}

	for( unsigned Line = 0; Line < m_LinesCount; Line++ )
		{
		const uint8_t	*p;

		UpdateAddress( Line, m_StartAddress + Line * m_WidthBytes );

		p	= &Diff[Line*m_WidthBytes];

		for( unsigned Column = 0; Column < m_ColumnsCount; Column++ )
			{
			unsigned	i;
			uint8_t		v;

			for( v = 0, i = 0; i < m_ColumnWidth; i++, p++ )
				v	= *p > v ? *p : v;

			UpdateHexCell( Line, Column, m_StartAddress + Line * m_WidthBytes + Column * m_ColumnWidth, Colours[v & 3] );
			}

		p	= &Diff[Line*m_WidthBytes];

		for( unsigned Column = 0; Column < m_WidthBytes; Column++, p++ )
			UpdateCharCell( Line, Column, m_StartAddress + Line * m_WidthBytes + Column, Colours[*p & 3] );
		}
	}
//==============================================================================
void MemoryDumpPanel::SetActive( bool isactive )
	{
	m_IsActive	= isactive;
	}
//==============================================================================
bool MemoryDumpPanel::SetEditable( bool editable )
	{
	if( m_BufferLength > 0 )
		{
		m_Editable	= editable;
		return true;
		}
	else
		{
		m_Editable	= false;
		return false;
		}
	}
//==============================================================================
void MemoryDumpPanel::SwitchSide()
	{
	SelectCell(( m_CurrentSide + 1 ) & 1, m_CurrentLine, m_CurrentColumn, true );
	}
//==============================================================================
void MemoryDumpPanel::Home( DumpEditCtrl *Control )
	{
	if( m_CurrentColumn > 0 )
		{
		Control->FinishEditing();
		SelectCell( m_CurrentSide, m_CurrentLine, 0, true );
		}
	}
//==============================================================================
void MemoryDumpPanel::End( DumpEditCtrl *Control, unsigned bytes )
	{
	if( m_CurrentColumn < m_WidthBytes - bytes )
		{
		Control->FinishEditing();
		SelectCell( m_CurrentSide, m_CurrentLine, m_WidthBytes - bytes, true );
		}
	}
//==============================================================================
void MemoryDumpPanel::ControlHome( DumpEditCtrl *Control )
	{
	if( m_StartAddress > m_FirstPageStartAddress || m_CurrentLine > 0 || m_CurrentColumn > 0 )
		{
		Control->FinishEditing();
		ChangeStartAddress( m_FirstPageStartAddress );
		SelectCell( m_CurrentSide, 0, 0, true );
		}
	}
//==============================================================================
void MemoryDumpPanel::ControlEnd( DumpEditCtrl *Control, unsigned bytes )
	{
	if( m_StartAddress < m_LastPageStartAddress || m_CurrentLine < m_LinesCount - 1 || m_CurrentColumn < m_WidthBytes - bytes )
		{
		Control->FinishEditing();
		ChangeStartAddress( m_LastPageStartAddress );
		SelectCell( m_CurrentSide, m_LinesCount - 1, m_WidthBytes - bytes, true );
		}
	}
//==============================================================================
void MemoryDumpPanel::PageUp( DumpEditCtrl *Control )
	{
	if( m_StartAddress > m_FirstPageStartAddress || m_CurrentLine > 0 )
		Control->FinishEditing();

	if( m_StartAddress > m_FirstPageStartAddress )
		{
		if( m_StartAddress > m_FirstPageStartAddress + ( m_LinesCount - m_CurrentLine - 1 ) * m_WidthBytes )
			ChangeStartAddress( m_StartAddress - ( m_LinesCount - m_CurrentLine - 1 ) * m_WidthBytes );
		else
			ChangeStartAddress( m_FirstPageStartAddress );
		}
	SelectCell( m_CurrentSide, 0, m_CurrentColumn, true );
	}
//==============================================================================
void MemoryDumpPanel::PageDown( DumpEditCtrl *Control )
	{
	if( m_StartAddress < m_LastPageStartAddress || m_CurrentLine < m_LinesCount - 1 )
		Control->FinishEditing();

	if( m_StartAddress < m_LastPageStartAddress )
		{
		if( m_StartAddress < m_LastPageStartAddress - m_CurrentLine * m_WidthBytes )
			ChangeStartAddress( m_StartAddress + m_CurrentLine * m_WidthBytes );
		else
			ChangeStartAddress( m_LastPageStartAddress );
		}
	SelectCell( m_CurrentSide, m_LinesCount - 1, m_CurrentColumn, true );
	}
//==============================================================================
bool MemoryDumpPanel::MoveUp( DumpEditCtrl *Control )
	{
	if( m_CurrentLine > 0 )
		{
		Control->FinishEditing();
		return SelectCell( m_CurrentSide, m_CurrentLine - 1, m_CurrentColumn, true );
		}
	else if( m_StartAddress > m_FirstPageStartAddress )
		{
		Control->FinishEditing();
		ChangeStartAddress( m_StartAddress - m_WidthBytes );
		return true;
		}
	return false;
	}
//==============================================================================
bool MemoryDumpPanel::MoveDown( DumpEditCtrl *Control )
	{
	if( m_CurrentLine < m_LinesCount - 1 )
		{
		Control->FinishEditing();
		return SelectCell( m_CurrentSide, m_CurrentLine + 1, m_CurrentColumn, true );
		}
	else if( m_StartAddress < m_LastPageStartAddress )
		{
		Control->FinishEditing();
		ChangeStartAddress( m_StartAddress + m_WidthBytes );
		return true;
		}
	return false;
	}
//==============================================================================
bool MemoryDumpPanel::MoveLeft( DumpEditCtrl *Control, unsigned bytes )
	{
	if( m_CurrentColumn >= bytes )
		{
		Control->FinishEditing();
		return SelectCell( m_CurrentSide, m_CurrentLine, m_CurrentColumn - bytes, true );
		}
	else if( m_CurrentLine > 0 )
		{
		Control->FinishEditing();
		return SelectCell( m_CurrentSide, m_CurrentLine - 1, m_WidthBytes - bytes, true );
		}
	else if( m_StartAddress > m_FirstPageStartAddress )
		{
		Control->FinishEditing();
		ChangeStartAddress( m_StartAddress - m_WidthBytes );
		return SelectCell( m_CurrentSide, 0, m_WidthBytes - bytes, true );
		}
	return false;
	}
//==============================================================================
bool MemoryDumpPanel::MoveRight( DumpEditCtrl *Control, unsigned bytes )
	{
	if( m_CurrentColumn < m_WidthBytes - bytes )
		{
		Control->FinishEditing();
		return SelectCell( m_CurrentSide, m_CurrentLine, m_CurrentColumn + bytes, true );
		}
	else if( m_CurrentLine < m_LinesCount - 1 )
		{
		Control->FinishEditing();
		return SelectCell( m_CurrentSide, m_CurrentLine + 1, 0, true );
		}
	else if( m_StartAddress < m_LastPageStartAddress )
		{
		Control->FinishEditing();
		ChangeStartAddress( m_StartAddress + m_WidthBytes );
		return SelectCell( m_CurrentSide, m_LinesCount - 1, 0, true );
		}
	return false;
	}
//==============================================================================
bool MemoryDumpPanel::SelectCell( unsigned side, unsigned line, unsigned column, bool setfocus )
	{
	if(( side == m_CurrentSide && line == m_CurrentLine && column == m_CurrentColumn ) || side >= 2 || line >= m_LinesCount || column >= m_WidthBytes )
		return false;

	if( line != m_CurrentLine || column != m_CurrentColumn || ( m_ColumnWidth > 1 && m_CurrentSide == 0 && side != 0 ))
		ClearCellBackGround();

	m_CurrentSide	= side;
	m_CurrentLine	= line;
	// When passing from the characters side to the hexadecimal side we must round the column.
	m_CurrentColumn	= m_CurrentSide != 0 && side == 0 ? column / m_ColumnWidth * m_ColumnWidth : column;
	m_CurrentControl= m_CurrentSide == 0 ? static_cast<DumpEditCtrl*>( m_Lines[m_CurrentLine].Hexadecimal[m_CurrentColumn/m_ColumnWidth] ) : static_cast<DumpEditCtrl*>( m_Lines[m_CurrentLine].Characters[m_CurrentColumn] );

	if( setfocus )
		SetCellFocus();

	return true;
	}
//==============================================================================
void MemoryDumpPanel::SetCellValueHex( unsigned line, unsigned column, unsigned long int value )
	{
	if( line >= m_LinesCount || column >= m_ColumnsCount )
		return;

	uint32_t	Address	= m_StartAddress + line * m_WidthBytes + column * m_ColumnWidth;

	if( m_ColumnWidth == 1 )
		{
		if( wxGetApp().SimulatorThread->StoreByte( Address, value ) >= 0 )
			m_Buffer[Address - m_StartAddress]	= value;
		}
	else if( m_ColumnWidth == 2 )
		{
		if( m_BigEndian )
			value	= ( value << 8 & 0xff00 ) | ( value >> 8 & 0x00ff );
		if( wxGetApp().SimulatorThread->StoreHalf( Address, value ) >= 0 )
			{
			m_Buffer[Address - m_StartAddress + 0 ]	= value >>  0;
			m_Buffer[Address - m_StartAddress + 1 ]	= value >>  8;
			}
		}
	else if( m_ColumnWidth == 4 )
		{
		if( m_BigEndian )
			value	= ( value << 24 & 0xff000000 ) | ( value << 8 & 0x00ff0000 ) | ( value >> 8 & 0x0000ff00 ) | ( value >> 24 & 0x000000ff );
		if( wxGetApp().SimulatorThread->StoreWord( Address, value ) >= 0 )
			{
			m_Buffer[Address - m_StartAddress + 0 ]	= value >>  0;
			m_Buffer[Address - m_StartAddress + 1 ]	= value >>  8;
			m_Buffer[Address - m_StartAddress + 2 ]	= value >> 16;
			m_Buffer[Address - m_StartAddress + 3 ]	= value >> 24;
			}
		}

	UpdateHexCell( line, column, Address, wxYELLOW );

	UpdateCharCell( line, column * m_ColumnWidth + 0, Address + 0, wxYELLOW );
	if( m_ColumnWidth >= 2 )
		UpdateCharCell( line, column * m_ColumnWidth + 1, Address + 1, wxYELLOW );
	if( m_ColumnWidth == 4 )
		{
		UpdateCharCell( line, column * m_ColumnWidth + 2, Address + 2, wxYELLOW );
		UpdateCharCell( line, column * m_ColumnWidth + 3, Address + 3, wxYELLOW );
		}
	}
//==============================================================================
void MemoryDumpPanel::SetCellValueChar( unsigned line, unsigned column, unsigned long int value )
	{
	if( line >= m_LinesCount || column >= m_WidthBytes )
		return;

	uint32_t	Address	= m_StartAddress + line * m_WidthBytes + column;

	if( wxGetApp().SimulatorThread->StoreByte( Address, value ) >= 0 )
		m_Buffer[Address - m_StartAddress]	= value;

	UpdateCharCell( line, column, Address, wxYELLOW );
	UpdateHexCell( line, column / m_ColumnWidth, Address / m_ColumnWidth * m_ColumnWidth, wxYELLOW );
	}
//==============================================================================
void MemoryDumpPanel::SetCellBackGround()
	{
	wxTextCtrl	*CtrlHexa	= static_cast<wxTextCtrl*>( m_Lines[m_CurrentLine].Hexadecimal[m_CurrentColumn/m_ColumnWidth] );
	wxTextCtrl	*CtrlChar	= static_cast<wxTextCtrl*>( m_Lines[m_CurrentLine].Characters[m_CurrentColumn] );

	wxTextCtrl	*Ctrl1	= m_CurrentSide == 0 ? CtrlHexa : CtrlChar;
	wxTextCtrl	*Ctrl2	= m_CurrentSide != 0 ? CtrlHexa : CtrlChar;

	Ctrl1->SetBackgroundColour( *wxBLUE );
	Ctrl1->SetForegroundColour( *wxWHITE );
	Ctrl1->Refresh();

	Ctrl2->SetBackgroundColour( wxColour( 192, 192, 192 ));
	Ctrl2->SetForegroundColour( *wxBLACK );
	Ctrl2->Refresh();

	if( m_CurrentSide == 0 && m_ColumnWidth > 1 )
		{
		for( unsigned i = 1; i < m_ColumnWidth; i++ )
			{
			CtrlChar	= static_cast<wxTextCtrl*>( m_Lines[m_CurrentLine].Characters[m_CurrentColumn + i] );
			CtrlChar->SetBackgroundColour( wxColour( 192, 192, 192 ));
			CtrlChar->SetForegroundColour( *wxBLACK );
			CtrlChar->Refresh();
			}
		}
	}
//==============================================================================
void MemoryDumpPanel::ClearCellBackGround()
	{
	DumpEditCtrl	*Control;

	Control	= static_cast<DumpEditCtrl*>( m_Lines[m_CurrentLine].Hexadecimal[m_CurrentColumn/m_ColumnWidth] );
	Control->SetBackgroundColour( Control->m_BkColour != nullptr ? *Control->m_BkColour : *wxWHITE );
	Control->SetForegroundColour( *wxBLACK );
	Control->Refresh();

	unsigned Column	= m_CurrentSide == 1 ? m_CurrentColumn : m_CurrentColumn / m_ColumnWidth * m_ColumnWidth;
	for( unsigned i = 0; i == 0 || ( m_CurrentSide == 0 && i < m_ColumnWidth ); i++ )
		{
		Control	= static_cast<DumpEditCtrl*>( m_Lines[m_CurrentLine].Characters[Column + i] );
		Control->SetBackgroundColour( Control->m_BkColour != nullptr ? *Control->m_BkColour : *wxWHITE );
		Control->SetForegroundColour( *wxBLACK );
		Control->Refresh();
		}
	}
//==============================================================================
void MemoryDumpPanel::SetCellFocus()
	{
	if( m_CurrentSide == 0 )
		m_Lines[m_CurrentLine].Hexadecimal[m_CurrentColumn/m_ColumnWidth]->SetFocus();
	else
		m_Lines[m_CurrentLine].Characters[m_CurrentColumn]->SetFocus();
	}
//==============================================================================
void MemoryDumpPanel::SetCurrentCell( unsigned side, unsigned line, unsigned column )
	{
	if( side >= 2 || line >= m_LinesCount || column >= m_WidthBytes )
		return;

	m_CurrentSide	= side;
	m_CurrentLine	= line;
	m_CurrentColumn	= column;
	m_CurrentControl= m_CurrentSide == 0 ? static_cast<DumpEditCtrl*>( m_Lines[m_CurrentLine].Hexadecimal[m_CurrentColumn/m_ColumnWidth] ) : static_cast<DumpEditCtrl*>( m_Lines[m_CurrentLine].Characters[m_CurrentColumn] );
	}
//==============================================================================
