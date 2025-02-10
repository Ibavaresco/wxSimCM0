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
class InertPanel;
class MemoryDumpPanelByte;
class MemoryDumpPanelHalf;
class MemoryDumpPanelWord;

#include <wx/string.h>
#include <wx/radiobut.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/panel.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class MemoryPanelBase
///////////////////////////////////////////////////////////////////////////////
class MemoryPanelBase : public wxPanel
{
	private:

	protected:
		enum
		{
			wxID_BUTTON_BYTE = 7000,
			wxID_BUTTON_HALF,
			wxID_BUTTON_WORD,
			wxID_BUTTON_ROM,
			wxID_BUTTON_RAM,
			wxID_BUTTON_BIG,
			wxID_BUTTON_LITTLE,
		};

		wxPanel* m_panel5;
		wxRadioButton* m_radioBtnByte;
		wxRadioButton* m_radioBtnHalf;
		wxRadioButton* m_radioBtnWord;
		wxPanel* m_panel4;
		wxRadioButton* m_RadioBtnROM;
		wxRadioButton* m_RadioBtnRAM;
		wxPanel* m_panel8;
		wxRadioButton* m_RadioBtnBigEndian;
		wxRadioButton* m_RadioBtnLittleEndian;
		InertPanel* m_panel41;
		MemoryDumpPanelByte* m_PanelByte;
		MemoryDumpPanelHalf* m_PanelHalf;
		MemoryDumpPanelWord* m_PanelWord;

		// Virtual event handlers, override them in your derived class
		virtual void OnRadioButton( wxCommandEvent& event ) { event.Skip(); }


	public:

		MemoryPanelBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 532,300 ), long style = 0, const wxString& name = wxEmptyString );

		~MemoryPanelBase();

};

