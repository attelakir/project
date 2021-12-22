// Created by attelakir

#include "Widget.h"

#include <wx/wx.h>

class MyApp : public wxApp
{
public:
    virtual bool OnInit();
};

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit( ) {
    mainWindow *main = new mainWindow(wxT("Listbox"));
    main->Show(true);
    return true;
}