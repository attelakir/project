// Created by attelakir

#include "LRUList.h"
#include "SystemUtils.h"
#include "TcpReassembly.h"
#include <cmath>
#include <map>
#include <sstream>

#include "TCPSniffer.h"
#include "Widget.h"
#include "mysql_connection.h"
#include "mysql_driver.h"

#include <cppconn/driver.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

#include <wx/listbox.h>
#include <wx/textdlg.h>
#include <wx/wx.h>

void Entrance(){
    sql::mysql::MySQL_Driver *driver = sql::mysql::get_mysql_driver_instance();
    sql::Connection *con = driver->connect("tcp://127.0.0.1:3306", "root", "rootpass");
    std::auto_ptr<sql::Statement> stmt(con->createStatement());

    stmt->execute("CREATE DATABASE IF NOT EXISTS TCP_DATA");
    stmt->execute("USE TCP_DATA");
    stmt->execute("CREATE TABLE IF NOT EXISTS PacketsSide0(ConNumber INTEGER, Time TEXT, PacketNumber INTEGER)");
    stmt->execute("CREATE TABLE IF NOT EXISTS PacketsSide1(ConNumber INTEGER, Time TEXT, PacketNumber INTEGER)");
    stmt->execute("CREATE TABLE IF NOT EXISTS DataFrom0(ConNumber INTEGER, PacketNumber INTEGER, Data TEXT)");
    stmt->execute("CREATE TABLE IF NOT EXISTS DataFrom1(ConNumber INTEGER, PacketNumber INTEGER, Data TEXT)");

    stmt->execute("DELETE FROM PacketsSide0");
    stmt->execute("DELETE FROM DataFrom0");
    stmt->execute("DELETE FROM PacketsSide1");
    stmt->execute("DELETE FROM DataFrom1");

    delete con;
}

mainWindow::mainWindow(const wxString &title) : wxFrame(NULL, wxID_ANY, "TCPSniffer", wxDefaultPosition, wxSize(1035, 700)) {

    Entrance();

    wxPanel *panel = new wxPanel(this, -1);
    wxColour col;
    col.Set("#4f5049");
    panel->SetBackgroundColour(col);

    CustomDialog custom(wxT("Interface"));

    wxFlexGridSizer *fgs = new wxFlexGridSizer(2, 2, 5, 5);

    con = new Connections(panel);
    fgs->Add(con, 1, wxEXPAND | wxALL, 5);

    pack1 = new Packets1(panel);
    pack2 = new Packets2(panel);
    wxBoxSizer *hbox1 = new wxBoxSizer(wxHORIZONTAL);
    hbox1->Add(pack1, 1, wxEXPAND | wxALL, 5);
    hbox1->Add(pack2, 1, wxEXPAND | wxALL, 5);
    fgs->Add(hbox1);

    info = new Info(panel);
    fgs->Add(info, 1, wxEXPAND | wxALL, 5);

    buttons = new Buttons(panel);
    buttons->interface = custom.interfaceIP;

    fgs->Add(buttons, 1, wxEXPAND | wxALL, 5);

    panel->SetSizer(fgs);
    this->Centre();
}

CustomDialog::CustomDialog(const wxString &title) : wxDialog(NULL, -1, title, wxDefaultPosition, wxSize(450, 380)) {
    sql::mysql::MySQL_Driver *driver = sql::mysql::get_mysql_driver_instance();
    sql::Connection *con = driver->connect("tcp://127.0.0.1:3306", "root", "rootpass");

    listInterfaces();

    listbox = new wxListBox(this, 5, wxPoint(-1, -1), wxSize(200, 200));

    wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);

    std::auto_ptr<sql::Statement> stmt(con->createStatement());
    stmt->execute("USE TCP_DATA");
    stmt->execute("SELECT Name, IP_address FROM network_interfaces;");
    std::auto_ptr<sql::ResultSet> res;
    std::stringstream buf;

    do {
        res.reset(stmt->getResultSet());
        while (res->next()) {
            buf << "Name:    " << (std::string)(res->getString("Name")) << "      IP address:    "
                << (std::string)(res->getString("IP_address"));
            listbox->Append(wxString(buf.str()));
            buf.str("");
        }
    } while (stmt->getMoreResults());

    Connect(wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler(CustomDialog::OnDblClick));

    hbox->Add(listbox, 1, wxEXPAND);
    SetSizer(hbox);
    delete con;
    ShowModal();
    Destroy();
}

void CustomDialog::OnDblClick(wxCommandEvent &event) {
    interfaceIP = listbox->GetString(listbox->GetSelection());
    interfaceIP = interfaceIP.substr(interfaceIP.rfind(' ') + 1);
    EndModal(1);
}

Packets1::Packets1(wxPanel *parent) : wxPanel(parent, wxID_ANY) {

    wxPanel *panel = new wxPanel(this, -1);
    listbox = new wxListBox(panel, 5, wxPoint(-1, -1), wxSize(250, 400));

    wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
    hbox->Add(panel, 1, wxEXPAND);

    Connect(wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler(Packets1::OnDblClick));

    mainWindow *main = (mainWindow *) parent->GetParent();
    (main->con)->m_pack1 = listbox;
    conNumber = &(main->con->selectNumb);

    SetSizer(hbox);
}

Packets2::Packets2(wxPanel *parent) : wxPanel(parent, wxID_ANY) {
    listbox = new wxListBox(this, 5, wxPoint(-1, -1), wxSize(250, 400));

    Connect(wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler(Packets2::OnDblClick));

    mainWindow *main = (mainWindow *) parent->GetParent();
    (main->con)->m_pack2 = listbox;
    conNumber = &(main->con->selectNumb);
}

Connections::Connections(wxPanel *parent) : wxPanel(parent, wxID_ANY) {
    listbox = new wxListBox(this, 5, wxPoint(-1, -1), wxSize(500, 400));

    mngr = conMngr();

    mainWindow *main = (mainWindow *) parent->GetParent();
    mngr.listbox = listbox;

    Connect(wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler(Connections::OnDblClick));
}

Info::Info(wxPanel *parent) : wxPanel(parent, wxID_ANY) {
    wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
    textctrl = new wxTextCtrl(this, -1, wxT(""), wxPoint(-1, -1), wxSize(500, 200), wxTE_MULTILINE | wxTE_READONLY);
    mainWindow *main = (mainWindow *) parent->GetParent();
    main->pack1->m_textctrl = textctrl;
    main->pack2->m_textctrl = textctrl;
    hbox->Add(textctrl, 1, wxEXPAND);
    SetSizer(hbox);
}

Buttons::Buttons(wxPanel *parent) : wxPanel(parent, wxID_ANY) {
    wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);

    wxColour col;
    col.Set("#4f5049");
    this->SetBackgroundColour(col);
    mainWindow *main = (mainWindow *) parent->GetParent();
    m_mngr = &(main->con->mngr);

    button1 = new wxButton(this, 1, wxT("Start"), wxPoint(-1, -1), wxSize(50, 50));
    button2 = new wxButton(this, 2, wxT("Clear"), wxPoint(-1, -1), wxSize(50, 50));

    hbox->Add(button1, 1, wxEXPAND | wxRIGHT, 5);
    hbox->Add(button2, 1, wxEXPAND | wxLEFT, 5);

    Connect(1, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(Buttons::Start));
    Connect(2, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(Buttons::Clear));

    SetSizer(hbox);
}

void Buttons::Start(wxCommandEvent &event) {
    sql::mysql::MySQL_Driver *driver = sql::mysql::get_mysql_driver_instance();
    sql::Connection *con = driver->connect("tcp://127.0.0.1:3306", "root", "rootpass");

    mainWindow *main = (mainWindow *) GetParent();

    pcpp::TcpReassembly tcpReassembly(tcpReassemblyMsgReadyCallback, m_mngr, tcpReassemblyConnectionStartCallback,
                                      tcpReassemblyConnectionEndCallback);

    pcpp::PcapLiveDevice *dev = pcpp::PcapLiveDeviceList::getInstance().getPcapLiveDeviceByIp(interface);

    dev->open();

    dev->startCapture(onPacketArrives, &tcpReassembly);

    size_t count = 10;

    while (count > 0) {
        pcpp::multiPlatformSleep(1);
        m_mngr->listbox->Update();
        m_mngr->listbox->Refresh();
        --count;
    }

    dev->stopCapture();
    dev->close();

    tcpReassembly.closeAllConnections();

    delete con;
}

void Buttons::Clear(wxCommandEvent &event) {
    sql::mysql::MySQL_Driver *driver = sql::mysql::get_mysql_driver_instance();
    sql::Connection *con = driver->connect("tcp://127.0.0.1:3306", "root", "rootpass");
    std::auto_ptr<sql::Statement> stmt(con->createStatement());

    stmt->execute("USE TCP_DATA");
    stmt->execute("DELETE FROM PacketsSide0");
    stmt->execute("DELETE FROM DataFrom0");
    stmt->execute("DELETE FROM PacketsSide1");
    stmt->execute("DELETE FROM DataFrom1");

    m_mngr->conCounter = 0;
    m_mngr->flowKey.clear();
    m_mngr->packetsCounterSide0.clear();
    m_mngr->packetsCounterSide1.clear();
    m_mngr->listbox->Clear();

    delete con;
}

void Connections::OnDblClick(wxCommandEvent &event) {
    sql::mysql::MySQL_Driver *driver = sql::mysql::get_mysql_driver_instance();
    sql::Connection *con = driver->connect("tcp://127.0.0.1:3306", "root", "rootpass");

    if (!(m_pack1->IsEmpty())) {
        m_pack1->Clear();
    }

    if (!(m_pack2->IsEmpty())) {
        m_pack2->Clear();
    }

    selectNumb = listbox->GetSelection();

    std::stringstream buf_id;
    buf_id << listbox->GetSelection();
    std::string id = buf_id.str();
    buf_id.str("");

    std::auto_ptr<sql::Statement> stmt(con->createStatement());
    stmt->execute("USE TCP_DATA");
    stmt->execute("SET @id = " + id);
    stmt->execute("SELECT Time from PacketsSide0 WHERE ConNumber = @id");
    std::auto_ptr<sql::ResultSet> res0;

    do {
        res0.reset(stmt->getResultSet());
        while (res0->next()) {
            std::string Time = res0->getString("Time");
            m_pack1->Append(wxString(Time));
        }
    } while (stmt->getMoreResults());

    stmt->execute("SELECT Time from PacketsSide1 WHERE ConNumber = @id");
    std::auto_ptr<sql::ResultSet> res1;

    do {
        res1.reset(stmt->getResultSet());
        while (res1->next()) {
            std::string Time = res1->getString("Time");
            m_pack2->Append(wxString(Time));
        }
    } while (stmt->getMoreResults());

    delete con;
}

void Packets1::OnDblClick(wxCommandEvent &event) {
    sql::mysql::MySQL_Driver *driver = sql::mysql::get_mysql_driver_instance();
    sql::Connection *con = driver->connect("tcp://127.0.0.1:3306", "root", "rootpass");

    if (!(m_textctrl->IsEmpty())) {
        m_textctrl->Clear();
    }

    size_t packetNumber = listbox->GetSelection();

    std::stringstream buf_id;
    buf_id << *conNumber;
    std::string conNumb = buf_id.str();
    buf_id.str("");
    buf_id << packetNumber;
    std::string packetNumb = buf_id.str();

    std::auto_ptr<sql::Statement> stmt(con->createStatement());
    stmt->execute("USE TCP_DATA");
    stmt->execute("SET @conNumb = " + conNumb);
    stmt->execute("SET @packetNumb = " + packetNumb);
    stmt->execute("SELECT Data from DataFrom0 WHERE ConNumber = @conNumb AND PacketNumber = @packetNumb");
    std::auto_ptr<sql::ResultSet> res;

    do {
        res.reset(stmt->getResultSet());
        while (res->next()) {
            m_textctrl->SetDefaultStyle(wxTextAttr(*wxRED));
            *m_textctrl << (wxString)(res->getString("Data"));
        }
    } while (stmt->getMoreResults());
    delete con;
}

void Packets2::OnDblClick(wxCommandEvent &event) {
    sql::mysql::MySQL_Driver *driver = sql::mysql::get_mysql_driver_instance();
    sql::Connection *con = driver->connect("tcp://127.0.0.1:3306", "root", "rootpass");

    if (!(m_textctrl->IsEmpty())) {
        m_textctrl->Clear();
    }

    size_t packetNumber = listbox->GetSelection();

    std::stringstream buf_id;
    buf_id << *conNumber;
    std::string conNumb = buf_id.str();
    buf_id.str("");
    buf_id << packetNumber;
    std::string packetNumb = buf_id.str();

    std::auto_ptr<sql::Statement> stmt(con->createStatement());
    stmt->execute("USE TCP_DATA");
    stmt->execute("SET @conNumb = " + conNumb);
    stmt->execute("SET @packetNumb = " + packetNumb);
    stmt->execute("SELECT Data from DataFrom1 WHERE ConNumber = @conNumb AND PacketNumber = @packetNumb");
    std::auto_ptr<sql::ResultSet> res;

    do {
        res.reset(stmt->getResultSet());
        while (res->next()) {
            m_textctrl->SetDefaultStyle(wxTextAttr(*wxBLUE));
            *m_textctrl << (wxString) (res->getString("Data"));
        }
    } while (stmt->getMoreResults());
    delete con;
}