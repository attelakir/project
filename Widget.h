// Created by attelakir

#ifndef UNTITLED9_WIDGET_H
#define UNTITLED9_WIDGET_H

#include <map>

#include <wx/listbox.h>
#include <wx/textdlg.h>
#include <wx/wx.h>

struct conMngr {
    size_t conCounter = 0;
    std::map<uint32_t, size_t> flowKey;
    std::map<size_t, size_t> packetsCounterSide0;
    std::map<size_t, size_t> packetsCounterSide1;
    wxListBox *listbox;
};

class CustomDialog : public wxDialog {
public:
    CustomDialog(const wxString &title);
    wxListBox *listbox;
    std::string interfaceIP;
    void OnDblClick(wxCommandEvent &event);
};

class Connections : public wxPanel {
public:
    Connections(wxPanel *parent);
    wxListBox *listbox;
    wxListBox *m_pack1;
    wxListBox *m_pack2;
    void OnDblClick(wxCommandEvent &event);
    conMngr mngr;
    int selectNumb;
};

class Packets1 : public wxPanel {
public:
    Packets1(wxPanel *parent);
    wxListBox *listbox;
    wxTextCtrl *m_textctrl;
    void OnDblClick(wxCommandEvent &event);
    int *conNumber;
};

class Packets2 : public wxPanel {
public:
    Packets2(wxPanel *parent);
    wxListBox *listbox;
    wxTextCtrl *m_textctrl;
    void OnDblClick(wxCommandEvent &event);
    int *conNumber;
};

class Info : public wxPanel {
public:
    Info(wxPanel *parent);
    wxTextCtrl *textctrl;
};

class Buttons : public wxPanel {
public:
    Buttons(wxPanel *parent);
    wxButton *button1;
    wxButton *button2;
    void Start(wxCommandEvent &event);
    void Clear(wxCommandEvent &event);
    std::string interface;
    conMngr *m_mngr;
};

class mainWindow : public wxFrame {
public:
    mainWindow(const wxString &title);
    Connections *con;
    Packets1 *pack1;
    Packets2 *pack2;
    Info *info;
    Buttons *buttons;
};
#endif//UNTITLED9_WIDGET_H
