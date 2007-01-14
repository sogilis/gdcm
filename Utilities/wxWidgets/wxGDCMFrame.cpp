// -*- C++ -*- generated by wxGlade 0.4.1 on Sat Aug 19 15:28:55 2006

#include "wxGDCMFrame.h"
#include "wxVTKRenderWindowInteractor.h"
#include "vtkImageViewer.h"
#include "vtkGDCMReader.h"

BEGIN_EVENT_TABLE( wxGDCMFrame, wxGDCMFrameBase )
    EVT_MENU(wxID_OPEN, wxGDCMFrame::OnOpen)
    EVT_MENU(wxID_HELP, wxGDCMFrame::OnAbout)
    EVT_MENU(wxID_EXIT, wxGDCMFrame::OnQuit)
    EVT_CLOSE(          wxGDCMFrame::OnCloseFrame)
END_EVENT_TABLE( );


wxGDCMFrame::wxGDCMFrame(wxWindow* parent, int id, const wxString& title, const wxPoint& pos, const wxSize& size, long style):
    wxGDCMFrameBase(parent, id, title, pos, size, wxDEFAULT_FRAME_STYLE)
{

    imageViewer = vtkImageViewer::New();
    imageViewer->SetupInteractor( VTKWindow );
    Reader      = vtkGDCMReader::New();
    directory = wxT("/home/mathieu/Projects/GDCM/gdcmData");
}

wxGDCMFrame::~wxGDCMFrame()
{
  VTKWindow->Delete();
  imageViewer->Delete();
  Reader->Delete();
}


void wxGDCMFrame::OnCloseFrame( wxCloseEvent& event )
{
  std::cerr << "Close" << std::endl;
  Destroy();
}

void wxGDCMFrame::OnOpen(wxCommandEvent& event)
{
  std::cerr << "Open" << std::endl;
  wxString filemask = wxT("DICOM files (*.dcm)|*.dcm");
  wxFileDialog* dialog = new wxFileDialog( this, wxT("Open DICOM"), directory,
	filename, filemask, wxOPEN );
  dialog->CentreOnParent();
  if ( dialog->ShowModal() == wxID_OK )
  {
    directory = dialog->GetDirectory();
    filename  = dialog->GetFilename();
    std::cerr << "Dir: " << directory.fn_str() << std::endl;
    std::cerr << "File: " << filename.fn_str() << std::endl;
    //wxString fn = dialog->GetFilename();
    //std::cerr << "fn: " << fn.fn_str() << std::endl;
    std::string fn = (const char*)directory.fn_str();
    fn += "/";
    fn += (const char *)filename.fn_str();
    Reader->SetFileName( fn.c_str() );
    //Reader->Update();
    imageViewer->SetInputConnection( Reader->GetOutputPort() );
    imageViewer->Render();
  }
  dialog->Close();
  dialog->Destroy();
}

void wxGDCMFrame::OnQuit( wxCommandEvent& event )
{
  std::cerr << "Quit" << std::endl;
  Close(true);
}

void wxGDCMFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxMessageBox( _T("This is the about box for wxGDCM"), _T("About wxGDCM"));
/*
  wxMessageDialog* msgDialog = new wxMessageDialog( this, wxString(
	text.c_str(), wxConvUTF8 ), wxString( title.c_str(), wxConvUTF8 ), wxOK );
  msgDialog->ShowModal();
  msgDialog->Close();
  msgDialog->Destroy();
*/

}

