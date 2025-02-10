///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-0-g80c4cb6)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "DisassemblyPanel.h"
#include "RegistersPanel.h"
#include "ThumbMemoryPanel.h"

#include "wxThumbFrameBase.h"

///////////////////////////////////////////////////////////////////////////

wxThumbFrameBase::wxThumbFrameBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( 800,600 ), wxDefaultSize );

	m_MenuBar = new wxMenuBar( 0 );
	m_MenuFile = new wxMenu();
	wxMenuItem* m_MenuItemNew;
	m_MenuItemNew = new wxMenuItem( m_MenuFile, wxID_ANY, wxString( _("&New") ) , wxEmptyString, wxITEM_NORMAL );
	m_MenuFile->Append( m_MenuItemNew );

	wxMenuItem* m_MenuItemLoad;
	m_MenuItemLoad = new wxMenuItem( m_MenuFile, wxID_ANY, wxString( _("&Load ELF File...") ) , wxEmptyString, wxITEM_NORMAL );
	m_MenuFile->Append( m_MenuItemLoad );

	m_MenuFile->AppendSeparator();

	wxMenuItem* m_MenuItemExit;
	m_MenuItemExit = new wxMenuItem( m_MenuFile, wxID_ANY, wxString( _("E&xit") ) + wxT('\t') + wxT("Alt-F4"), wxEmptyString, wxITEM_NORMAL );
	m_MenuFile->Append( m_MenuItemExit );

	m_MenuBar->Append( m_MenuFile, _("&File") );

	m_MenuRun = new wxMenu();
	wxMenuItem* m_MenuItemStepInto;
	m_MenuItemStepInto = new wxMenuItem( m_MenuRun, wxID_ANY, wxString( _("Step &Into") ) + wxT('\t') + wxT("F7"), _("Executes the instruction currently pointed to by R15/PC."), wxITEM_NORMAL );
	m_MenuRun->Append( m_MenuItemStepInto );

	wxMenuItem* m_MenuItemStepOver;
	m_MenuItemStepOver = new wxMenuItem( m_MenuRun, wxID_ANY, wxString( _("Step &Over") ) + wxT('\t') + wxT("F8"), _("Executes the instruction currently pointed to by R15/PC. If it is a call to a subroutine, stops immediately after the subroutine returns."), wxITEM_NORMAL );
	m_MenuRun->Append( m_MenuItemStepOver );

	wxMenuItem* m_MenuItemAnimate;
	m_MenuItemAnimate = new wxMenuItem( m_MenuRun, wxID_ANY, wxString( _("&Animate") ) + wxT('\t') + wxT("Alt-A"), wxEmptyString, wxITEM_NORMAL );
	m_MenuRun->Append( m_MenuItemAnimate );

	wxMenuItem* m_MenuItemRun;
	m_MenuItemRun = new wxMenuItem( m_MenuRun, wxID_ANY, wxString( _("&Run") ) + wxT('\t') + wxT("F9"), _("Runs the program continuously until the user pauses it or an error occurs."), wxITEM_NORMAL );
	m_MenuRun->Append( m_MenuItemRun );

	wxMenuItem* m_MenuItemMaxSpeed;
	m_MenuItemMaxSpeed = new wxMenuItem( m_MenuRun, wxID_ANY, wxString( _("Run at &Max. Speed") ) + wxT('\t') + wxT("Alt-M"), wxEmptyString, wxITEM_NORMAL );
	m_MenuRun->Append( m_MenuItemMaxSpeed );

	m_MenuRun->AppendSeparator();

	wxMenuItem* m_MenuItemPause;
	m_MenuItemPause = new wxMenuItem( m_MenuRun, wxID_ANY, wxString( _("&Pause") ) + wxT('\t') + wxT("Alt-P"), _("Pauses a running program."), wxITEM_NORMAL );
	m_MenuRun->Append( m_MenuItemPause );

	wxMenuItem* m_MenuItemResetCPU;
	m_MenuItemResetCPU = new wxMenuItem( m_MenuRun, wxID_ANY, wxString( _("Reset &CPU") ) + wxT('\t') + wxT("Ctrl-R"), _("Resets the CPU without changing the memory contents."), wxITEM_NORMAL );
	m_MenuRun->Append( m_MenuItemResetCPU );

	wxMenuItem* m_MenuItemFullReset;
	m_MenuItemFullReset = new wxMenuItem( m_MenuRun, wxID_ANY, wxString( _("&Full Reset") ) + wxT('\t') + wxT("Alt-R"), _("Resets the CPU and restores the original memory contents."), wxITEM_NORMAL );
	m_MenuRun->Append( m_MenuItemFullReset );

	m_MenuRun->AppendSeparator();

	wxMenuItem* m_MenuItemClearTrace;
	m_MenuItemClearTrace = new wxMenuItem( m_MenuRun, wxID_ANY, wxString( _("&Clear Trace") ) + wxT('\t') + wxT("Alt+T"), _("Clears the trace information about which instructions were already executed."), wxITEM_NORMAL );
	m_MenuRun->Append( m_MenuItemClearTrace );

	wxMenuItem* m_MenuItemClearAllBreakpoints;
	m_MenuItemClearAllBreakpoints = new wxMenuItem( m_MenuRun, wxID_ANY, wxString( _("Delete All &Breakpoints") ) + wxT('\t') + wxT("Alt-B"), wxEmptyString, wxITEM_NORMAL );
	m_MenuRun->Append( m_MenuItemClearAllBreakpoints );

	wxMenuItem* m_MenuItemToggleBreakpoint;
	m_MenuItemToggleBreakpoint = new wxMenuItem( m_MenuRun, wxID_ANY, wxString( _("&Toggle Breakpoint") ) + wxT('\t') + wxT("Ctrl-B"), _("Enables/disables a breakpoint at the current R15/PC address."), wxITEM_NORMAL );
	m_MenuRun->Append( m_MenuItemToggleBreakpoint );

	m_MenuBar->Append( m_MenuRun, _("&Run") );

	m_MenuHelp = new wxMenu();
	wxMenuItem* m_MenuItemAbout;
	m_MenuItemAbout = new wxMenuItem( m_MenuHelp, wxID_ANY, wxString( _("&About") ) , wxEmptyString, wxITEM_NORMAL );
	m_MenuHelp->Append( m_MenuItemAbout );

	m_MenuBar->Append( m_MenuHelp, _("&Help") );

	this->SetMenuBar( m_MenuBar );

	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxVERTICAL );

	m_PanelMain = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxCLIP_CHILDREN );
	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxHORIZONTAL );

	wxBoxSizer* bSizer51;
	bSizer51 = new wxBoxSizer( wxVERTICAL );

	m_PanelDisassembly = new DisassemblyPanel( m_PanelMain, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE );
	bSizer51->Add( m_PanelDisassembly, 1, wxEXPAND|wxTOP|wxLEFT, 5 );

	m_PanelMemory = new ThumbMemoryPanel( m_PanelMain, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE );
	m_PanelMemory->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHTTEXT ) );

	bSizer51->Add( m_PanelMemory, 0, wxTOP|wxBOTTOM|wxLEFT|wxEXPAND, 5 );


	bSizer4->Add( bSizer51, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer( wxVERTICAL );

	m_PanelRegisters = new RegistersPanel( m_PanelMain, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE );
	bSizer6->Add( m_PanelRegisters, 0, wxALL|wxALIGN_RIGHT, 5 );


	bSizer4->Add( bSizer6, 0, wxEXPAND, 5 );


	bSizer5->Add( bSizer4, 1, wxEXPAND, 5 );

	m_BitmapLCD = new wxBitmapButton( m_PanelMain, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize( 878,32 ), wxBU_AUTODRAW|0|wxBORDER_SIMPLE );
	bSizer5->Add( m_BitmapLCD, 0, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );


	m_PanelMain->SetSizer( bSizer5 );
	m_PanelMain->Layout();
	bSizer5->Fit( m_PanelMain );
	bSizer3->Add( m_PanelMain, 1, wxEXPAND | wxALL, 5 );


	this->SetSizer( bSizer3 );
	this->Layout();
	bSizer3->Fit( this );
	m_StatusBar = this->CreateStatusBar( 1, wxSTB_SIZEGRIP, wxID_ANY );

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_IDLE, wxIdleEventHandler( wxThumbFrameBase::IdleLoop ) );
	m_MenuFile->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxThumbFrameBase::New ), this, m_MenuItemNew->GetId());
	m_MenuFile->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxThumbFrameBase::LoadELFFile ), this, m_MenuItemLoad->GetId());
	m_MenuFile->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxThumbFrameBase::OnQuit ), this, m_MenuItemExit->GetId());
	m_MenuRun->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxThumbFrameBase::OnStepInto ), this, m_MenuItemStepInto->GetId());
	m_MenuRun->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxThumbFrameBase::OnStepOver ), this, m_MenuItemStepOver->GetId());
	m_MenuRun->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxThumbFrameBase::OnAnimate ), this, m_MenuItemAnimate->GetId());
	m_MenuRun->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxThumbFrameBase::OnRun ), this, m_MenuItemRun->GetId());
	m_MenuRun->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxThumbFrameBase::OnMaxSpeed ), this, m_MenuItemMaxSpeed->GetId());
	m_MenuRun->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxThumbFrameBase::OnPause ), this, m_MenuItemPause->GetId());
	m_MenuRun->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxThumbFrameBase::OnResetCPU ), this, m_MenuItemResetCPU->GetId());
	m_MenuRun->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxThumbFrameBase::OnFullReset ), this, m_MenuItemFullReset->GetId());
	m_MenuRun->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxThumbFrameBase::OnClearTrace ), this, m_MenuItemClearTrace->GetId());
	m_MenuRun->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxThumbFrameBase::OnMenuDeleteAllBreakpoints ), this, m_MenuItemClearAllBreakpoints->GetId());
	m_MenuRun->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxThumbFrameBase::OnMenuToggleBreakpoint ), this, m_MenuItemToggleBreakpoint->GetId());
	m_MenuHelp->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxThumbFrameBase::OnAbout ), this, m_MenuItemAbout->GetId());
}

wxThumbFrameBase::~wxThumbFrameBase()
{
}
