/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "SequenceVideoPanel.h"
#include "VideoReader.h"

#include <algorithm>

#include "SequenceVideoPreview.h"

//(*InternalHeaders(SequenceVideoPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(SequenceVideoPanel)
//*)

BEGIN_EVENT_TABLE(SequenceVideoPanel,wxPanel)
	//(*EventTable(SequenceVideoPanel)
	//*)
   EVT_SIZE( SequenceVideoPanel::Resized )
END_EVENT_TABLE()

SequenceVideoPanel::SequenceVideoPanel(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
   : _path(), _videoReader(), _isValidVideo(false), _videoWidth(0), _videoHeight(0), _videoLength(0), _videoPreview(nullptr)
{
	//(*Initialize(SequenceVideoPanel)
	Create(parent, id, wxDefaultPosition, wxDLG_UNIT(parent,wxSize(160,90)), wxTAB_TRAVERSAL, _T("id"));
	//*)

   _videoPreview = new SequenceVideoPreview( this );
}

SequenceVideoPanel::~SequenceVideoPanel()
{
	//(*Destroy(SequenceVideoPanel)
	//*)
}

void SequenceVideoPanel::SetMediaPath(const std::string& path)
{
    if (!VideoReader::IsVideoFile(path))
    {
        _videoReader.reset();
        _videoWidth = 0;
        _videoHeight = 0;
        _videoLength = 0;
        _isValidVideo = false;
        _videoPreview->Clear();
    }
    else
    {
        bool isHWAccel = VideoReader::IsHardwareAcceleratedVideo();
        // if using hardware acceleration, keep it in the native BGRA format
        // to avoid some byte reordering, extra copies, etc...
        bool wantsHWType = false;
#ifdef XL_DRAWING_WITH_METAL
        wantsHWType = isHWAccel;
#endif
        
        _videoReader.reset(new VideoReader(path, 0, 0, true, true, isHWAccel, isHWAccel, wantsHWType));
        if (_videoReader->IsValid()) {
            _isValidVideo = true;
            _videoWidth = _videoReader->GetWidth();
            _videoHeight = _videoReader->GetHeight();
            _videoLength = _videoReader->GetLengthMS();
        }
    }
}

void SequenceVideoPanel::UpdateVideo( int ms )
{
    if ( !_isValidVideo || !IsShownOnScreen() )
        return;

    int clampedTime = std::min( ms, _videoLength );

    AVFrame *frame = _videoReader->GetNextFrame( clampedTime );
    if ( frame != nullptr ) {
        _videoPreview->Render( frame );
    }
}

void SequenceVideoPanel::Resized( wxSizeEvent& evt )
{
   if ( _videoPreview ) {
      _videoPreview->Move( 0, 0 );
      _videoPreview->SetSize( evt.GetSize() );
   }
}

