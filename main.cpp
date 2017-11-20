#include <iostream>
#include <wx-3.0/wx/wxprec.h>
#include <wx-3.0/wx/filedlg.h>
#include <wx/wfstream.h>
#include <wx-3.0/wx/textfile.h>
#include <wx-3.0/wx/notebook.h>

#ifndef WX_PRECOMP

#include <wx-3.0/wx/wx.h>

#endif

#include <vector>
#include <sstream>

#ifdef __UNIX_LIKE__

#include <pwd.h>
#include <array>
#include <memory>
#include <future>

#endif

using namespace std;

enum {
    ID_New = 1,
    ID_Open = 2,
    ID_SaveAs = 3,
    ID_Save = 4,
    ID_NewTab = 5,
    ID_NoteBook = 6,
    ID_Run = 7,
    ID_ConsoleOut = 8,
};

string currentProjectPath;
char *IDEFolder = new char(); // NOLINT

class MyApp : public wxApp {
public:
    bool OnInit() override;
};

class MyFrame : public wxFrame {
public:
    MyFrame();

private:

    void OnNew(wxCommandEvent &event);

    void OnExit(wxCommandEvent &event);

    void OnAbout(wxCommandEvent &event);

    void OnOpen(wxCommandEvent &event);

    void OnSaveAs(wxCommandEvent &event);

    void OnSave(wxCommandEvent &event);

    void OnNewTab(wxCommandEvent &event);

    void OnResize(wxSizeEvent &event);

    void OnRun(wxCommandEvent &event);

    //void OnStatusBarClick(wxCommandEvent &event);
};

class TextTab {
public:
    wxTextCtrl *text{};
    wxString name;
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
    wxString filePath;
#pragma clang diagnostic pop
    wxPanel *panel{};
    int id{};
    int panelId{};
    int textId{};

    void createPanel(wxNotebook *nb, int width, int height, int id_) {
        id = id_;
        panelId = id++;
        panel = new wxPanel(nb, panelId);
        panel->SetSize(panel->GetParent()->GetSize());
        textId = id++;
        text = new wxTextCtrl(panel, textId, wxT(""), wxDefaultPosition, wxSize(width, height),
                              wxTE_MULTILINE | wxTE_DONTWRAP | wxHSCROLL | wxVSCROLL);
        text->SetSize(text->GetParent()->GetSize());
    }
};

void SaveAs(wxWindow *mf);

void getData(const wxString &in, vector<string> *files, string *projectName);

bool startsWith(const char a[], const char b[]);

bool exists(const wxString &pathname);

bool fexists(const wxString &pathname);

string getFilename(char path[]);

//void colorCode(TextTab tt);

string getFullFilename(char path[]);

void readConsole(const char *cmd);

void Save();

void resize();

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
void Alarm(int sig);

#pragma clang diagnostic pop

//int popen3(int fd[3], const char **const cmd);

wxString path; // NOLINT

vector<TextTab> textTabs;
wxWindow *window;
wxNotebook *nb;
wxString curProjectName; // NOLINT
wxTextCtrl *wtc;

wxIMPLEMENT_APP(MyApp); // NOLINT

class sm_eventTable;

bool MyApp::OnInit() {
    MyFrame *frame;
    frame = new MyFrame();
    frame->Show(true);
    return true;
}


MyFrame::MyFrame() : wxFrame(nullptr, wxID_ANY, "P5c IDE") {
    path = "";
    this->SetMinSize(wxSize(200, 200));
    //Find Home directory
#ifdef USERPROFILE
#define windows 1
    path = {$USERPROFILE};
#endif
#ifdef HOMEDRIVE
#define windows 1
#ifdef HOMEPATH
    path = {$HOMEDRIVE}+{$HOMEPATH};
#endif
#endif
#ifdef __UNIX_LIKE__
#ifdef HOME
    path = {$HOME};
#else
    struct passwd *pwd = getpwuid(getuid());
    path = wxString(string(pwd->pw_dir));
#endif
#endif
    //End Find Home directory
    //Find current directory for G++
#ifdef __LINUX__
    readlink("/proc/self/exe", IDEFolder, 256);
#endif
#ifdef _WIN32
    GetModuleFileName(NULL, (char *) IDEFolder, 256);
#endif
    //End Find current directory for G++
    this->SetSize(800, 600);
    Center();
    window = new wxWindow(this, -1, wxDefaultPosition, wxSize(this->GetSize().x, this->GetSize().y - 118));
    auto *menuFile = new wxMenu;
    nb = new wxNotebook(window, ID_NoteBook, wxDefaultPosition,
                        wxSize(window->GetSize().x, window->GetSize().y / 2 + 50), wxNB_MULTILINE);
    TextTab tt = TextTab();
    tt.createPanel(nb, nb->GetSize().x, nb->GetSize().y, static_cast<int>(nb->GetPageCount()));
    nb->AddPage(tt.panel, "New Page");
    textTabs.emplace_back(tt);
    wtc = new wxTextCtrl(window, ID_ConsoleOut, wxT(""), wxPoint(0, window->GetSize().y / 2 + 75),
                         wxSize(window->GetSize().x, window->GetSize().y / 2 - 75),
                         wxTE_MULTILINE | wxTE_DONTWRAP | wxTE_READONLY);
    menuFile->Append(ID_New, "&New project...\tCtrl-N", "Create a new P5c project");
    menuFile->Append(ID_NewTab, "&New tab...\tCtrl-T", "Create a new file in project");
    menuFile->Append(ID_Open, "&Open...\tCtrl-O", "Open an existing P5c project");
    menuFile->Append(ID_Save, "&Save...\tCtrl-S", "Save P5c project");
    menuFile->Append(ID_Run, "&Run the project...\tCtrl-R", "Run your P5c project");
    menuFile->Append(ID_SaveAs, "&Save as...\tCtrl-Shift-S", "Save P5c project as a new project");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);
    auto *menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);
    auto *menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuHelp, "&Help");
    SetMenuBar(menuBar);
    CreateStatusBar();
    SetStatusText("Welcome to P5c IDE!");
    Bind(wxEVT_MENU, &MyFrame::OnNew, this, ID_New);
    Bind(wxEVT_MENU, &MyFrame::OnOpen, this, ID_Open);
    Bind(wxEVT_MENU, &MyFrame::OnSave, this, ID_Save);
    Bind(wxEVT_MENU, &MyFrame::OnSaveAs, this, ID_SaveAs);
    Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);
    Bind(wxEVT_MENU, &MyFrame::OnNewTab, this, ID_NewTab);
    Bind(wxEVT_MENU, &MyFrame::OnRun, this, ID_Run);
//    Bind(wxEVT_MENU, &MyFrame::OnStatusBarClick, this, GetStatusBar()->GetId());
    Bind(wxEVT_SIZE, &MyFrame::OnResize, this);
    signal(SIGEV_SIGNAL, Alarm);
    alarm(1);
}

void MyFrame::OnExit(wxCommandEvent &event) {
    Close(true);
}

void MyFrame::OnAbout(wxCommandEvent &event) {
    wxMessageBox(
            "This is a P5c IDE.\nP5c is a C++ alternative to Java's Processing.\nEverything made and built by MGlolenstine.\nSend bug reports and suggestions to mglolenstine@gmail.com",
            "About P5c IDE", wxOK | wxICON_INFORMATION);
}

void MyFrame::OnNew(wxCommandEvent &event) {

}

void MyFrame::OnOpen(wxCommandEvent &event) {
    wxString sPath = path + "/P5c-sketchbook/";
    if (!exists(sPath)) {
        mode_t nMode = 0733; // UNIX style permissions
        int nError = 0;
#if defined(_WIN32)
        nError = _mkdir(sPath.c_str()); // can be used on Windows
#else
        nError = mkdir(sPath.c_str(), nMode); // can be used on non-Windows
#endif
        if (nError != 0) {
            wxLogError("Folder P5c-sketchbook couldn't be created in " + path);
        }
    }
    wxFileDialog
            openFileDialog(this, _("Open existing project"), path + "/P5c-sketchbook", "",
                           "P5c project files (*.p5c) |*.p5c", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (openFileDialog.ShowModal() == wxID_CANCEL)
        return;     // the user changed idea...

    // proceed loading the file chosen by the user;
    // this can be done with e.g. wxWidgets input streams:
    wxFileInputStream input_stream(openFileDialog.GetPath());
    if (!input_stream.IsOk()) {
        wxLogError("Cannot open file '%s'.", openFileDialog.GetPath());
        return;
    }
    wxString filename = getFilename(const_cast<char *>(openFileDialog.GetPath().c_str().AsChar()));
    wxString folder = openFileDialog.GetPath().substr(0, openFileDialog.GetPath().length() - getFullFilename(
            const_cast<char *>(openFileDialog.GetPath().c_str().AsChar())).length());
    currentProjectPath = folder;
    //folder = folder.substr(0, folder.length() - filename.length() - 1);
    vector<string> files;
    string projectName;
    getData(openFileDialog.GetPath(), &files, &projectName);
    textTabs.clear();
    nb->DeleteAllPages();
    //nb->SetSize(wxSize(window->GetSize().x, window->GetSize().y/2-25));
    for (auto &file : files) {
        TextTab tmp = TextTab();
        wxTextFile f;
        f.Open(folder + "/" + file);
        tmp.filePath = folder + "/" + file;
        tmp.name = getFilename(const_cast<char *>(file.c_str()));
        //cout << "Opening: " << folder + "/" + files.at(i) + ".cpp" << endl;
        wxString value = f.GetFirstLine();
        while (!f.Eof()) {
            value += "\n" + f.GetNextLine();
        }
        tmp.createPanel(nb, nb->GetSize().x, nb->GetSize().y, static_cast<int>(nb->GetPageCount()));
        tmp.text->SetValue(value);
        nb->AddPage(tmp.panel, tmp.name);
        textTabs.emplace_back(tmp);
    }
    alarm(1);
    //resize();
    curProjectName = projectName;

    char programName[] = "P5c IDE - ";
    projectName = programName + projectName;
    this->SetLabel(projectName);
}

void MyFrame::OnSave(wxCommandEvent & WXUNUSED(event)) {

    // save the current contents in the file;
    // this can be done with e.g. wxWidgets output streams:
    if (currentProjectPath.empty()) {
        SaveAs(this);
        return;
    }
    //wxFileOutputStream output_stream(currentProjectPath);
    if (!fexists(currentProjectPath)) {
        //wxLogError("Cannot save current contents in file '%s'.", currentProjectPath);
        SaveAs(this);
        return;
    }
    Save;
}

void MyFrame::OnSaveAs(wxCommandEvent & WXUNUSED(event)) {
    SaveAs(this);
}

void MyFrame::OnNewTab(wxCommandEvent & WXUNUSED(event)) {
    TextTab tmp = TextTab();
    tmp.createPanel(nb, m_width, m_height, static_cast<int>(nb->GetPageCount()));
    nb->AddPage(tmp.panel, "New tab");
    textTabs.emplace_back(tmp);
}

void MyFrame::OnResize(wxSizeEvent &event) {
    window->SetSize(event.GetSize().x, event.GetSize().y);
    resize();
}

void MyFrame::OnRun(wxCommandEvent & WXUNUSED(event)) {
    puts(currentProjectPath + curProjectName + ".p5c");
    if (!fexists(currentProjectPath + curProjectName + ".p5c")) {
        //wxLogError("Cannot save current contents in file '%s'.", currentProjectPath);
        SaveAs(this);
        return;
    }
    if (fexists(currentProjectPath + curProjectName + ".p5c")) {
        Save();
        //cout<<"Your program is now being run!"<<endl;
        //cout<<"g++ "+currentProjectPath+curProjectName+".cpp -o "+currentProjectPath+"output && cd / &&."+currentProjectPath+"output"<<endl;
        // gnome-terminal --
        readConsole("g++ " + currentProjectPath + curProjectName + ".cpp -o " + currentProjectPath +
                    "output && cd / && sleep 1 &&." + currentProjectPath + "output");
        //system("gnome-terminal -- g++ "+currentProjectPath+curProjectName+".cpp -o "+currentProjectPath+"output && cd / &&."+currentProjectPath+"output");
    } else {
        cout << "Project path is empty!" << endl;
    }
}

//void MyFrame::OnStatusBarClick(wxCommandEvent & WXUNUSED(event)) {
//    cout << "I've been clicked!" << endl;
//}

void SaveAs(wxWindow *mf) {
    wxString sPath = path + "/P5c-sketchbook";
    if (!exists(sPath)) {
        mode_t nMode = 0733; // UNIX style permissions
        int nError = 0;
#if defined(_WIN32)
        nError = _mkdir(sPath.c_str()); // can be used on Windows
#else
        nError = mkdir(sPath.c_str(), nMode); // can be used on non-Windows
#endif
        if (nError != 0) {
            wxLogError("Folder P5c-sketchbook couldn't be created in " + sPath);
        }
    }
    wxFileDialog
            saveFileDialog(mf, _("Save P5c project"), sPath, "untitled",
                           "P5c project folder | *.*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    //"P5c project files (*.p5c) |*.p5c"
    if (saveFileDialog.ShowModal() == wxID_CANCEL)
        return;     // the user changed idea...

    // save the current contents in the file;
    // this can be done with e.g. wxWidgets output streams:
    wxString filename = getFilename(const_cast<char *>(saveFileDialog.GetPath().c_str().AsChar()));
    wxString sPath1 = saveFileDialog.GetPath();
    if (!exists(sPath1)) {
        mode_t nMode = 0733; // UNIX style permissions
        int nError = 0;
#if defined(_WIN32)
        nError = _mkdir(sPath.c_str()); // can be used on Windows
#else
        nError = mkdir(sPath1.c_str(), nMode); // can be used on non-Windows
#endif
        if (nError != 0) {
            wxLogError("Folder " + sPath1 + " couldn't be created in " +
                       saveFileDialog.GetPath().substr(0, saveFileDialog.GetPath().length() - filename.length()));
            return;
        }
    }

//    wxFileOutputStream output_stream(sPath1 + "/" + filename + ".p5c");
//    if (!output_stream.IsOk()) {
//        wxLogError("Cannot save current contents in file '%s'.", sPath1);
//        return;
//    }
    currentProjectPath = sPath1 + "/";
    curProjectName = filename;
    wxTextFile projectFile(sPath1 + "/" + filename + ".p5c");
    // Create new project file
    textTabs.at(0).name = filename;
    // TODO: Add settable names for tabs (input name at creation)
    if (!projectFile.Exists()) {
        projectFile.Create();
    }
    //projectFile.Open(sPath1 + "/" + filename + ".p5c");
    projectFile.AddLine("name: " + filename);
    projectFile.AddLine("file: " + filename + ".cpp");
    projectFile.Write();
    projectFile.Close();
    // Create new Main file from current one
    if (!textTabs.empty()) {
        for (auto &textTab : textTabs) {
            //cout << "Opening file" << endl;
            if (!exists(sPath1 + "/" + textTab.name + ".cpp")) {
                projectFile.Create(sPath1 + "/" + textTab.name + ".cpp");
            }
            projectFile.Open(sPath1 + "/" + textTab.name + ".cpp");
            //projectFile.Open(sPath1 + "/" + textTabs.at(i).name + ".cpp");
            //cout << "Opened file" << endl;
            for (int j = 0; j < textTab.text->GetNumberOfLines(); j++) {
                //cout << "Adding line to file "<<filename << ": " << textTabs.at(i).text->GetLineText(j) << endl;
                projectFile.AddLine(textTab.text->GetLineText(j));
            }
            projectFile.Write();
            projectFile.Close();
        }
    }
    vector<string> files;
    string projectName;
    //cout << "Getting data!" << endl;
    getData(sPath1 + "/" + filename + ".p5c", &files, &projectName);
    //cout << "Data received!" << endl;
    char programName[] = "P5c IDE - ";
    projectName = programName + projectName;
    //cout << "Setting program name!" << endl;
    mf->SetLabel(projectName);
}

void Save() {
    if (fexists(currentProjectPath)) {
        //wxLogError("Cannot save current contents in file '%s'.", currentProjectPath);
        //SaveAs(this);
        wxTextFile tf;
        for (TextTab tt : textTabs) {
            tf.Open(currentProjectPath + tt.name + ".cpp");
            tf.Clear();
            for (int i = 0; i < tt.text->GetNumberOfLines(); i++) {
                tf.AddLine(tt.text->GetLineText(i));
            }
            tf.Write();
            tf.Close();
        }
        return;
    }
}

string getFilename(char path[]) {
    int lastIndex = 0;
    char filename[256];
    for (char &i : filename) {
        i = '\0';
    }
    for (int i = 0; i < strlen(path); i++) {
        if (path[i] == '/') {
            lastIndex = i + 1;
        }
    }
    for (int i = lastIndex; i < strlen(path); i++) {
        if (path[i] == '.') {
            break;
        } else {
            filename[i - lastIndex] = path[i];
        }
    }
    string ret = filename;
    return ret;
}

void getData(const wxString &in, vector<string> *files, string *projectName) {
    //string input = in;
    vector<wxString> lines;
    wxTextFile tfile;
    tfile.Open(in);
    wxString tmp = tfile.GetFirstLine();
    lines.push_back(tmp);
    while (!tfile.Eof()) {
        tmp = tfile.GetNextLine();
        if (!tmp.empty())
            lines.push_back(tmp);
    }
    for (wxString s : lines) {
        if (startsWith(s.c_str().AsChar(), "file: ")) {
            s = s.substr(sizeof("file: ") - 1, strlen(s) - sizeof("file: ") + 1);
            files->push_back(s.c_str().AsChar());
            //cout << s << endl;
        } else if (startsWith(s.c_str().AsChar(), "name: ")) {
            s = s.substr(sizeof("name: ") - 1, strlen(s) - sizeof("name: ") + 1);
            *projectName = std::string(s.mb_str());
        }
    }
}

bool startsWith(const char a[], const char b[]) {
    for (int i = 0; i < strlen(b); i++) {
        if (a[i] != b[i]) {
            return false;
        }
    }
    return true;
}

bool exists(const wxString &pathname) {
    struct stat info{};
    stat(pathname, &info);
    return (info.st_mode & S_IFDIR) != 0;
}

bool fexists(const wxString &pathname) {
    struct stat info{};
    return (stat(pathname.c_str(), &info) == 0);
}

//void colorCode(TextTab tt) {
//    wxString text[tt.text->GetNumberOfLines()];
//    int n = nb->GetSelection();
//    for (TextTab tt : textTabs) {
//        if (tt.id == n) {
//            for (int i = 0; i < tt.text->GetNumberOfLines(); i++) {
//                text[i] = tt.text->GetLineText(i);
//            }
//        }
//    }
//    for (int i = 0; i < text->size(); i++) {
//        cout << text[i] << endl;
//    }
//    return;
//}

string getFullFilename(char path[]) {
    int lastIndex = 0;
    char filename[256];
    for (char &i : filename) {
        i = '\0';
    }
    for (int i = 0; i < strlen(path); i++) {
        if (path[i] == '/') {
            lastIndex = i + 1;
        }
    }
    for (int i = lastIndex; i < strlen(path); i++) {
        filename[i - lastIndex] = path[i];
    }
    string ret = filename;
    return ret;
}

void readConsole(const char *cmd) {
    array<char, 128> buffer{};
    string result;
    shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    //TextTab tmp;
    //tmp.createPanel(nb, nb->GetSize().x, nb->GetSize().y, id);
    //nb->AddPage(tmp.panel, "Command Output");
    if (!pipe) throw runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr) {
            result += buffer.data();
            wtc->SetValue(result);
        }
    }
}

void resize() {
    //window->SetSize();
    nb->SetSize(wxSize(window->GetSize().x, window->GetSize().y / 2 + 50));
    wtc->SetPosition(wxPoint(0, window->GetSize().y / 2 + 75));
    wtc->SetSize(wxSize(window->GetSize().x, window->GetSize().y / 2 - 75));
    for (TextTab tt : textTabs) {
        tt.text->SetSize(tt.text->GetParent()->GetSize());
        tt.panel->SetSize(tt.panel->GetParent()->GetSize());
    }
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
void Alarm(int sig) {
#pragma clang diagnostic pop
    resize();
}