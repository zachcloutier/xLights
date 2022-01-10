/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "../xLightsMain.h"
#include "../xLightsVersion.h"

#include "../FSEQFile.h"
#include "../outputs/Controller.h"
#include "../outputs/ControllerEthernet.h"
#include "../LayoutPanel.h"
#include "../ViewsModelsPanel.h"
#include "../controllers/ControllerCaps.h"
#include "../controllers/FPP.h"
#include "../controllers/Falcon.h"
#include "../../xSchedule/wxJSON/jsonreader.h"
#include "../../xSchedule/wxJSON/jsonwriter.h"
#include "../UtilFunctions.h"
#include "../xLightsApp.h"
#include "../JukeboxPanel.h"
#include "../outputs/E131Output.h"
#include "../../xSchedule/wxHTTPServer/wxhttpserver.h"
#include "../sequencer/MainSequencer.h"
#include <wx/uri.h>

#include "LuaRunner.h"

#include <log4cpp/Category.hh>

std::string xLightsFrame::FindSequence(const std::string& seq)
{
    if (wxFile::Exists(seq))
        return seq;

    if (wxFile::Exists(CurrentDir + wxFileName::GetPathSeparator() + seq))
        return CurrentDir + wxFileName::GetPathSeparator() + seq;
    
    return "";
}
static const char HTTP_ERROR_PAGE[] = "Could not process xLights Automation";
static bool HttpRequestFunction(HttpConnection &connection, HttpRequest &request) {
    return xLightsApp::__frame->ProcessHttpRequest(connection, request);
}

static wxString MIME_JSON = "application/json";
static wxString MIME_TEXT = "text/plain";

static std::map<std::string, std::string> ParseParams(const wxString &params) {
    std::map<std::string, std::string> p;
    std::string np = params;
    while (!np.empty()) {
        std::string np2 = np;
        size_t idx = np2.find('&');
        if (idx != std::string::npos) {
            np = np2.substr(idx + 1);
            np2 = np2.substr(0, idx);
        } else {
            np = "";
        }
        idx = np2.find('=');
        std::string value = "";
        if (idx != std::string::npos) {
            value = np2.substr(idx + 1);
            np2 = np2.substr(0, idx);
        }
        p[np2] = wxURI::Unescape(value);
    }
    return p;
}
inline bool ReadBool(const wxJSONValue &v) {
    if (v.IsBool()) {
        return v.AsBool();
    }
    if (v.IsInt()) {
        return v.IsInt() != 0;
    }
    return v.AsString() == "true" || v.AsString() == "1";
}
inline bool ReadBool(const std::string &v) {
    return v == "true" || v == "1";
}


bool xLightsFrame::ProcessAutomation(std::vector<std::string> &paths,
                                     std::map<std::string, std::string> &params,
                                     const std::function<bool(const std::string &msg,
                                                              const std::string &jsonKey,
                                                              int responseCode,
                                                              bool msgIsJSON)> &sendResponse) {

    if (paths.size() == 0) {
        return sendResponse("No command", "msg", 503, false);
    }

    std::string cmd = paths[0];
    if (cmd == "getVersion") {
        return sendResponse(GetDisplayVersionString(), "version", 200, false);
    } else if (cmd == "openSequence" || cmd == "getOpenSequence" || cmd == "loadSequence") {
        wxString fname = "";
        if (paths.size() > 1) {
            fname = wxURI::Unescape(paths[1]);
        }
        bool force = false;
        bool prompt = false;
        
        if (params["_METHOD"] == "POST" && !params["_DATA"].empty()) {
            wxString fname = params["_DATA"];
            wxJSONValue val;
            wxJSONReader reader;
            if (reader.Parse(fname, &val) == 0) {
                fname = val["seq"].AsString();
                if (val.HasMember("promptIssues")) {
                    prompt = ReadBool(params["promptIssues"]);
                }
                if (val.HasMember("force")) {
                    force = ReadBool(params["force"]);
                }
            } else {
                fname = "";
            }
        } else {
            if (params["seq"] != "") {
                fname = params["seq"];
            }
            prompt = ReadBool(params["promptIssues"]);
            force = ReadBool(params["force"]);
        }
        if (fname.empty()) {
            if (CurrentSeqXmlFile != nullptr) {
                std::string response = wxString::Format("{\"seq\":\"%s\",\"fullseq\":\"%s\",\"media\":\"%s\",\"len\":%u,\"framems\":%u}",
                                                        JSONSafe(CurrentSeqXmlFile->GetName()),
                                                        JSONSafe(CurrentSeqXmlFile->GetFullPath()),
                                                        JSONSafe(CurrentSeqXmlFile->GetMediaFile()),
                                                        CurrentSeqXmlFile->GetSequenceDurationMS(),
                                                        CurrentSeqXmlFile->GetFrameMS());

                return sendResponse(response, "", 200, true);
            } else {
                return sendResponse("Sequence not open.", "msg", 503, false);
            }
        } else {
            std::string seq = FindSequence(fname);
            if (seq == "") {
                return sendResponse("Sequence not found.", "msg", 503, false);
            }
            if (CurrentSeqXmlFile != nullptr && force) {
                return sendResponse("Sequence already open.", "msg", 503, false);
            }
            auto oldPrompt = _promptBatchRenderIssues;
            _promptBatchRenderIssues = prompt; // off by default
            OpenSequence(seq, nullptr);
            _promptBatchRenderIssues = oldPrompt;
            std::string response = wxString::Format("{\"seq\":\"%s\",\"fullseq\":\"%s\",\"media\":\"%s\",\"len\":%u,\"framems\":%u}",
                                                    JSONSafe(CurrentSeqXmlFile->GetName()),
                                                    JSONSafe(CurrentSeqXmlFile->GetFullPath()),
                                                    JSONSafe(CurrentSeqXmlFile->GetMediaFile()),
                                                    CurrentSeqXmlFile->GetSequenceDurationMS(),
                                                    CurrentSeqXmlFile->GetFrameMS());

            return sendResponse(response, "", 200, true);
        }
    } else if (cmd == "closeSequence") {
        if (CurrentSeqXmlFile == nullptr) {
            if (!ReadBool(params["quiet"])) {
                return sendResponse("Sequence not open.", "msg", 503, false);
            }
            return sendResponse("Sequence closed.", "msg", 200, false);
        }

        auto force = ReadBool(params["force"]);
        if (mSavedChangeCount != _sequenceElements.GetChangeCount()) {
            if (force) {
                mSavedChangeCount = _sequenceElements.GetChangeCount();
            } else {
                return sendResponse("Sequence has unsaved changes.", "msg", 504, false);
            }
        }

        AskCloseSequence();
        return sendResponse("Sequence closed.", "msg", 200, false);
    } else if (cmd == "newSequence") {
        if (CurrentSeqXmlFile != nullptr && !ReadBool(params["force"])) {
            return sendResponse("Sequence already open.", "msg", 503, false);
        }

        auto media = params["mediaFile"];
        if (media == "null")
            media = "";
        auto duration = wxAtoi(params["durationSecs"]) * 1000;

        NewSequence(media, duration);
        EnableSequenceControls(true);
        return sendResponse("Sequence created.", "msg", 200, false);
    } else if (cmd == "saveSequence") {
        if (CurrentSeqXmlFile == nullptr) {
            return sendResponse("No sequence open.", "msg", 503, false);
        }
        auto seq = params["seq"];

        if (seq != "" && seq != "null") {
            SaveAsSequence(seq);
        } else {
            if (xlightsFilename.IsEmpty()) {
                return sendResponse("Saving unnamed sequence needs a name to be sent.", "msg", 503, false);
            }
            SaveSequence();
        }
        return sendResponse("Sequence Saved.", "msg", 200, false);
    } else if (cmd == "renderAll") {
        if (CurrentSeqXmlFile == nullptr) {
            return sendResponse("No sequence open.", "msg", 503, false);
        }
        RenderAll();
        while (mRendering) {
            wxYield();
        }
        return sendResponse("Rendered.", "msg", 200, false);
    } else if (cmd == "batchRender") {
        wxArrayString files;
                
        auto seqs = params["seqs_0"];
        int snum = 0;
        while (seqs != "") {
            auto seq = FindSequence(seqs);
            if (seq == "") {
                return sendResponse("Sequence not found '" + seq + "'", "msg", 503, false);
            }
            files.push_back(seq);
            snum++;
            seqs = params["seqs_" + std::to_string(snum)];
        }
        auto oldPrompt = _promptBatchRenderIssues;
        _promptBatchRenderIssues = ReadBool(params["promptIssues"]);

        _renderMode = true;
        OpenRenderAndSaveSequences(files, false);

        while (_renderMode) {
            wxYield();
        }

        _promptBatchRenderIssues = oldPrompt;
        return sendResponse("Sequence batch rendered.", "msg", 200, false);
    } else if (cmd == "uploadController") {
        auto ip = params["ip"];
        Controller* c = _outputManager.GetControllerWithIP(ip);
        if (c == nullptr) {
            return sendResponse("Controller not found '" + ip + "'", "msg", 503, false);
        }

        // ensure all start channels etc are up to date
        RecalcModels();

        bool res = true;
        auto caps = GetControllerCaps(c->GetName());
        if (caps != nullptr) {
            wxString message;
            if (caps->SupportsInputOnlyUpload()) {
                res = res && UploadInputToController(c, message);
            }
            res = res && UploadOutputToController(c, message);
        } else {
            res = false;
        }
        if (res) {
            return sendResponse("Uploaded to controller '" + ip + "'", "msg", 200, false);
        }
        return sendResponse("Upload to controller '" + ip + "' failed.", "msg", 503, false);
    } else if (cmd == "uploadFPPConfig") {
        auto ip = params["ip"];
        auto udp = params["udp"];
        auto models = params["models"];
        auto map = params["displayMap"];

        // discover the FPP instances
        auto instances = FPP::GetInstances(this, &_outputManager);

        FPP* fpp = nullptr;
        for (const auto& it : instances) {
            if (it->ipAddress == ip && it->fppType == FPP_TYPE::FPP) {
                fpp = it;
                break;
            }
        }
        if (fpp == nullptr) {
            return sendResponse("FPP not found '" + ip + "'.", "msg", 503, false);
        }

        std::map<int, int> udpRanges;
        wxJSONValue outputs = FPP::CreateUniverseFile(_outputManager.GetControllers(), false, &udpRanges);

        if (udp == "all") {
            fpp->UploadUDPOut(outputs);
            fpp->SetRestartFlag();
        } else if (udp == "proxy") {
            fpp->UploadUDPOutputsForProxy(&_outputManager);
            fpp->SetRestartFlag();
        }

        if (models == "true") {
            wxJSONValue memoryMaps = FPP::CreateModelMemoryMap(&AllModels);
            fpp->UploadModels(memoryMaps);
        }

        if (map == "true") {
            std::string displayMap = FPP::CreateVirtualDisplayMap(&AllModels, GetDisplay2DCenter0());
            fpp->UploadDisplayMap(displayMap);
            fpp->SetRestartFlag();
        }

        //if restart flag is now set, restart and recheck range
        fpp->Restart("", true);

        return sendResponse("Uploaded to FPP '" + ip + "'.", "msg", 200, false);
    } else if (cmd == "uploadSequence") {
        bool res = true;
        auto ip = params["ip"];
        auto media = ReadBool(params["media"]);
        auto format = params["format"];
        auto xsq = params["seq"];
        xsq = FindSequence(xsq);

        if (xsq == "") {
            return sendResponse("Sequence not found.", "msg", 503, false);
        }

        auto fseq = xLightsXmlFile::GetFSEQForXSQ(xsq, GetFseqDirectory());
        auto m2 = xLightsXmlFile::GetMediaForXSQ(xsq, CurrentDir, GetMediaFolders());

        if (!wxFile::Exists(fseq)) {
            return sendResponse("Unable to find sequence FSEQ file.", "msg", 503, false);
        }

        // discover the FPP instances
        auto instances = FPP::GetInstances(this, &_outputManager);

        FPP* fpp = nullptr;
        for (const auto& it : instances) {
            if (it->ipAddress == ip) {
                fpp = it;
                break;
            }
        }
        if (fpp == nullptr) {
            return sendResponse("Player " + ip + " not found.", "msg", 503, false);
        }

        int fseqType = 0;
        if (format == "v1") {
            fseqType = 0;
        } else if (format == "v2std") {
            fseqType = 1;
        } else if (format == "v2zlib") {
            fseqType = 5;
        } else if (format == "v2uncompressedsparse") {
            fseqType = 3;
        } else if (format == "v2uncompressed") {
            fseqType = 4;
        } else if (format == "v2stdsparse") {
            fseqType = 2;
        } else if (format == "v2zlibsparse") {
            fseqType = 6;
        }

        if (!media) {
            m2 = "";
        }

        FSEQFile* seq = FSEQFile::openFSEQFile(fseq);
        if (seq) {
            fpp->PrepareUploadSequence(*seq, fseq, m2, fseqType);
            static const int FRAMES_TO_BUFFER = 50;
            std::vector<std::vector<uint8_t>> frames(FRAMES_TO_BUFFER);
            for (size_t x = 0; x < frames.size(); x++) {
                frames[x].resize(seq->getMaxChannel() + 1);
            }

            for (size_t frame = 0; frame < seq->getNumFrames(); frame++) {
                int lastBuffered = 0;
                size_t startFrame = frame;
                //Read a bunch of frames so each parallel thread has more info to work with before returning out here
                while (lastBuffered < FRAMES_TO_BUFFER && frame < seq->getNumFrames()) {
                    FSEQFile::FrameData* f = seq->getFrame(frame);
                    if (f != nullptr) {
                        if (!f->readFrame(&frames[lastBuffered][0], frames[lastBuffered].size())) {
                            //logger_base.error("FPPConnect FSEQ file corrupt.");
                            res = false;
                        }
                        delete f;
                    }
                    lastBuffered++;
                    frame++;
                }
                frame--;
                for (int x = 0; x < lastBuffered; x++) {
                    fpp->AddFrameToUpload(startFrame + x, &frames[x][0]);
                }
            }
            fpp->FinalizeUploadSequence();

            if (fpp->fppType == FPP_TYPE::FALCONV4) {
                // a falcon
                std::string proxy = "";
                auto c = _outputManager.GetControllers(fpp->ipAddress);
                if (c.size() == 1)
                    proxy = c.front()->GetFPPProxy();
                Falcon falcon(fpp->ipAddress, proxy);

                if (falcon.IsConnected()) {
                    falcon.UploadSequence(fpp->GetTempFile(), fseq, fpp->mode == "remote" ? "" : m2, nullptr);
                } else {
                    res = false;
                }
                fpp->ClearTempFile();
            }
            delete seq;
        } else {
            return sendResponse("Failed to generate FSEQ.", "msg", 503, false);
        }

        if (!res) {
            return sendResponse("Failed to upload.", "msg", 503, false);
        }
        return sendResponse("Sequence uploaded.", "msg", 200, false);
    } else if (cmd == "checkSequence") {
        auto seq = params["seq"];
        seq = FindSequence(seq);
        if (seq == "") {
            return sendResponse("Sequence not found.", "msg", 503, false);
        }
        auto file = OpenAndCheckSequence(seq);

        std::string response = wxString::Format("{\"msg\":\"Sequence checked.\",\"output\":\"%s\"}", JSONSafe(file));
        return sendResponse(response, "", 200, true);
    } else if (cmd == "changeShowFolder") {
        auto shw = params["folder"];
        if (!wxDir::Exists(shw)) {
            return sendResponse("Folder does not exist.", "msg", 503, false);
        }

        auto force = ReadBool(params["force"]);
        if (CurrentSeqXmlFile != nullptr && mSavedChangeCount != _sequenceElements.GetChangeCount()) {
            if (force) {
                mSavedChangeCount = _sequenceElements.GetChangeCount();
            } else {
                return sendResponse("Sequence has unsaved changes.", "msg", 503, false);
            }
        }

        if (UnsavedRgbEffectsChanges) {
            if (force) {
                UnsavedRgbEffectsChanges = false;
            } else {
                return sendResponse("Layout has unsaved changes.", "msg", 503, false);
            }
        }

        if (UnsavedNetworkChanges) {
            if (force) {
                UnsavedNetworkChanges = false;
            } else {
                return sendResponse("Controller has unsaved changes.", "msg", 503, false);
            }
        }

        displayElementsPanel->SetSequenceElementsModelsViews(nullptr, nullptr, nullptr, nullptr, nullptr);
        layoutPanel->ClearUndo();
        SetDir(shw, true);

        return sendResponse("Show folder changed to " + shw + ".", "msg", 200, false);
    } else if (cmd == "openController") {
        auto ip = params["ip"];
        ::wxLaunchDefaultBrowser(ip);

        return sendResponse("Controller opened", "msg", 200, false);

    } else if (cmd == "openControllerProxy") {
        auto ip = params["ip"];
        auto controller = _outputManager.GetControllerWithIP(ip);

        if (controller == nullptr) {
            return "{\"res\":504,\"msg\":\"Controller not found.\"}";
        }

        auto proxy = controller->GetFPPProxy();

        if (proxy == "") {
            return "{\"res\":504,\"msg\":\"Controller has no proxy.\"}";
        }

        ::wxLaunchDefaultBrowser(proxy);

        return "{\"res\":200,\"msg\":\"Proxy opened.\"}";

    } else if (cmd == "exportModelsCSV") {
        auto filename = params["filename"];
        if (filename == "" || filename == "null") {
            wxFileName f;
            f.AssignTempFileName("Models_");
            filename = f.GetFullPath();
        }

        ExportModels(filename);

        std::string response = wxString::Format("{\"msg\":\"Models Exported.\",\"output\":\"%s\"}", JSONSafe(filename));
        return sendResponse(response, "", 200, true);
    } else if (cmd == "exportModel") {
        if (CurrentSeqXmlFile == nullptr) {
            return sendResponse("Sequence not open.", "msg", 503, false);
        }

        auto model = params["model"];
        if (AllModels.GetModel(model) == nullptr) {
            return sendResponse("Unknown model.", "msg", 503, false);
        }

        auto filename = params["filename"];
        auto format = params["format"];

        if (format == "lsp") {
            format = "LSP";
        } else if (format == "lorclipboard") {
            format = "Lcb";
        } else if (format == "lorclipboards5") {
            format = "LcbS5";
        } else if (format == "vixenroutine") {
            format = "Vir";
        } else if (format == "hls") {
            format = "HLS";
        } else if (format == "eseq") {
            format = "FPP";
        } else if (format == "eseqcompressed") {
            format = "FPPCompressed";
        } else if (format == "avicompressed") {
            format = "Com";
        } else if (format == "aviuncompressed") {
            format = "Unc";
        } else if (format == "minleon") {
            format = "Min";
        } else if (format == "gif") {
            format = "GIF";
        } else {
            return sendResponse("Unknown format.", "msg", 503, false);
        }

        if (DoExportModel(0, 0, model, filename, format, false)) {
            return sendResponse("Model exported.", "msg", 200, false);
        } else {
            return sendResponse("Failed to export.", "msg", 503, false);
        }
    } else if (cmd == "closexLights") {
        auto force = ReadBool(params["force"]);
        if (CurrentSeqXmlFile != nullptr && mSavedChangeCount != _sequenceElements.GetChangeCount()) {
            if (force) {
                mSavedChangeCount = _sequenceElements.GetChangeCount();
            } else {
                return sendResponse("Sequence has unsaved changes.", "msg", 503, false);
            }
        }

        if (UnsavedRgbEffectsChanges) {
            if (force) {
                UnsavedRgbEffectsChanges = false;
            } else {
                return sendResponse("Layout has unsaved changes.", "msg", 503, false);
            }
        }

        if (UnsavedNetworkChanges) {
            if (force) {
                UnsavedNetworkChanges = false;
            } else {
                return sendResponse("Controller has unsaved changes.", "msg", 503, false);
            }
        }

        wxCloseEvent evt;
        wxPostEvent(this, evt);
        return sendResponse("xLights closed.", "msg", 200, false);
    } else if (cmd == "lightsOn") {
        EnableOutputs(true);
        return sendResponse("Lights on.", "msg", 200, false);
    } else if (cmd == "lightsOff") {
        DisableOutputs();
        return sendResponse("Lights off.", "msg", 200, false);
    } else if (cmd == "playJukebox") {
        int button = wxAtoi(params["button"]);
        if (CurrentSeqXmlFile != nullptr) {
            jukeboxPanel->PlayItem(button);
            return sendResponse("Played button " + std::to_string(button), "msg", 200, false);
        } else {
            return sendResponse("Sequence not open.", "msg", 503, false);
        }
     } else if (cmd == "jukeboxButtonTooltips" || cmd == "getJukeboxButtonTooltips") {
        if (CurrentSeqXmlFile != nullptr) {
            return sendResponse(jukeboxPanel->GetTooltipsJSON(), "tooltips", 200, true);
        } else {
            return sendResponse("Sequence not open.", "msg", 503, false);
        }
    } else if (cmd == "jukeboxButtonEffectPresent" || cmd == "getJukeboxButtonEffectPresent") {
        if (CurrentSeqXmlFile != nullptr) {
            return sendResponse(jukeboxPanel->GetEffectPresentJSON(), "effects", 200, true);
        } else {
            return sendResponse("Sequence not open.", "msg", 503, false);
        }
    } else if (cmd == "e131Tag" || cmd == "getE131Tag") {
        return sendResponse(E131Output::GetTag(), "tag", 200, false);
    } else if (cmd == "addEthernetController") {
        auto c = new ControllerEthernet(&_outputManager);
        //c->SetProtocol(params["protocol"]);
        c->SetIP(params["ip"]);
        c->SetId(1);
        c->EnsureUniqueId();
        c->SetName(params["name"]);
        auto const vendors = ControllerCaps::GetVendors(c->GetType());
        if (std::find(vendors.begin(), vendors.end(), params["vendor"]) != vendors.end()) {
            c->SetVendor(params["vendor"]);
            auto models = ControllerCaps::GetModels(c->GetType(), params["vendor"]);
            if (std::find(models.begin(), models.end(), params["model"]) != models.end()) {
                c->SetModel(params["model"]);
                auto variants = ControllerCaps::GetVariants(c->GetType(), params["vendor"], params["model"]);
                if (std::find(variants.begin(), variants.end(), params["variant"]) != variants.end()) {
                    c->SetVariant(params["variant"]);
                }
            }
        }
        
        _outputManager.AddController(c);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "Automation:ADDETHERNET");
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "Automation:ADDETHERNET");
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "Automation:ADDETHERNET", nullptr, c);
        _outputModelManager.AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "Automation:ADDETHERNET");
        return sendResponse("Added Ethernet Controller", "msg", 200, false);
    
    } else if (cmd == "packageSequence") {
        if (CurrentSeqXmlFile == nullptr) {
            return sendResponse("Sequence not open.", "msg", 503, false);
        }
        auto const filename = PackageSequence(false);
        std::string response = wxString::Format("{\"msg\":\"Sequence Packaged.\",\"output\":\"%s\"}", JSONSafe(filename));
        return sendResponse(response, "", 200, true);
    } else if (cmd == "packageLogFiles") {
        auto const filename = PackageDebugFiles(false);
        std::string response = wxString::Format("{\"msg\":\"Log Files Packaged.\",\"output\":\"%s\"}", JSONSafe(filename));
        return sendResponse(response, "", 200, true);

    } else if (cmd == "exportVideoPreview") {
        if (CurrentSeqXmlFile == nullptr) {
            return sendResponse("Sequence not open.", "msg", 503, false);
        }

        auto filename = params["filename"];
        if (filename == "" || filename == "null") {
            filename = CurrentDir + wxFileName::GetPathSeparator() + CurrentSeqXmlFile->GetName() + ".mp4";
        }
        auto const worked = ExportVideoPreview(filename);
        if (worked) {
            std::string response = wxString::Format("{\"msg\":\"Export Video Preview.\",\"output\":\"%s\"}", JSONSafe(filename));
            return sendResponse(response, "", 200, true);
        }        
        return sendResponse("Export Video Preview Failed", "msg", 503, true);
    } else if (cmd == "runScript") {
        auto filename = params["filename"];
        if (filename.empty() || filename == "null" || !wxFile::Exists(filename)) {
            return sendResponse("Invalid Script Path.", "msg", 503, false);
        }

        LuaRunner runner(this);
        auto const worked = runner.Run_Script(filename, [](std::string const& m) {});
        if (worked) {
            std::string response = "{\"msg\":\"Script Was Successful.\"}";
            return sendResponse(response, "", 200, true);
        }
        return sendResponse("Script Failed", "msg", 503, true);
    } else if (cmd == "cloneModelEffects") {
        if (CurrentSeqXmlFile == nullptr) {
            return sendResponse("Sequence not open.", "msg", 503, false);
        }
        auto target = params["target"];
        auto source = params["source"];
        auto erase = false;

        if (!params["eraseModel"].empty()) {
            erase = ReadBool(params["eraseModel"]);
        }
        auto const worked = CloneXLightsEffects(target, source, _sequenceElements, erase);
        mainSequencer->PanelEffectGrid->Refresh();
        std::string response = wxString::Format("{\"msg\":\"Model Effects Cloned.\",\"worked\":\"%s\"}", JSONSafe(toStr(worked)));
        return sendResponse(response, "", 200, true);
    } else if (cmd == "addEffect") {
        if (CurrentSeqXmlFile == nullptr) {
            return sendResponse("Sequence not open.", "msg", 503, false);
        }
        auto target = params["target"];
        auto effect = params["effect"];
        auto settings = params["settings"];
        auto palette = params["palette"];
        Element* to = _sequenceElements.GetElement(target);
        int startTime = 0;
        int endTime = CurrentSeqXmlFile->GetSequenceDurationMS();
        int layer = 0;

        if (!params["layer"].empty()) {
            layer = std::stoi(params["layer"]);
        }
        if (!params["startTime"].empty()) {
            startTime = std::stoi(params["startTime"]);
        }
        if (!params["endTime"].empty()) {
            endTime = std::stoi(params["endTime"]);
        }

        if (to == nullptr) {
            return sendResponse("target element doesnt exists.", "msg", 503, false);
        }
        _sequenceElements.get_undo_mgr().CreateUndoStep();
        while (to->GetEffectLayerCount() < layer) {
            to->AddEffectLayer();
        }
        auto valid = to->GetEffectLayer(layer)->AddEffect(0, effect, settings, palette,
                                                          startTime, endTime, 0, false);
        mainSequencer->PanelEffectGrid->Refresh();
        std::string response = wxString::Format("{\"msg\":\"Added Effects.\",\"worked\":\"%s\"}", JSONSafe(toStr(valid != nullptr)));
        return sendResponse(response, "", 200, true);
    }


    return false;
}


bool xLightsFrame::ProcessHttpRequest(HttpConnection &connection, HttpRequest &request) {
    wxString uri = request.URI();
    wxString params = "";
    
    if (uri.find('?') != std::string::npos) {
        params = uri.substr(uri.find('?') + 1);
        uri = uri.substr(0, uri.find('?'));
    }
    std::vector<std::string> paths;
    std::map<std::string, std::string> paramMap = ParseParams(params);
    
    if (uri[0] == '/') {
        uri = uri.substr(1);
    }
    while (uri.find('/') != std::string::npos) {
        wxString p = uri.substr(0, uri.find('/'));
        paths.push_back(wxURI::Unescape(p));
        uri = uri.substr(uri.find('/') + 1);
    }
    paths.push_back(wxURI::Unescape(uri));

    wxString accept = request["Accept"];
    if (paths[0] == "xlDoAutomation") {
        paths.clear();
        params.clear();
        accept = MIME_JSON;
        
        wxJSONValue val;
        wxJSONReader reader;
        if (reader.Parse(request.Data(), &val) == 0) {
            if (!val.HasMember("cmd")) {
                HttpResponse resp(connection, request, (HttpStatus::HttpStatusCode)503);
                resp.AddHeader("access-control-allow-origin", "*");
                resp.MakeFromText("{\"res\":503,\"msg\":\"Missing cmd.\"}", MIME_JSON);
                connection.SendResponse(resp);
                return true;
            } else {
                for (auto &mn : val.GetMemberNames()) {
                    wxJSONValue v = val[mn];
                    if (mn == "cmd") {
                        paths.push_back(v.AsString());
                    } else if (v.IsString()) {
                        paramMap[mn] = v.AsString();
                    } else if (v.IsLong()) {
                        paramMap[mn] = std::to_string(v.AsLong());
                    } else if (v.IsInt()) {
                        paramMap[mn] = std::to_string(v.AsInt());
                    } else if (v.IsBool()) {
                        paramMap[mn] = v.AsBool() ? "true" : "false";
                    } else if (v.IsArray()) {
                        for (int x = 0; x < v.Size(); x++) {
                            std::string k = mn.ToStdString() + "_" + std::to_string(x);
                            paramMap[k] = v[x].AsString();
                        }
                    }
                }

                if (paramMap.empty()) {
                    paramMap["_METHOD"] = "GET";
                } else {
                    paramMap["_METHOD"] = "POST";
                }
            }
        }
    } else {
        paramMap["_METHOD"] = request.Method();
        if (request.Method() == "POST" || request.Method() == "PUT")  {
            paramMap["_DATA"] = request.Data();
        }
    }

    return ProcessAutomation(paths, paramMap, [&] (const std::string &msg,
                                                   const std::string &jsonKey,
                                                   int responseCode,
                                                   bool isJson) {
        HttpResponse resp(connection, request, (HttpStatus::HttpStatusCode)responseCode);
        resp.AddHeader("access-control-allow-origin", "*");

        if (accept == MIME_JSON) {
            if (isJson) {
                if (jsonKey == "") {
                    resp.MakeFromText(msg, MIME_JSON);
                } else {
                    wxString json = "{\"" + jsonKey + "\":" + msg + "}";
                    resp.MakeFromText(json, MIME_JSON);
                }
            } else {
                wxString json = "{\"" + jsonKey + "\":\"" + msg + "\"}";
                resp.MakeFromText(json, MIME_JSON);
            }
        } else if (isJson) {
            resp.MakeFromText(msg, MIME_JSON);
        } else {
            resp.MakeFromText(msg, MIME_TEXT);
        }
        connection.SendResponse(resp);
        return true;
    });
}


void xLightsFrame::StartAutomationListener()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_automationServer != nullptr) {
        _automationServer->Stop();
        delete _automationServer;
        _automationServer = nullptr;
    }

    if (_xFadePort == 0) return;
    
    
    HttpServer *server = new HttpServer();
    HttpContext ctx;
    ctx.Port = ::GetxFadePort(_xFadePort);
    
    ctx.RequestHandler = HttpRequestFunction;
    ctx.MessageHandler = nullptr;

    // default error pages content
    ctx.ErrorPage400 = HTTP_ERROR_PAGE;
    ctx.ErrorPage404 = HTTP_ERROR_PAGE;
    
    if (!server->Start(ctx)) {
        logger_base.debug("xLights Automation could not listen on %d", ::GetxFadePort(_xFadePort));
        delete server;
        return;
    }
    logger_base.debug("xLights Automation listening on %d", ::GetxFadePort(_xFadePort));
    _automationServer = server;
}

std::string xLightsFrame::ProcessxlDoAutomation(const std::string& msg)
{
    std::vector<std::string> paths;
    std::map<std::string, std::string> paramMap;

    
    wxJSONValue val;
    wxJSONReader reader;
    if (reader.Parse(msg, &val) == 0) {
        if (!val.HasMember("cmd")) {
            return "{\"res\":504,\"msg\":\"Missing cmd.\"}";
        } else {
            for (auto &mn : val.GetMemberNames()) {
                wxJSONValue v = val[mn];
                if (mn == "cmd") {
                    paths.push_back(v.AsString());
                } else if (v.IsString()) {
                    paramMap[mn] = v.AsString();
                } else if (v.IsLong()) {
                    paramMap[mn] = std::to_string(v.AsLong());
                } else if (v.IsInt()) {
                    paramMap[mn] = std::to_string(v.AsInt());
                } else if (v.IsBool()) {
                    paramMap[mn] = v.AsBool() ? "true" : "false";
                } else if (v.IsArray()) {
                    for (int x = 0; x < v.Size(); x++) {
                        std::string k = mn.ToStdString() + "_" + std::to_string(x);
                        paramMap[k] = v[x].AsString();
                    }
                }
            }

            if (paramMap.empty()) {
                paramMap["_METHOD"] = "GET";
            } else {
                paramMap["_METHOD"] = "POST";
            }

            std::string result;
            bool processed = ProcessAutomation(paths, paramMap, [&](const std::string &msg,
                                                                    const std::string &jsonKey,
                                                                    int responseCode,
                                                                    bool isJson) {
                if (isJson) {
                    if (jsonKey == "") {
                        result = "{\"res\":" + std::to_string(responseCode) +"," + msg.substr(1);
                    } else {
                        result = "{\"res\":" + std::to_string(responseCode) +",\"" + jsonKey + "\":" + msg + "}";
                    }
                } else {
                    result = "{\"res\":" + std::to_string(responseCode) +",\"" + jsonKey + "\":\"" + msg + "\"}";
                }
                return true;
            });
            
            if (!processed) {
                auto cmd = val["cmd"].AsString();
                return wxString::Format("{\"res\":504,\"msg\":\"Unknown command: '%s'.\"}", cmd);
            }
            return result;
        }
    }
    return "{\"res\":504,\"msg\":\"Error parsing request.\"}";
}
 
/*

            

 } else if (cmd == "runDiscovery") {
     return "{\"res\":504,\"msg\":\"Not implemented.\"}";
     // TODO
 } else if (cmd == "exportModel") {
     return "{\"res\":504,\"msg\":\"Not implemented.\"}";
     // TODO
     // pass in name of the file to write to ... pass back the name of the file written to
 } else if (cmd == "exportModelAsCustom") {
     return "{\"res\":504,\"msg\":\"Not implemented.\"}";
     // TODO
     // pass in name of the file to write to ... pass back the name of the file written to

            } else if (cmd == "shiftAllEffects") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // TODO
                // pass in number of MS
            } else if (cmd == "shiftSelectedEffects") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // TODO
            } else if (cmd == "unselectEffects") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // TODO
            } else if (cmd == "selectEffectsOnModel") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // TODO
            } else if (cmd == "selectAllEffectsOnAllModels") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // TODO

            } else if (cmd == "turnOnOutputToLights") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // TODO
            } else if (cmd == "turnOffOutputToLights") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // TODO
            } else if (cmd == "playSequence") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // TODO
            } else if (cmd == "printLayout") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // TODO
            } else if (cmd == "printWiring") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // TODO
            } else if (cmd == "exportLayoutImage") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // TODO
            } else if (cmd == "exportWiringImage") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // TODO
            } else if (cmd == "cleanupFileLocations") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // TODO
            } else if (cmd == "hinksPixExport") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // TODO
            } else if (cmd == "purgeDownloadCache") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // MEDIUM PRIORITY
                // TODO
            } else if (cmd == "purgeRenderCache") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // MEDIUM PRIORITY
                // TODO
            } else if (cmd == "convertSequence") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // TODO
            } else if (cmd == "prepareAudio") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // TODO
            } else if (cmd == "resetToDefaults") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // MEDIUM PRIORITY
                // TODO
            } else if (cmd == "resetWindowLayout") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // MEDIUM PRIORITY
                // TODO
            } else if (cmd == "setAudioVolume") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // TODO
            } else if (cmd == "setAudioSpeed") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // TODO
            } else if (cmd == "gotoZoom") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // TODO
            } else if (cmd == "importSequence") {
                return "{\"res\":504,\"msg\":\"Not implemented.\"}";
                // TODO
             else {
                return wxString::Format("{\"res\":504,\"msg\":\"Unknown command: '%s'.\"}", cmd);
            }
        }
    } else {
        return "{\"res\":504,\"msg\":\"Error parsing request.\"}";
    }
}
*/
