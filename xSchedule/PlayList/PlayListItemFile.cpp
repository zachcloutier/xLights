/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/xml/xml.h>
#include <wx/notebook.h>
#include <wx/file.h>

#include "PlayListItemFile.h"
#include "PlayListItemFilePanel.h"
#include "../xScheduleMain.h"
#include "../ScheduleManager.h"

#include <log4cpp/Category.hh>

PlayListItemFile::PlayListItemFile(wxXmlNode* node) : PlayListItem(node)
{
    _started = false;
    _append = false;
    _content = "";
	_fileName = "";
    PlayListItemFile::Load(node);
}

void PlayListItemFile::Load(wxXmlNode* node)
{
    PlayListItem::Load(node);
    _content = node->GetAttribute("Content", "");
    _fileName = node->GetAttribute("FileName", "");
    _append = node->GetAttribute("Append", "FALSE") == "TRUE";
}

PlayListItemFile::PlayListItemFile() : PlayListItem()
{
    _type = "PLIFile";
    _started = false;
    _append = false;
    _content = "";
    _fileName = "";
}

PlayListItem* PlayListItemFile::Copy(const bool isClone) const
{
    PlayListItemFile* res = new PlayListItemFile();
    res->_content = _content;
    res->_append = _append;
    res->_fileName = _fileName;
    res->_started = false;
    PlayListItem::Copy(res, isClone);

    return res;
}

wxXmlNode* PlayListItemFile::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, GetType());

    node->AddAttribute("Content", _content);
    node->AddAttribute("FileName", _fileName);
    if (_append) node->AddAttribute("Append", "TRUE");

    PlayListItem::Save(node);

    return node;
}

std::string PlayListItemFile::GetTitle() const
{
    return "File";
}

void PlayListItemFile::Configure(wxNotebook* notebook)
{
    notebook->AddPage(new PlayListItemFilePanel(notebook, this), GetTitle(), true);
}

std::string PlayListItemFile::GetNameNoTime() const
{
    if (_name != "") return _name;

    return "File";
}

std::string PlayListItemFile::GetTooltip()
{
    return GetTagHint() + "\n    %SHOWDIR% - the current show directory";
}

std::string PlayListItemFile::FileReplaceTags(const std::string s)
{
	wxString res = s;

	if (res.Contains("%SHOWDIR%"))
	{
		res.Replace("%SHOWDIR%", xScheduleFrame::GetScheduleManager()->GetShowDir(), true);
	}

    return ReplaceTags(res);
}

void PlayListItemFile::Frame(uint8_t* buffer, size_t size, size_t ms, size_t framems, bool outputframe)
{
    if (ms >= _delay && !_started)
    {
        _started = true;

        wxString content = FileReplaceTags(_content);
        wxString fileName = FileReplaceTags(_fileName);

        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

        // ensure we are in the show directory before creating the file
        wxSetWorkingDirectory(xScheduleFrame::GetScheduleManager()->GetShowDir());
        logger_base.info("Currentdirectory %s.", (const char *)wxGetCwd().c_str());
        logger_base.info("Writing file %s. Content: '%s'", (const char *)fileName.c_str(), (const char *)content.c_str());

		wxFile f;
		
        if (_append) {
            f.Open(fileName, wxFile::OpenMode::write_append);
            f.Write(content);
            f.Close();
        }
        else {
            f.Create(fileName, true);
            f.Write(content);
            f.Close();
        }

        logger_base.info("File written.");
    }
}

void PlayListItemFile::Start(long stepLengthMS)
{
    PlayListItem::Start(stepLengthMS);

    _started = false;
}
