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
//===============================================================================
#include "wxThumbApp.h"
#include "RegistersPanel.h"
#include <stdint.h>
//===============================================================================
wxDECLARE_APP( wxThumbApp );
//===============================================================================
class	RegisterEditCtrl: public wxTextCtrl
	{
	//--------------------------------------------------------------------------
	public:
	//--------------------------------------------------------------------------
							RegisterEditCtrl	( RegistersPanel	*container,
												  unsigned			line,
												  unsigned			column,
												  wxPanel			*parent,
												  wxWindowID		id,
												  const wxString	&value		= wxEmptyString,
												  const wxPoint		&pos		= wxDefaultPosition,
												  const wxSize		&size		= wxDefaultSize,
												  long				style		= 0,
												  const wxValidator	&validator	= wxDefaultValidator,
												  const wxString	&name		= wxASCII_STR( wxTextCtrlNameStr ))
												: wxTextCtrl( parent, id, value, pos, size, style, validator, name ),
												  m_Container( container ), m_Line( line ), m_Column( column ),
												  m_Editing( false ), m_BkColour( nullptr ) {};

		virtual				~RegisterEditCtrl	() {};
	//--------------------------------------------------------------------------
		virtual void		OnChar				( wxKeyEvent &event );
		virtual void		OnKillFocus			( wxFocusEvent &event );
		virtual void		OnLeftDown			( wxMouseEvent &event );
		virtual void		OnSetFocus			( wxFocusEvent &event );
		virtual void		DummyMouseEvent		( wxMouseEvent& event ) {};
		virtual void		OnRightDown			( wxMouseEvent& event ) {};
	//--------------------------------------------------------------------------
		unsigned			GetLine				() { return m_Line; };
		virtual unsigned	GetColumn			() { return m_Column; };
	//--------------------------------------------------------------------------
		virtual bool		StartEditing		();
		virtual bool		CancelEditing		();
		virtual bool		FinishEditing		();

		void				SetBkColour			( const wxColour *colour );
		void				ClearBkColour		();
		void				RestoreBkColour		();
	//--------------------------------------------------------------------------
	protected:
	//--------------------------------------------------------------------------
		RegistersPanel		*m_Container;
		unsigned			m_Line, m_Column;
		bool				m_Editing;
		wxString			m_OriginalValue;
		const wxColour		*m_BkColour;
	//--------------------------------------------------------------------------
	};
//==============================================================================
void RegisterEditCtrl::SetBkColour( const wxColour *colour )
	{
	RegisterEditCtrl	*Ctrl0	= m_Container->m_RegsEditCtrl[m_Line][0];
	RegisterEditCtrl	*Ctrl1	= m_Container->m_RegsEditCtrl[m_Line][1];
	RegisterEditCtrl	*Ctrl2	= m_Container->m_RegsEditCtrl[m_Line][2];

	Ctrl0->m_BkColour	= colour;
	Ctrl1->m_BkColour	= colour;
	Ctrl2->m_BkColour	= colour;

	if( !Ctrl0->HasFocus() )
		Ctrl0->SetBackgroundColour( *colour );
	if( !Ctrl1->HasFocus() )
		Ctrl1->SetBackgroundColour( *colour );
	if( !Ctrl2->HasFocus() )
		Ctrl2->SetBackgroundColour( *colour );
	}
//==============================================================================
void RegisterEditCtrl::ClearBkColour()
	{
	RegisterEditCtrl	*Ctrl0	= m_Container->m_RegsEditCtrl[m_Line][0];
	RegisterEditCtrl	*Ctrl1	= m_Container->m_RegsEditCtrl[m_Line][1];
	RegisterEditCtrl	*Ctrl2	= m_Container->m_RegsEditCtrl[m_Line][2];

	Ctrl0->m_BkColour	= nullptr;
	Ctrl1->m_BkColour	= nullptr;
	Ctrl2->m_BkColour	= nullptr;

	bool	Focus0	= Ctrl0->HasFocus();
	bool	Focus1	= Ctrl1->HasFocus();
	bool	Focus2	= Ctrl2->HasFocus();

	// None of the three controls on the line has focus...
	if( !Focus0 & !Focus1 && !Focus2 )
		{
		// ..so we just set all their backgrounds to white.
		Ctrl0->SetBackgroundColour( *wxWHITE );
		Ctrl1->SetBackgroundColour( *wxWHITE );
		Ctrl2->SetBackgroundColour( *wxWHITE );
		}
	// One of the three controls on the line has focus...
	else
		{
		// ...we will change the background of the other two to gray.
		if( !Focus0 )
			Ctrl0->SetBackgroundColour( wxColour( 192, 192, 192 ) );
		if( !Focus1 )
			Ctrl1->SetBackgroundColour( wxColour( 192, 192, 192 ) );
		if( !Focus2 )
			Ctrl2->SetBackgroundColour( wxColour( 192, 192, 192 ) );
		}
	}
//==============================================================================
void RegisterEditCtrl::RestoreBkColour()
	{
	m_Container->m_RegsEditCtrl[m_Line][0]->SetBackgroundColour( m_BkColour != nullptr ? *m_BkColour : *wxWHITE );
	m_Container->m_RegsEditCtrl[m_Line][1]->SetBackgroundColour( m_BkColour != nullptr ? *m_BkColour : *wxWHITE );
	m_Container->m_RegsEditCtrl[m_Line][2]->SetBackgroundColour( m_BkColour != nullptr ? *m_BkColour : *wxWHITE );
	}
//==============================================================================
void RegisterEditCtrl::OnLeftDown( wxMouseEvent &event )
	{
	wxWindow	*FocusOwner	= FindFocus();

	if( FocusOwner == event.GetEventObject() )
		return;

	m_Container->SelectCell( m_Line, m_Column );
	}
//==============================================================================
#define	LENGTH(a)	( sizeof (a) /sizeof (a)[0] )
//==============================================================================
void RegisterEditCtrl::OnSetFocus( wxFocusEvent &event )
	{
	if( m_Line != m_Container->m_SelectedLine || m_Column != m_Container->m_SelectedColumn )
		{
		m_Container->m_RegsEditCtrl[m_Container->m_SelectedLine][m_Container->m_SelectedColumn]->SetFocus();
		return;
		}

	RegisterEditCtrl	*CtrlB	= m_Container->m_RegsEditCtrl[m_Line][( m_Column + 1 ) % COLUMNS_COUNT];
	RegisterEditCtrl	*CtrlC	= m_Container->m_RegsEditCtrl[m_Line][( m_Column + 2 ) % COLUMNS_COUNT];

	SetInsertionPoint( -1 );

	SetBackgroundColour( *wxBLUE );
	SetForegroundColour( *wxWHITE );
	Refresh();

	if( CtrlB->m_BkColour == nullptr )
		{
		CtrlB->SetBackgroundColour( wxColour( 192, 192, 192 ) );
		CtrlB->Refresh();
		}

	if( CtrlC->m_BkColour == nullptr )
		{
		CtrlC->SetBackgroundColour( wxColour( 192, 192, 192 ) );
		CtrlC->Refresh();
		}

	event.Skip();
	}
//==============================================================================
void RegisterEditCtrl::OnKillFocus( wxFocusEvent &event )
	{
	CancelEditing();

	SetForegroundColour( *wxBLACK );

	m_Container->m_RegsEditCtrl[m_Line][0]->RestoreBkColour();

	m_Container->m_RegsEditCtrl[m_Line][0]->Refresh();
	m_Container->m_RegsEditCtrl[m_Line][1]->Refresh();
	m_Container->m_RegsEditCtrl[m_Line][2]->Refresh();

	event.Skip();
	}
//==============================================================================
bool RegisterEditCtrl::StartEditing()
	{
	if( wxGetApp().SimulatorThread->IsBusy() || m_Editing )
		return false;

	m_OriginalValue	= GetValue();
	m_Editing		= true;

	return true;
	}
//==============================================================================
bool RegisterEditCtrl::CancelEditing()
	{
	if( !m_Editing )
		return false;

	SetValue( m_OriginalValue );
	Refresh();
	m_Editing	= false;

	return true;
	}
//==============================================================================
bool RegisterEditCtrl::FinishEditing()
	{
	if( !m_Editing )
		return false;

	if( GetValue().Length() == 0  )
		{
		SetValue( m_OriginalValue );
		Refresh();
		m_Editing	= false;
		return false;
		}

	unsigned long int	Value;
	wxString	Text	= GetValue();
	switch( m_Column )
		{
		default:
		case 0:
			if( Text[0] == '0' && Text[1] == 'x' )
				{
				if( Text.Length() == 2 )
					return false;
				Text	= Text.Right( Text.Length() - 2 );
				}
			sscanf( Text, "%lx", &Value );
			break;

			break;
		case 1:
			sscanf( Text, "%lu", &Value );
			break;
		case 2:
			if( Text.Length() > 0 && Text.Right( 1 ) == "d" )
				Text.RemoveLast();
			if( Text.Length() == 0 || !isdigit( Text.Right( 1 )[0] ))
				return false;

			sscanf( Text, "%ld", &Value );
			break;
		}

	uint32_t	OriginalValue, Control;
	wxGetApp().SimulatorThread->GetRegisterValue( m_Line, OriginalValue );
	wxGetApp().SimulatorThread->GetRegisterValue( CONTROL, Control );

	Text.Printf( "0x%08X", Value );
	m_Container->m_RegsEditCtrl[m_Line][0]->SetValue( Text );
	Text.Printf( "%lu", Value );
	m_Container->m_RegsEditCtrl[m_Line][1]->SetValue( Text );
	Text.Printf( "%+ld", Value );
	m_Container->m_RegsEditCtrl[m_Line][2]->SetValue( Text );

	if( m_Line == SP )
		{
		Text.Printf( "0x%08X", Value );
		m_Container->m_RegsEditCtrl[(Control&2)?PSP:MSP][0]->SetValue( Text );
		Text.Printf( "%lu", Value );
		m_Container->m_RegsEditCtrl[(Control&2)?PSP:MSP][1]->SetValue( Text );
		Text.Printf( "%+ld", Value );
		m_Container->m_RegsEditCtrl[(Control&2)?PSP:MSP][2]->SetValue( Text );
		}
	else if(( m_Line == MSP && ( Control & 2 ) == 0 ) || ( m_Line == PSP && ( Control & 2 ) != 0 ))
		{
		Text.Printf( "0x%08X", Value );
		m_Container->m_RegsEditCtrl[SP][0]->SetValue( Text );
		Text.Printf( "%lu", Value );
		m_Container->m_RegsEditCtrl[SP][1]->SetValue( Text );
		Text.Printf( "%+ld", Value );
		m_Container->m_RegsEditCtrl[SP][2]->SetValue( Text );
		}

	if( Value != OriginalValue )
		{
		wxGetApp().SimulatorThread->SetRegisterValue( m_Line, Value );
		m_Container->m_RegsEditCtrl[m_Line][0]->SetBkColour( wxYELLOW );
		if( m_Line == SP )
			{
			m_Container->m_RegsEditCtrl[(Control&2)?PSP:MSP][0]->SetBkColour( wxYELLOW );
			m_Container->m_RegsEditCtrl[(Control&2)?PSP:MSP][0]->Refresh();
			}
		else if(( m_Line == MSP && ( Control & 2 ) == 0 ) || ( m_Line == PSP && ( Control & 2 ) != 0 ))
			{
			m_Container->m_RegsEditCtrl[SP][0]->SetBkColour( wxYELLOW );
			m_Container->m_RegsEditCtrl[SP][0]->Refresh();
			}

		}

	m_Editing	= false;

	return true;
	}
//==============================================================================
void RegisterEditCtrl::OnChar( wxKeyEvent& event )
	{
	int			KeyCode		= event.GetKeyCode();

	if( !isprint( KeyCode ))
		{
		int	Modifiers	= event.GetModifiers();

		switch( KeyCode )
			{
			case WXK_HOME:
				if( Modifiers & wxMOD_CONTROL )
					{
					if( m_Line > 0 || m_Column > 0 )
						{
						FinishEditing();
						m_Container->SelectCell( 0, 0 );;
						}
					}
				else
					{
					if( m_Column > 0 )
						{
						FinishEditing();
						m_Container->SelectCell( m_Line, 0 );
						}
					}
				return;
			case WXK_PAGEUP:
				FinishEditing();
				if( m_Line > 0 )
					m_Container->SelectCell( 0, m_Column );
				return;
			case WXK_PAGEDOWN:
				FinishEditing();
				if( m_Line < ARM_REGISTERS_COUNT + 2 - 1 )
					m_Container->SelectCell( ARM_REGISTERS_COUNT + 2 - 1, m_Column );
				return;
			case WXK_UP:
				FinishEditing();
				if( m_Line > 0 )
					m_Container->SelectCell( m_Line - 1, m_Column );
				return;
			case WXK_RETURN:
				if( m_Editing && !FinishEditing() )
					{
					SetInsertionPoint( -1 );
					return;
					}
			case WXK_DOWN:
				FinishEditing();
				if( m_Line < ARM_REGISTERS_COUNT + 2 - 1 )
					m_Container->SelectCell( m_Line + 1, m_Column );
				return;
			case WXK_LEFT:
				FinishEditing();
				if( m_Column > 0 )
					m_Container->SelectCell( m_Line, m_Column - 1 );
/*
				else if( m_Line > 0 )
					m_Container->m_RegsEditCtrl[m_Line - 1][2]->SetFocus();
*/
				return;
			case WXK_RIGHT:
				FinishEditing();
				if( m_Column < COLUMNS_COUNT - 1 )
					m_Container->SelectCell( m_Line, m_Column + 1 );
/*
				else if( m_Line < ARM_REGISTERS_COUNT + 2 - 1 )
					m_Container->m_RegsEditCtrl[m_Line + 1][0]->SetFocus();
*/
				return;
			case WXK_END:
				if( Modifiers & wxMOD_CONTROL )
					{
					if( m_Line < ARM_REGISTERS_COUNT + 2 - 1 || m_Column < COLUMNS_COUNT - 1 )
						{
						FinishEditing();
						m_Container->SelectCell( ARM_REGISTERS_COUNT + 2 - 1, 2 );
						}
					}
				else
					{
					if( m_Column < COLUMNS_COUNT - 1 )
						{
						FinishEditing();
						m_Container->SelectCell( m_Line, 2 );
						}
					}
				return;
			case WXK_DELETE:
				if( wxGetApp().SimulatorThread->IsBusy() )
					return;
				if( StartEditing() )
					{
					SetValue( "" );
					Refresh();
					}
				return;
			case WXK_BACK:
				{
				if( wxGetApp().SimulatorThread->IsBusy() )
					return;
				StartEditing();
				wxString	Text	= GetValue();
				Text.Truncate( Text.Length() - 1 );
				SetValue( Text );
				Refresh();
				SetInsertionPoint( -1 );
				return;
				}
			case WXK_ESCAPE:
				CancelEditing();
				SetInsertionPoint( -1 );
				return;
			case WXK_TAB:
				FinishEditing();
				if( Modifiers & wxMOD_SHIFT )
					GetParent()->Navigate( wxNavigationKeyEvent::IsBackward );
				else
					GetParent()->Navigate( wxNavigationKeyEvent::IsForward );
				return;
			default:
				return;
			}
		}

	if( wxGetApp().SimulatorThread->IsBusy() )
		return;

	wxString	Text	= GetValue();

	switch( m_Column )
		{
		default:
		case 0:
			{
			if(( KeyCode = toupper( KeyCode )) == 'X' )
				KeyCode	= 'x';

			if( !isxdigit( KeyCode ) && ( KeyCode != 'x' || !m_Editing || Text.Length() != 1 || Text[0] != '0' ))
				return;

			if( !m_Editing )
				{
				Text			= (wchar_t)KeyCode;
				StartEditing();
				}
			else
				{
				if(( Text.Length() >= 8 && Text[1] != 'x' ) || Text.Length() >= 10 )
					return;
				Text.Append( (wchar_t)KeyCode );
				}
			break;
			}
		case 1:
			{
			if( !isdigit( KeyCode ))
				return;

			if( !m_Editing )
				{
				Text			= (wchar_t)KeyCode;
				StartEditing();
				}
			else
				{
				uint64_t	Temp;

				if( Text.Length() >= 10 )
					return;

				Text.Append( (wchar_t)KeyCode );

				if( sscanf( Text.ToAscii(), "%llu", &Temp ) != 1 || Temp > UINT_MAX )
					return;

				Text.Printf( "%llu", Temp );
				}
			break;
			}
		case 2:
			{
			if( !m_Editing )
				{
				if( !isdigit( KeyCode ) && KeyCode != '-' && KeyCode != '+' )
					return;

				Text		= (wchar_t)KeyCode;
				StartEditing();
				}
			else
				{
				wxString	Sign;

				// The user just typed a sign...
				if( KeyCode == '-' || KeyCode == '+' )
					{
					// ...but the string is not empty...
					if( Text.Length() != 0 )
						// ...we cannot accept it.
						return;

					Text	= (wchar_t)KeyCode;
					break;
					}

				// The string begins with a sign...
				if( Text[0] == '-' || Text[0] == '+' )
					{
					// ...let's save the sign...
					Sign	= Text[0];
					// ...and remove it for further processing of the value.
					Text	= Text.Mid( 1 );
					}
				// The string does not begin with a sign...
				else
					Sign	= "";

				// At this point we can only accept decimal digits...
				if( !isdigit( KeyCode ))
					return;

				Text.Append( (wchar_t)KeyCode );

				int64_t	Temp;
				// Convert the string back to a number and check whether it will fit the variable...
				if( sscanf( Text.ToAscii(), "%lld", &Temp ) != 1 || Temp < INT_MIN || Temp > INT_MAX )
					// ...the new value will not fit the destination, reject it.
					return;

				Text.Printf( "%s%lld", Sign, Temp );
				}
			break;
			}
		}

	SetValue( Text );
	SetInsertionPoint( -1 );
	}
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
class	CPSREditCtrl: public wxTextCtrl
	{
	//--------------------------------------------------------------------------
	public:
	//--------------------------------------------------------------------------
							CPSREditCtrl		( RegistersPanel	*container,
												  unsigned			index,
												  wxPanel			*parent,
												  wxWindowID		id,
												  const wxString	&value		= wxEmptyString,
												  const wxPoint		&pos		= wxDefaultPosition,
												  const wxSize		&size		= wxDefaultSize,
												  long				style		= 0,
												  const wxValidator	&validator	= wxDefaultValidator,
												  const wxString	&name		= wxASCII_STR( wxTextCtrlNameStr ))
												: wxTextCtrl( parent, id, value, pos, size, style, validator, name ),
												  m_Container( container ), m_Index( index ), m_BkColour( nullptr ) {};

		virtual				~CPSREditCtrl		() {};
	//--------------------------------------------------------------------------
		virtual void		OnChar				( wxKeyEvent &event ) = 0;
		virtual void		OnLeftDown			( wxMouseEvent &event );
		virtual void		OnSetFocus			( wxFocusEvent &event );
		virtual void		OnKillFocus			( wxFocusEvent &event );
		virtual void		OnLeftDClick		( wxMouseEvent &event ) {};
		virtual void		DummyMouseEvent		( wxMouseEvent& event ) {};

		virtual void		RedrawBit			( uint32_t value, uint32_t written, uint32_t writtenbyapp ) = 0;
	//--------------------------------------------------------------------------
	protected:
	//--------------------------------------------------------------------------
		virtual bool		CancelEditing		() { return true; };
	//--------------------------------------------------------------------------
		RegistersPanel		*m_Container;
		unsigned			m_Index;
		const wxColour		*m_BkColour;
	//--------------------------------------------------------------------------
	};
//==============================================================================
void CPSREditCtrl::OnLeftDown( wxMouseEvent &event )
	{
	wxWindow	*FocusOwner	= FindFocus();

	if( FocusOwner == event.GetEventObject() )
		return;

	m_Container->SelectBit( m_Index );
	}
//==============================================================================
void CPSREditCtrl::OnSetFocus( wxFocusEvent &event )
	{
	if( m_Index != m_Container->m_SelectedBit )
		{
		m_Container->m_BitsEditCtrl[m_Container->m_SelectedBit]->SetFocus();
		return;
		}

	SetInsertionPoint( -1 );

	SetBackgroundColour( *wxBLUE );
	SetForegroundColour( *wxWHITE );
	Refresh();
	}
//==============================================================================
void CPSREditCtrl::OnKillFocus( wxFocusEvent &event )
	{
	CancelEditing();

	SetBackgroundColour( m_BkColour != nullptr ? *m_BkColour : *wxWHITE );
	SetForegroundColour( *wxBLACK );
	Refresh();

	event.Skip();
	}
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
class	BitEditCtrl: public CPSREditCtrl
	{
	//--------------------------------------------------------------------------
	public:
	//--------------------------------------------------------------------------
							BitEditCtrl			( RegistersPanel	*container,
												  unsigned			index,
												  unsigned			bit,
												  wchar_t			charon,
												  wchar_t			charoff,
												  wxPanel			*parent,
												  wxWindowID		id,
												  const wxString	&value		= wxEmptyString,
												  const wxPoint		&pos		= wxDefaultPosition,
												  const wxSize		&size		= wxDefaultSize,
												  long				style		= 0,
												  const wxValidator	&validator	= wxDefaultValidator,
												  const wxString	&name		= wxASCII_STR( wxTextCtrlNameStr ))
												: CPSREditCtrl( container, index, parent, id, value, pos, size, style, validator, name ),
												  m_BitMask( 1 << bit ), m_CharOn( charon ), m_CharOff( charoff ) {};

		virtual				~BitEditCtrl		() {};
	//--------------------------------------------------------------------------
		virtual void		OnChar				( wxKeyEvent &event ) override;
		virtual void		OnLeftDClick		( wxMouseEvent &event ) override;
		bool				Toggle				( unsigned state );
		virtual void		RedrawBit			( uint32_t value, uint32_t written, uint32_t writtenbyapp ) override;
	//--------------------------------------------------------------------------
	protected:
	//--------------------------------------------------------------------------
		uint32_t			m_BitMask;
		wchar_t				m_CharOn, m_CharOff;
	//--------------------------------------------------------------------------
	};
//==============================================================================
class	BitFieldEditCtrl: public CPSREditCtrl
	{
	//--------------------------------------------------------------------------
	public:
	//--------------------------------------------------------------------------
							BitFieldEditCtrl	( RegistersPanel	*container,
												  unsigned			index,
												  unsigned			firstbit,
												  unsigned			numbits,
												  wxPanel			*parent,
												  wxWindowID		id,
												  const wxString	&value		= wxEmptyString,
												  const wxPoint		&pos		= wxDefaultPosition,
												  const wxSize		&size		= wxDefaultSize,
												  long				style		= 0,
												  const wxValidator	&validator	= wxDefaultValidator,
												  const wxString	&name		= wxASCII_STR( wxTextCtrlNameStr ))
												: CPSREditCtrl( container, index, parent, id, value, pos, size, style, validator, name ),
												  m_FirstBit( firstbit ), m_NumBits( numbits ), m_Editing( false ) {};

		virtual				~BitFieldEditCtrl	() {};
	//--------------------------------------------------------------------------
		virtual void		OnChar				( wxKeyEvent &event );
		virtual bool		CancelEditing		() override;
		virtual void		RedrawBit			( uint32_t value, uint32_t written, uint32_t writtenbyapp ) override;
	//--------------------------------------------------------------------------
	protected:
	//--------------------------------------------------------------------------
		unsigned			m_FirstBit, m_NumBits;
		bool				m_Editing;
		wxString			m_OriginalValue;
	//--------------------------------------------------------------------------
	};
//==============================================================================
bool BitEditCtrl::Toggle( unsigned state )
	{
	if( wxGetApp().SimulatorThread->IsBusy() )
		return false;

	uint32_t	OriginalValue, Value;

	if( wxGetApp().SimulatorThread->GetCPSRValue( OriginalValue ) < 0 )
		return false;

	Value	= state == 0 ? OriginalValue & ~m_BitMask : state == 1 ? OriginalValue | m_BitMask : OriginalValue ^ m_BitMask;

	SetValue(( Value & m_BitMask ) != 0 ? m_CharOn : m_CharOff );
	wxGetApp().SimulatorThread->SetCPSRValue( Value );

	if( Value != OriginalValue )
		{
		m_BkColour	= wxYELLOW;
		if( !HasFocus() )
			SetBackgroundColour( *m_BkColour );
		}

	return true;
	}
//==============================================================================
void BitEditCtrl::RedrawBit( uint32_t value, uint32_t written, uint32_t writtenbyapp )
	{
	SetValue(( value & m_BitMask ) != 0 ? m_CharOn : m_CharOff );

	m_BkColour	=  ( writtenbyapp & m_BitMask ) &&  ( written & m_BitMask ) ? wxYELLOW
				:  ( writtenbyapp & m_BitMask ) && !( written & m_BitMask ) ? wxRED
				: !( writtenbyapp & m_BitMask ) &&  ( written & m_BitMask ) ? wxCYAN : nullptr;

	if( !HasFocus() )
		SetBackgroundColour( m_BkColour != nullptr ? *m_BkColour : *wxWHITE );

	Refresh();
	}
//==============================================================================
void BitEditCtrl::OnLeftDClick( wxMouseEvent &event )
	{
	Toggle( 2 );
	}
//==============================================================================
void BitEditCtrl::OnChar( wxKeyEvent& event )
	{
	int	KeyCode		= event.GetKeyCode();

	switch( KeyCode )
		{
		case WXK_HOME:
			if( m_Index != 0 )
				m_Container->SelectBit( 0 );
			return;
		case WXK_LEFT:
			if( m_Index != 0 )
				m_Container->SelectBit( m_Index - 1 );
			return;
		case WXK_RIGHT:
			if( m_Index != 10 )
				m_Container->SelectBit( m_Index + 1 );
			return;
		case WXK_END:
			m_Container->SelectBit( 10 );
			return;
		case '0':
		case '1':
			Toggle( KeyCode & 1 );
			return;
		case WXK_SPACE:
		case WXK_RETURN:
			Toggle( 2 );
			return;
		case WXK_TAB:
			if( event.GetModifiers() & wxMOD_SHIFT )
				GetParent()->Navigate( wxNavigationKeyEvent::IsBackward );
			else
				GetParent()->Navigate( wxNavigationKeyEvent::IsForward );
			return;
		default:
			// CharOn and CharOff are the same letter in upper- and lower-case...
			if( toupper( m_CharOn ) == toupper( m_CharOff ) )
				{
				// ...let's compare them case-sensitively.
				if( KeyCode == m_CharOn )
					Toggle( 1 );
				else if( KeyCode == m_CharOff )
					Toggle( 0 );
				}
			// CharOn and CharOff are two different letters...
			else
				{
				// ...let's do a case-insensitive compare.
				if( toupper( KeyCode ) == toupper( m_CharOn ))
					Toggle( 1 );
				else if( toupper( KeyCode ) == toupper( m_CharOff ))
					Toggle( 0 );
				}
			return;
		}
	}
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
void BitFieldEditCtrl::RedrawBit( uint32_t value, uint32_t written, uint32_t writtenbyapp )
	{
	wxString	Text;

	Text.Printf( "" );

	uint32_t	Aux	= value >> m_FirstBit;
	for( unsigned i = 0; i < m_NumBits; i++, Aux >>= 1 )
		Text   += Aux & 1 ? L'1' : L'0';

	SetValue( Text );

	uint32_t	Mask	= (( 1 << m_NumBits ) - 1 ) << m_FirstBit;
	m_BkColour	=  ( writtenbyapp & Mask ) &&  ( written & Mask ) ? wxYELLOW
				:  ( writtenbyapp & Mask ) && !( written & Mask ) ? wxRED
				: !( writtenbyapp & Mask ) &&  ( written & Mask ) ? wxCYAN : nullptr;

	if( !HasFocus() )
		SetBackgroundColour( m_BkColour != nullptr ? *m_BkColour : *wxWHITE );

	Refresh();
	}
//==============================================================================
bool BitFieldEditCtrl::CancelEditing()
	{
	if( !m_Editing )
		return false;

	SetValue( m_OriginalValue );
	Refresh();
	m_Editing	= false;

	return true;
	}
//==============================================================================
void BitFieldEditCtrl::OnChar( wxKeyEvent& event )
	{
	int	KeyCode		= event.GetKeyCode();

	switch( KeyCode )
		{
		case WXK_HOME:
			m_Container->SelectBit( 0 );
			return;
		case WXK_LEFT:
			m_Container->SelectBit( m_Index - 1 );
			return;
		case WXK_RIGHT:
			return;
		case WXK_END:
			return;
		case WXK_RETURN:
			{
			wxString	Text	= GetValue();

			if( Text.Length() != m_NumBits )
				return;

			uint32_t	OriginalValue;
			if( wxGetApp().SimulatorThread->GetCPSRValue( OriginalValue ) < 0 )
				return;

			uint32_t Bits	= 0;
			for( unsigned i = 0; i < m_NumBits; i++ )
				Bits	= ( Bits << 1 ) | ( Text[i] - '0' );
			Bits  <<= m_FirstBit;

			uint32_t	Value	= ( OriginalValue & ~((( 1 << m_NumBits ) - 1 ) << m_FirstBit )) | Bits;

			if( Value != OriginalValue )
				{
				if( wxGetApp().SimulatorThread->SetCPSRValue( Value ) < 0 )
					{
					SetValue( m_OriginalValue );
					return;
					}

				m_BkColour	= wxYELLOW;

				if( !HasFocus() )
					SetBackgroundColour( *m_BkColour );
				}

			m_Editing	= 0;
			return;
			}
		case WXK_TAB:
			if( event.GetModifiers() & wxMOD_SHIFT )
				GetParent()->Navigate( wxNavigationKeyEvent::IsBackward );
			else
				GetParent()->Navigate( wxNavigationKeyEvent::IsForward );
			return;
		case WXK_BACK:
			if( !m_Editing )
				{
				m_OriginalValue	= GetValue();
				m_Editing		= true;
				}
			SetValue( GetValue().RemoveLast() );
			return;
		case WXK_ESCAPE:
			if( !m_Editing )
				return;
			SetValue( m_OriginalValue );
			m_Editing	= false;
			return;
		case '0':
		case '1':
			if( wxGetApp().SimulatorThread->IsBusy() )
				return;
			if( !m_Editing )
				{
				m_OriginalValue	= GetValue();
				SetValue( (wchar_t)KeyCode );
				m_Editing	= true;
				}
			else
				{
				wxString	Value	= GetValue();
				if( Value.Length() < m_NumBits )
					SetValue( Value.Append( (wchar_t)KeyCode ));
				}
			return;
		default:
			return;
		}
	}
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
RegistersPanel::RegistersPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name ) : wxPanel( parent, id, pos, size, style, name )
	{
	static const char * const	HeaderTexts[4]	=
		{ "", "Hexadecimal", "Unsigned Decimal", "Signed Decimal" };
	static const char * const	RegistersNames[ARM_REGISTERS_COUNT+2]	=
		{ "R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7", "R8", "R9", "R10/SL", "R11/FP", "R12/IP", "R13/SP", "R14/LR", "R15/PC", "MSP", "PSP" };
	static const char * const	RegistersTips[ARM_REGISTERS_COUNT+2]	=
		{ "R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7", "R8", "R9", "R10/Stack Limit Pointer", "R11/Frame Pointer", "R12/Intra-Call Scratch Register", "R13/Stack Pointer (It is an alias for either MSP or PSP, depending on the value of bit ASPSEL in register CONTROL)", "R14/Link Register (Sub-Routine Return Address)", "R15/Program Counter", "Master Stack Pointer (Always used by interrupts and exceptions, and used by thread code if bit ASPSEL in register CONTROL is 0)", "Process Stack Pointer (Used by thread code if bit ASPSEL in register CONTROL is 1)" };

	typedef struct
		{
		wchar_t     Label;
		unsigned	Bit;
		wchar_t		CharOn;
		wchar_t		CharOff;
		const char	*Tip;
		} creationinfo_t;

	static const creationinfo_t Info[10]	=
		{
		[0]	= { .Label = 'N', .Bit = 31, .CharOn = 'N', .CharOff = '-', .Tip = "Negative Flag ( 0 = - = Result of last operation was not negative, 1 = N = Result of last operation was negative )" },
		[1]	= { .Label = 'Z', .Bit = 30, .CharOn = 'Z', .CharOff = '-', .Tip = "Zero Flag ( 0 = - = Result of last operation was not zero, 1 = Z = Result of last operation was zero )"			 },
		[2]	= { .Label = 'C', .Bit = 29, .CharOn = 'C', .CharOff = '-', .Tip = "Carry Flag ( 0 = - = Last operation did not generate a carry bit, 1 = C = Last operation generated a carry bit )" },
		[3]	= { .Label = 'V', .Bit = 28, .CharOn = 'V', .CharOff = '-', .Tip = "Overflow Flag ( 0 = - = Last arithmetic operation did not overflow, 1 = V = Last arithmetic operation overflowed )" },
		[4]	= { .Label = 'T', .Bit = 24, .CharOn = 'T', .CharOff = 'A', .Tip = "Thumb Execution Mode Bit ( 0 = A = Executing ARM Instructions, 1 = T = Executing Thumb Instructions )" }
		};

	int 	i;

	m_SelectedLine		= 0;
	m_SelectedColumn	= 0;
	m_SelectedBit		= 0;

	wxBoxSizer		*bSizer3;
	bSizer3			= new wxBoxSizer( wxVERTICAL );

	m_RegistersPanelInternal = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );

	wxFlexGridSizer	*fgSizer2;
	fgSizer2		= new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizer2->SetFlexibleDirection( wxHORIZONTAL );
	fgSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	for( i = 0; i < 4; i++ )
		{
		m_HeaderTexts[i] = new wxStaticText( m_RegistersPanelInternal, wxID_ANY, HeaderTexts[i], wxDefaultPosition, wxSize( -1,17 ), 0 );
		m_HeaderTexts[i]->Wrap( -1 );
		fgSizer2->Add( m_HeaderTexts[i], 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_BOTTOM, 5 );
		}

	for( i = 0; i < ARM_REGISTERS_COUNT; i++ )
		{
		m_TextRegisterNames[i] = new wxStaticText( m_RegistersPanelInternal, wxID_ANY, RegistersNames[i], wxDefaultPosition, wxSize( 40,17 ), wxALIGN_LEFT );
		m_TextRegisterNames[i]->Wrap( -1 );
		m_TextRegisterNames[i]->SetToolTip( RegistersTips[i] );
		fgSizer2->Add( m_TextRegisterNames[i], 1, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxBOTTOM|wxRIGHT, 1 );

		for(  int j = 0; j < 3; j++ )
			{
			m_RegsEditCtrl[i][j] = new RegisterEditCtrl( this, i, j, m_RegistersPanelInternal, wxID_ANY, _(""), wxDefaultPosition, wxSize( 92 + 8 * j, 17 ), wxTE_PROCESS_ENTER | wxTE_RIGHT );
#ifdef __WXGTK__
			if( !m_RegsEditCtrl[i][0]->HasFlag( wxTE_MULTILINE ) )
#endif
				m_RegsEditCtrl[i][j]->SetMaxLength( 10 + j );
			m_RegsEditCtrl[i][j]->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Courier") ) );
			m_RegsEditCtrl[i][j]->SetToolTip( wxString( RegistersTips[i] ) /*+ _(" - ") + HeaderTexts[j+1] + _(" Value")*/ );

			fgSizer2->Add( m_RegsEditCtrl[i][j], 0, wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL|wxBOTTOM|wxLEFT|wxRIGHT, 1 );

			m_RegsEditCtrl[i][j]->Connect( wxEVT_CHAR, wxKeyEventHandler( RegisterEditCtrl::OnChar ));
			m_RegsEditCtrl[i][j]->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler( RegisterEditCtrl::OnKillFocus ));
			m_RegsEditCtrl[i][j]->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( RegisterEditCtrl::OnLeftDown ));
			m_RegsEditCtrl[i][j]->Connect( wxEVT_SET_FOCUS, wxFocusEventHandler( RegisterEditCtrl::OnSetFocus ));
			m_RegsEditCtrl[i][j]->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( RegisterEditCtrl::DummyMouseEvent ));
			m_RegsEditCtrl[i][j]->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( RegisterEditCtrl::OnRightDown ));
			}
		}

	m_RegistersPanelInternal->SetSizer( fgSizer2 );
	m_RegistersPanelInternal->Layout();
	fgSizer2->Fit( m_RegistersPanelInternal );
	bSizer3->Add( m_RegistersPanelInternal, 0, wxALL|wxEXPAND, 5 );

	m_StaticLine0 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer3->Add( m_StaticLine0, 0, wxEXPAND|wxLEFT|wxRIGHT, 5 );

	m_RegistersPanelInternal2 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );

	wxFlexGridSizer	*fgSizer3;
	fgSizer3		= new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizer3->SetFlexibleDirection( wxHORIZONTAL );
	fgSizer3->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	for( ; i < ARM_REGISTERS_COUNT + 2; i++ )
		{
		m_TextRegisterNames[i] = new wxStaticText( m_RegistersPanelInternal2, wxID_ANY, RegistersNames[i], wxDefaultPosition, wxSize( 40,17 ), wxALIGN_LEFT );
		m_TextRegisterNames[i]->Wrap( -1 );
		m_TextRegisterNames[i]->SetToolTip( RegistersTips[i] );
		fgSizer3->Add( m_TextRegisterNames[i], 1, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxBOTTOM|wxRIGHT, 1 );

		for(  int j = 0; j < 3; j++ )
			{
			m_RegsEditCtrl[i][j] = new RegisterEditCtrl( this, i, j, m_RegistersPanelInternal2, wxID_ANY, _(""), wxDefaultPosition, wxSize( 92 + 8 * j, 17 ), wxTE_PROCESS_ENTER | wxTE_RIGHT );
#ifdef __WXGTK__
			if( !m_RegsEditCtrl[i][0]->HasFlag( wxTE_MULTILINE ) )
#endif
				m_RegsEditCtrl[i][j]->SetMaxLength( 10 + j );
			m_RegsEditCtrl[i][j]->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Courier") ) );
			m_RegsEditCtrl[i][j]->SetToolTip( wxString( RegistersTips[i] ) /*+ _(" - ") + HeaderTexts[j+1] + _(" Value")*/ );

			fgSizer3->Add( m_RegsEditCtrl[i][j], 0, wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL|wxBOTTOM|wxLEFT|wxRIGHT, 1 );

			m_RegsEditCtrl[i][j]->Connect( wxEVT_CHAR, wxKeyEventHandler( RegisterEditCtrl::OnChar ));
			m_RegsEditCtrl[i][j]->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler( RegisterEditCtrl::OnKillFocus ));
			m_RegsEditCtrl[i][j]->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( RegisterEditCtrl::OnLeftDown ));
			m_RegsEditCtrl[i][j]->Connect( wxEVT_SET_FOCUS, wxFocusEventHandler( RegisterEditCtrl::OnSetFocus ));
			m_RegsEditCtrl[i][j]->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( RegisterEditCtrl::DummyMouseEvent ));
			m_RegsEditCtrl[i][j]->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( RegisterEditCtrl::OnRightDown ));
			}
		}

	m_RegistersPanelInternal2->SetSizer( fgSizer3 );
	m_RegistersPanelInternal2->Layout();
	fgSizer3->Fit( m_RegistersPanelInternal2 );
	bSizer3->Add( m_RegistersPanelInternal2, 0, wxLEFT|wxRIGHT|wxTOP|wxEXPAND, 5 );

	m_StaticLine = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer3->Add( m_StaticLine, 0, wxEXPAND|wxALL, 5 );

	m_CPSRPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxFlexGridSizer* fgSizerCPSR1;
	fgSizerCPSR1 = new wxFlexGridSizer( 0, 9, 0, 0 );
	fgSizerCPSR1->SetFlexibleDirection( wxBOTH );
	fgSizerCPSR1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_StaticTextCPSR0 = new wxStaticText( m_CPSRPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_StaticTextCPSR0->Wrap( -1 );
	fgSizerCPSR1->Add( m_StaticTextCPSR0, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	for( i = 0; i < 5; i++ )
		{
		m_HeaderTextsCPSR[i] = new wxStaticText( m_CPSRPanel, wxID_ANY, Info[i].Label, wxDefaultPosition, wxDefaultSize, 0 );
		m_HeaderTextsCPSR[i]->Wrap( -1 );
		m_HeaderTextsCPSR[i]->SetToolTip( Info[i].Tip );
		fgSizerCPSR1->Add( m_HeaderTextsCPSR[i], 0, wxALIGN_CENTER_VERTICAL | wxALIGN_CENTER_HORIZONTAL /*| ( i == 0 ? wxLEFT : 0 )*/, 5 );

		if( i == 3 )
			{
			m_StaticTextReserved0 = new wxStaticText( m_CPSRPanel, wxID_ANY, _("Rsvd"), wxDefaultPosition, wxDefaultSize, 0 );
			m_StaticTextReserved0->Wrap( -1 );
			m_StaticTextReserved0->SetToolTip( _("Reserved Bits. Must Be Kept As Zeroes") );
			fgSizerCPSR1->Add( m_StaticTextReserved0, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
			}
		else if( i == 4 )
			{
			m_StaticTextReserved1 = new wxStaticText( m_CPSRPanel, wxID_ANY, _("Reserved"), wxDefaultPosition, wxDefaultSize, 0 );
			m_StaticTextReserved1->Wrap( -1 );
			m_StaticTextReserved1->SetToolTip( _("Reserved Bits. Must Be Kept As Zeroes") );
			fgSizerCPSR1->Add( m_StaticTextReserved1, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
			}
		}

	m_HeaderTextsCPSR[5] = new wxStaticText( m_CPSRPanel, wxID_ANY, _("Ex. Num."), wxDefaultPosition, wxDefaultSize, 0 );
	m_HeaderTextsCPSR[5]->Wrap( -1 );
	m_HeaderTextsCPSR[5]->SetToolTip( _("Exception Number. 000000 = Executing Thread Code. Other Values = Exception Number") );
	fgSizerCPSR1->Add( m_HeaderTextsCPSR[5], 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );

	m_StaticTextCPSR1 = new wxStaticText( m_CPSRPanel, wxID_ANY, _("CPSR"), wxDefaultPosition, wxSize( 55, -1 ), 0 );
	m_StaticTextCPSR1->Wrap( -1 );
	m_StaticTextCPSR1->SetToolTip( _("Current Processor Status Register") );

	fgSizerCPSR1->Add( m_StaticTextCPSR1, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5 );

	for( i = 0; i < 5; i++ )
		{
		m_BitsEditCtrl[i] = new BitEditCtrl( this, i, Info[i].Bit, Info[i].CharOn, Info[i].CharOff, m_CPSRPanel, wxID_ANY, Info[i].CharOn, wxDefaultPosition, wxSize( 13,18 ), wxTE_PROCESS_ENTER | wxTE_CENTER );
		m_BitsEditCtrl[i]->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Courier New") ) );
		m_BitsEditCtrl[i]->SetToolTip( Info[i].Tip );

		fgSizerCPSR1->Add( m_BitsEditCtrl[i], 0, wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL /*| ( i == 0 ? wxLEFT : 0 )*/, 5 );

		m_BitsEditCtrl[i]->Connect( wxEVT_CHAR, wxKeyEventHandler( BitEditCtrl::OnChar ));
		m_BitsEditCtrl[i]->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( CPSREditCtrl::OnLeftDown ));
		m_BitsEditCtrl[i]->Connect( wxEVT_SET_FOCUS, wxFocusEventHandler( CPSREditCtrl::OnSetFocus ));
		m_BitsEditCtrl[i]->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler( CPSREditCtrl::OnKillFocus ));
		m_BitsEditCtrl[i]->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( BitEditCtrl::OnLeftDClick ));
		m_BitsEditCtrl[i]->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( CPSREditCtrl::DummyMouseEvent ));

		if( i == 3 )
			{
			m_StaticTextReserved2 = new wxStaticText( m_CPSRPanel, wxID_ANY, _("000"), wxDefaultPosition, wxSize( 27,18 ), wxALIGN_CENTER_HORIZONTAL|wxBORDER_SIMPLE );
			m_StaticTextReserved2->Wrap( -1 );
			m_StaticTextReserved2->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Courier New") ) );
			m_StaticTextReserved2->SetToolTip( _("Reserved Bits. Must Be Kept As Zeroes") );

			fgSizerCPSR1->Add( m_StaticTextReserved2, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
			}
		else if( i == 4 )
			{
			m_StaticTextReserved3 = new wxStaticText( m_CPSRPanel, wxID_ANY, _("000000000000000000"), wxDefaultPosition, wxSize( 132,18 ), wxALIGN_CENTER_HORIZONTAL|wxBORDER_SIMPLE );
			m_StaticTextReserved3->Wrap( -1 );
			m_StaticTextReserved3->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Courier New") ) );
			m_StaticTextReserved3->SetToolTip( _("Reserved Bits. Must Be Kept As Zeroes") );

			fgSizerCPSR1->Add( m_StaticTextReserved3, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );
			}
		}

	m_BitsEditCtrl[5] = new BitFieldEditCtrl( this, 5, 0, 6, m_CPSRPanel, wxID_ANY, _("000000"), wxDefaultPosition, wxSize( 50,18 ), wxTE_PROCESS_ENTER | wxTE_RIGHT );
	m_BitsEditCtrl[5]->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Courier New") ) );
	m_BitsEditCtrl[5]->SetToolTip( _("Exception Number. 000000 = Executing Thread Code. Other Values = Exception Number") );

	m_BitsEditCtrl[5]->Connect( wxEVT_CHAR, wxKeyEventHandler( BitFieldEditCtrl::OnChar ));
	m_BitsEditCtrl[5]->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( CPSREditCtrl::OnLeftDown ));
	m_BitsEditCtrl[5]->Connect( wxEVT_SET_FOCUS, wxFocusEventHandler( CPSREditCtrl::OnSetFocus ));
	m_BitsEditCtrl[5]->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler( CPSREditCtrl::OnKillFocus ));
	m_BitsEditCtrl[5]->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( CPSREditCtrl::DummyMouseEvent ));
	m_BitsEditCtrl[5]->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( CPSREditCtrl::DummyMouseEvent ));

	fgSizerCPSR1->Add( m_BitsEditCtrl[5], 0, wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_CPSRPanel->SetSizer( fgSizerCPSR1 );
	m_CPSRPanel->Layout();
	fgSizerCPSR1->Fit( m_CPSRPanel );
	bSizer3->Add( m_CPSRPanel, 0, wxRIGHT|wxLEFT|wxEXPAND, 5 );

	m_PRIMASKPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxFlexGridSizer* fgSizerPRIMASK;
	fgSizerPRIMASK = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizerPRIMASK->SetFlexibleDirection( wxBOTH );
	fgSizerPRIMASK->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_StaticPRIMASK01 = new wxStaticText( m_PRIMASKPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_StaticPRIMASK01->Wrap( -1 );
	fgSizerPRIMASK->Add( m_StaticPRIMASK01, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_StaticTextReserved01 = new wxStaticText( m_PRIMASKPanel, wxID_ANY, _("Reserved"), wxDefaultPosition, wxDefaultSize, 0 );
	m_StaticTextReserved01->Wrap( -1 );
	m_StaticTextReserved01->SetToolTip( _("Reserved Bits. Must Be Kept As Zeroes") );
	fgSizerPRIMASK->Add( m_StaticTextReserved01, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );

	m_StaticTextP = new wxStaticText( m_PRIMASKPanel, wxID_ANY, _("P"), wxDefaultPosition, wxDefaultSize, 0 );
	m_StaticTextP->Wrap( -1 );
	m_StaticTextP->SetToolTip( _("Prioritized Interrupts Disable Bit. 0 = Interrupts Enabled, 1 = Interrupts Disabled") );
	fgSizerPRIMASK->Add( m_StaticTextP, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );

	m_StaticTextPRIMASK02 = new wxStaticText( m_PRIMASKPanel, wxID_ANY, _("PRIMASK"), wxDefaultPosition, wxSize( 55,-1 ), 0 );
	m_StaticTextPRIMASK02->Wrap( -1 );
	m_StaticTextPRIMASK02->SetToolTip( _("Prioritized Interrupts Disable Register") );

	fgSizerPRIMASK->Add( m_StaticTextPRIMASK02, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5 );

	m_StaticTextReserved21 = new wxStaticText( m_PRIMASKPanel, wxID_ANY, _("0000000000000000000000000000000"), wxDefaultPosition, wxSize( 224,18 ), wxALIGN_CENTER_HORIZONTAL|wxBORDER_SIMPLE );
	m_StaticTextReserved21->Wrap( -1 );
	m_StaticTextReserved21->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Courier New") ) );
	m_StaticTextReserved21->SetToolTip( _("Reserved Bits. Must Be Kept As Zeroes") );

	fgSizerPRIMASK->Add( m_StaticTextReserved21, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );

	m_TextCtrlPRIMASK = new wxTextCtrl( m_PRIMASKPanel, wxID_ANY, _("0"), wxDefaultPosition, wxSize( 13,18 ), wxTE_CENTER );
	m_TextCtrlPRIMASK->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Courier New") ) );
	m_TextCtrlPRIMASK->SetToolTip( _("Prioritized Interrupts Disable Bit. 0 = Interrupts Enabled, 1 = Interrupts Disabled") );

	fgSizerPRIMASK->Add( m_TextCtrlPRIMASK, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );


	m_PRIMASKPanel->SetSizer( fgSizerPRIMASK );
	m_PRIMASKPanel->Layout();
	fgSizerPRIMASK->Fit( m_PRIMASKPanel );
	bSizer3->Add( m_PRIMASKPanel, 0, wxEXPAND|wxRIGHT|wxLEFT, 5 );

	m_CONTROLPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxFlexGridSizer* fgSizerCONTROL;
	fgSizerCONTROL = new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizerCONTROL->SetFlexibleDirection( wxBOTH );
	fgSizerCONTROL->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_StaticTextCONTROL = new wxStaticText( m_CONTROLPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_StaticTextCONTROL->Wrap( -1 );
	fgSizerCONTROL->Add( m_StaticTextCONTROL, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_StaticTextReserved11 = new wxStaticText( m_CONTROLPanel, wxID_ANY, _("Reserved"), wxDefaultPosition, wxDefaultSize, 0 );
	m_StaticTextReserved11->Wrap( -1 );
	m_StaticTextReserved11->SetToolTip( _("Reserved Bits. Must Be Kept As Zeroes") );
	fgSizerCONTROL->Add( m_StaticTextReserved11, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );

	m_StaticTextS = new wxStaticText( m_CONTROLPanel, wxID_ANY, _("S"), wxDefaultPosition, wxDefaultSize, 0 );
	m_StaticTextS->Wrap( -1 );
	m_StaticTextS->SetToolTip( _("ASPSEL, Active SP Select Bit. 0 = SP is an alias for MSP, 1 = SP is an alias for PSP. When in Exception mode can only be 0") );
	fgSizerCONTROL->Add( m_StaticTextS, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );

	m_StaticTextM111 = new wxStaticText( m_CONTROLPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_StaticTextM111->Wrap( -1 );
	fgSizerCONTROL->Add( m_StaticTextM111, 0, 0, 5 );

	m_StaticTextCPSR111 = new wxStaticText( m_CONTROLPanel, wxID_ANY, _("CONTROL"), wxDefaultPosition, wxSize( 55,-1 ), 0 );
	m_StaticTextCPSR111->Wrap( -1 );
	m_StaticTextCPSR111->SetToolTip( _("Control Register") );

	fgSizerCONTROL->Add( m_StaticTextCPSR111, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5 );

	m_StaticTextReserved211 = new wxStaticText( m_CONTROLPanel, wxID_ANY, _("000000000000000000000000000000"), wxDefaultPosition, wxSize( 216,18 ), wxALIGN_CENTER_HORIZONTAL|wxBORDER_SIMPLE );
	m_StaticTextReserved211->Wrap( -1 );
	m_StaticTextReserved211->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Courier New") ) );
	m_StaticTextReserved211->SetToolTip( _("Reserved Bits. Must Be Kept As Zeroes") );

	fgSizerCONTROL->Add( m_StaticTextReserved211, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );

	m_TextCtrlS = new wxTextCtrl( m_CONTROLPanel, wxID_ANY, _("0"), wxDefaultPosition, wxSize( 13,18 ), wxTE_CENTER );
	m_TextCtrlS->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Courier New") ) );
	m_TextCtrlS->SetToolTip( _("ASPSEL, Active SP Select Bit. 0 = SP is an alias for MSP, 1 = SP is an alias for PSP. When in Exception Mode can only be 0") );

	fgSizerCONTROL->Add( m_TextCtrlS, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_StaticTextReserved2111 = new wxStaticText( m_CONTROLPanel, wxID_ANY, _("0"), wxDefaultPosition, wxSize( 12,18 ), wxALIGN_CENTER_HORIZONTAL|wxBORDER_SIMPLE );
	m_StaticTextReserved2111->Wrap( -1 );
	m_StaticTextReserved2111->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Courier New") ) );
	m_StaticTextReserved2111->SetToolTip( _("Reserved Bit. Must Be Kept As Zero") );

	fgSizerCONTROL->Add( m_StaticTextReserved2111, 0, wxALIGN_CENTER_VERTICAL, 5 );


	m_CONTROLPanel->SetSizer( fgSizerCONTROL );
	m_CONTROLPanel->Layout();
	fgSizerCONTROL->Fit( m_CONTROLPanel );
	bSizer3->Add( m_CONTROLPanel, 0, wxRIGHT|wxLEFT|wxDOWN|wxEXPAND, 5 );

	this->SetSizer( bSizer3 );
	this->Layout();
	}
//===============================================================================
void RegistersPanel::SelectCell( unsigned line, unsigned column )
	{
	if( line >= ARM_REGISTERS_COUNT + 2 || column >= COLUMNS_COUNT )
		return;

	m_SelectedLine		= line;
	m_SelectedColumn	= column;

	m_RegsEditCtrl[line][column]->SetFocus();
	}
//===============================================================================
void RegistersPanel::SelectBit( unsigned index )
	{
	if( index > 5 )
		return;

	m_SelectedBit	= index;

	m_BitsEditCtrl[index]->SetFocus();
	}
//===============================================================================
void RegistersPanel::RedrawRegisters()
	{
	static const wxColour	*Colours[]	= { nullptr, wxCYAN, wxRED, wxYELLOW };
	cpucore_t				Core;

	wxGetApp().SimulatorThread->GetCoreDiff( Core );

	for( int i = 0; i < ARM_REGISTERS_COUNT + 2; i++ )
		{
		wxString	Text;
		uint32_t	Value, Written;

		Value	= Core.Regs[i];
		Written	= Core.Written[i];

		Text.Printf( "0x%08X", Value );
		m_RegsEditCtrl[i][0]->SetValue( Text );
		Text.Printf( "%lu", Value );
		m_RegsEditCtrl[i][1]->SetValue( Text );
		Text.Printf( "%+ld", Value );
		m_RegsEditCtrl[i][2]->SetValue( Text );

		const wxColour	*Colour	= Colours[Written % 3];

		if( Colour != nullptr )
			m_RegsEditCtrl[i][0]->SetBkColour( Colour );
		else
			m_RegsEditCtrl[i][0]->ClearBkColour();

		m_RegsEditCtrl[i][0]->Refresh();
		m_RegsEditCtrl[i][1]->Refresh();
		m_RegsEditCtrl[i][2]->Refresh();
		}

	for( int i = 0; i < 5; i++ )
		m_BitsEditCtrl[i]->RedrawBit( Core.CPSR.Value, Core.Written[CPSR], Core.CPSRWrittenByApp );

	m_BitsEditCtrl[5]->RedrawBit( Core.CPSR.Value, Core.Written[CPSR], Core.CPSRWrittenByApp );
	}
//===============================================================================
