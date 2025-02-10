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
#include <stdint.h>
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
#include <wx/panel.h>
#include <wx/radiobut.h>
#include <wx/scrolbar.h>
//==============================================================================
#include "Simulator.h"
//==============================================================================
#define	MEMORY_DUMP_LINES			9
#define	MEMORY_DUMP_BYTE_COLUMNS   16
//==============================================================================
class	HexEdit;
class	CharEdit;
class	DumpEditCtrl;
class	InertPanel;
//==============================================================================
class MemoryDumpPanel : public wxPanel
	{
	//--------------------------------------------------------------------------
	protected:
	//--------------------------------------------------------------------------
		typedef struct
			{
			wxStaticText	*Address;
			wxStaticText	*Hexadecimal[MEMORY_DUMP_BYTE_COLUMNS];
			wxStaticText	*Spacer;
			wxStaticText	*Characters[MEMORY_DUMP_BYTE_COLUMNS];
			} header_t;

		typedef struct
			{
			wxStaticText	*Address;
			DumpEditCtrl	*Hexadecimal[MEMORY_DUMP_BYTE_COLUMNS];
			wxStaticText	*Spacer;
			DumpEditCtrl	*Characters[MEMORY_DUMP_BYTE_COLUMNS];
			} line_t;

		uint8_t				m_Buffer[MEMORY_DUMP_BYTE_COLUMNS * MEMORY_DUMP_LINES];

		uint32_t			m_StartAddress;
		uint32_t			m_FirstPageStartAddress;
		uint32_t			m_LastPageStartAddress;

		uint32_t			m_BufferBaseAddress;
		uint32_t			m_BufferLength;

		wxPanel				*m_panel4;
		wxRadioButton		*m_RadioBtnROM;
		wxRadioButton		*m_RadioBtnRAM;
		wxPanel				*m_panel5;
		wxRadioButton		*m_RadioBtnByte;
		wxRadioButton		*m_RadioBtnHalf;
		wxRadioButton		*m_RadioBtnWord;

		header_t			m_Header;
		line_t				m_Lines[MEMORY_DUMP_LINES];
		wxScrollBar			*m_ScrollBar;

		unsigned			m_WidthBytes;
		unsigned			m_ColumnWidth;
		bool				m_BigEndian;
		unsigned			m_AddressNibbles;
		wchar_t				m_Placeholder;
		bool				m_Editable;

		unsigned			m_LinesCount;
		unsigned			m_ColumnsCount;

		unsigned			m_CurrentSide;
		unsigned			m_CurrentLine;
		unsigned			m_CurrentColumn;
		DumpEditCtrl		*m_CurrentControl;
		bool				m_IsActive;

		void				(*m_PanelChangedCallBack)( uint32_t startaddress, uint32_t endadress );
	//--------------------------------------------------------------------------
		void				UpdateHeader			();
		bool				UpdateAddress			( unsigned line, uint32_t address );
		bool				UpdateHexCell			( unsigned line, unsigned column, uint32_t address, const wxColour *colour );
		bool				UpdateCharCell			( unsigned line, unsigned column, uint32_t address, const wxColour *colour );
	//--------------------------------------------------------------------------
	public:
		typedef struct
			{
			unsigned		Side;
			unsigned		Line;
			unsigned		Column;
			bool			BigEndian;
			uint32_t		StartAddress;
			} panelstate_t;
	//--------------------------------------------------------------------------
							MemoryDumpPanel			( unsigned			addressbits,
													  unsigned			widthbytes,
													  unsigned			columnwidth,
													  wchar_t			placeholder,
													  wxWindow			*parent,
													  wxWindowID		id		= wxID_ANY,
													  const wxPoint		&pos	= wxDefaultPosition,
													  const wxSize		&size	= wxSize( 500,300 ),
													  long				style	= wxTAB_TRAVERSAL,
													  const wxString	&name	= wxEmptyString );
							~MemoryDumpPanel		() {};
	//--------------------------------------------------------------------------
		virtual void		OnChar					( wxKeyEvent &event );
		virtual void		OnSetFocusScrollbar		( wxFocusEvent &event );
		virtual void		OnScrollChanged			( wxScrollEvent& event );
		virtual void		OnMouseWheel			( wxMouseEvent& event );
	//--------------------------------------------------------------------------
		sim_address_t		GetCellAddress			( unsigned side, unsigned line, unsigned column );
		bool				SetBuffer				( uint32_t bufferbaseaddress, uint32_t bufferlength, bool editable );
		void				SetPanelChangedCallBack	( void (*callback)( uint32_t, uint32_t ) );
		void				DumpRedraw				();
		bool				SetEditable				( bool editable );
	//--------------------------------------------------------------------------
		void				SetActive				( bool isactive );
		void				SetEndianness			( bool endianness ) { m_BigEndian = endianness; UpdateHeader(); DumpRedraw(); };
		bool				GetEndianness			() { return m_BigEndian; };
		void				GetState				( panelstate_t &state );
		void				SetState				( const panelstate_t &state );
		bool				ChangeStartAddress		( uint32_t newaddress );
		bool				CenterAddress			( uint32_t address );
		bool				IsEditable				() { return m_Editable; };
		bool				IsHexCellValid			( unsigned line, unsigned column );
		bool				IsCharCellValid			( unsigned line, unsigned column );
		void				SwitchSide				();
		void				Home					( DumpEditCtrl *Control );
		void				End						( DumpEditCtrl *Control, unsigned bytes );
		void				ControlHome				( DumpEditCtrl *Control );
		void				ControlEnd				( DumpEditCtrl *Control, unsigned bytes );
		void				PageUp					( DumpEditCtrl *Control );
		void				PageDown				( DumpEditCtrl *Control );
		bool				MoveUp					( DumpEditCtrl *Control );
		bool				MoveDown				( DumpEditCtrl *Control );
		bool				MoveLeft				( DumpEditCtrl *Control, unsigned bytes );
		bool				MoveRight				( DumpEditCtrl *Control, unsigned bytes );
		bool				SelectCell				( unsigned side, unsigned line, unsigned column, bool setfocus );
		void				SetCellValueHex			( unsigned line, unsigned column, unsigned long int value );
		void				SetCellValueChar		( unsigned line, unsigned column, unsigned long int value );
		void				SetCellBackGround		();
		void				ClearCellBackGround		();
		unsigned			GetCurrentSide			() { return m_CurrentSide; };
		unsigned			GetCurrentLine			() { return m_CurrentLine; };
		unsigned			GetCurrentColumn		() { return m_CurrentColumn; };
		unsigned			GetColumnWidth			() { return m_ColumnWidth; };
		void				SetCellFocus			();
		void				SetCurrentCell			( unsigned side, unsigned line, unsigned column );
	//--------------------------------------------------------------------------
	};
//==============================================================================
class MemoryDumpPanelByte : public MemoryDumpPanel
	{
	//--------------------------------------------------------------------------
	public:
	//--------------------------------------------------------------------------
							MemoryDumpPanelByte		( /*unsigned			addressbits,
													  unsigned			columnwidth,
													  wchar_t			placeholder,*/
													  wxWindow			*parent,
													  wxWindowID		id		= wxID_ANY,
													  const wxPoint		&pos	= wxDefaultPosition,
													  const wxSize		&size	= wxSize( 500,300 ),
													  long				style	= wxTAB_TRAVERSAL,
													  const wxString	&name	= wxEmptyString )
													  : MemoryDumpPanel( 32, 16, 1, 0, parent, id, pos, size, style, name ) {};
							~MemoryDumpPanelByte	() {};
	//--------------------------------------------------------------------------
	};
//==============================================================================
class MemoryDumpPanelHalf : public MemoryDumpPanel
	{
	//--------------------------------------------------------------------------
	public:
	//--------------------------------------------------------------------------
							MemoryDumpPanelHalf		( /*unsigned			addressbits,
													  unsigned			columnwidth,
													  wchar_t			placeholder,*/
													  wxWindow			*parent,
													  wxWindowID		id		= wxID_ANY,
													  const wxPoint		&pos	= wxDefaultPosition,
													  const wxSize		&size	= wxSize( 500,300 ),
													  long				style	= wxTAB_TRAVERSAL,
													  const wxString	&name	= wxEmptyString )
													  : MemoryDumpPanel( 32, 16, 2, 0, parent, id, pos, size, style, name ) {};
							~MemoryDumpPanelHalf	() {};
	//--------------------------------------------------------------------------
	};
//==============================================================================
class MemoryDumpPanelWord : public MemoryDumpPanel
	{
	//--------------------------------------------------------------------------
	public:
	//--------------------------------------------------------------------------
							MemoryDumpPanelWord		( /*unsigned			addressbits,
													  unsigned			columnwidth,
													  wchar_t			placeholder,*/
													  wxWindow			*parent,
													  wxWindowID		id		= wxID_ANY,
													  const wxPoint		&pos	= wxDefaultPosition,
													  const wxSize		&size	= wxSize( 500,300 ),
													  long				style	= wxTAB_TRAVERSAL,
													  const wxString	&name	= wxEmptyString )
													  : MemoryDumpPanel( 32, 16, 4, 0, parent, id, pos, size, style, name ) {};
							~MemoryDumpPanelWord	() {};
	//--------------------------------------------------------------------------
	};
//==============================================================================
