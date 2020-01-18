#include <QListWidget>
#include "mainwindow.h"
#include "tcpconnection.h"
#include <QStatusBar>
#include <QPushButton>
#include <QLineEdit>
#include <QLine>
#include <QLabel>

MainWindowProgram::MainWindowProgram(QWidget *parent) : QMainWindow(parent){
    //GLOWNE OKNO APLIKACJI
    setWindowTitle("Remote Shutdown System");
    setMinimumSize(800, 500);
    int y_center=100;
    int x_center=300;

    int y_change_value = 50;
    int x_change_value = 220;
    int up = -y_change_value;
    int very_down = x_change_value+50;
    int very_left = -x_change_value;
    int very_right = x_change_value;

    QLabel * label_text = new QLabel(this);

    QStatusBar * status_bar = new QStatusBar(this);
    setStatusBar(status_bar);

    //POLE ADRESU SERWERA
    label_text = new QLabel(this);
    label_text -> setGeometry(QRect(x_center+20, y_center+up, 200, 20));
    label_text -> setText("Server address:");
    server_address=new QLineEdit(this);
    server_address -> setGeometry(QRect(x_center+20, y_center, 150, 20));
    server_address->insert("student@lab-net-");

    //PANEL DOSTEPNYCH URZADZEN
    label_text = new QLabel(this);
    label_text -> setGeometry(QRect(x_center+very_right, y_center+up, 200, 20));
    label_text -> setText("Devices online:");

    devices_list=new QListWidget(this);
    devices_list->setSelectionMode(QAbstractItemView::MultiSelection);
    devices_list -> setGeometry(QRect(x_center+very_right, y_center, 200, 250));

    move_forward_button=new QPushButton(this);
    move_forward_button -> setGeometry(QRect(QPoint(x_center+120, y_center+very_down-180), QSize(50, 50)));
    move_forward_button -> setText("<=");
    connect(move_forward_button, SIGNAL (released()), this, SLOT(move_forward_handler()));

    load_button=new QPushButton(this);
    load_button -> setGeometry(QRect(QPoint(x_center+very_right, y_center+very_down), QSize(200, 50)));
    load_button -> setText("Refresh");
    connect(load_button, SIGNAL (released()), this, SLOT(load_handler()));

    //PANEL URZADZEN WYBRANYCH DO ZAMKNIECIA
    label_text = new QLabel(this);
    label_text -> setGeometry(QRect(x_center+very_left, y_center+up, 200, 20));
    label_text -> setText("Devices to shutdown:");

    shutdown_list=new QListWidget(this);
    shutdown_list->setSelectionMode(QAbstractItemView::MultiSelection);
    shutdown_list -> setGeometry(QRect(x_center+very_left, y_center, 200, 250));

    move_back_button= new QPushButton(this);
    move_back_button -> setGeometry(QRect(QPoint(x_center+20, y_center+very_down-180), QSize(50, 50)));
    move_back_button -> setText("=>");
    connect(move_back_button, SIGNAL (released()), this, SLOT (move_back_handler()));

    shutdown_button= new QPushButton(this);
    shutdown_button -> setGeometry(QRect(QPoint(x_center+very_left, y_center+very_down), QSize(200, 50)));
    shutdown_button -> setText("Shutdown");
    connect(shutdown_button, SIGNAL (released()), this, SLOT (shutdown_handler()));

    //WCZYTANIE LISTY URZADZEN
    load_handler();

}

//FUNKCJA DO ZABLOKOWANIA PRZYCISKOW NA CZAS WYKONYWANIA POLECENIA
void MainWindowProgram::buttons_freezing_hanler(bool state){
    shutdown_button->setDisabled(state);
    load_button->setDisabled(state);
    move_forward_button->setDisabled(state);
    move_back_button->setDisabled(state);
    strcpy(receiving_data,"");
    strcpy(sending_data,"");
}

//FUNKCJA OBSLUGUJACA WCISNIECIE PRZYCISKU SHUTDOWN
void MainWindowProgram::shutdown_handler(){
    buttons_freezing_hanler(true);
    statusBar()->showMessage("Shutdown in progress...");

    string choosen_devices;
    QList<QListWidgetItem *> selected_items=shutdown_list->selectedItems();
    for (int i=0;i<selected_items.length();i++){
       choosen_devices = choosen_devices+ (selected_items[i]->text().toLocal8Bit().data())+",";
       int place=shutdown_list->row(selected_items[i]);
       shutdown_list->takeItem(place);
    }

    std::cout<<"choosen devices: "<<choosen_devices<<'\n';
    char char_devices[choosen_devices.size()+1];
    strcpy(char_devices, choosen_devices.c_str());

    char* server = server_address->text().toLocal8Bit().data();
    handle_command(1, char_devices, server);

    buttons_freezing_hanler(false);
}

//FUNKCJA OBSLUGUJACA PRZENOSZENIE Z LISTY DOSTEPNYCH DO LISTY WYBRANYCH DO ZAMKNIECIA
void MainWindowProgram::move_forward_handler(){
    QList<QListWidgetItem *> selected_items=devices_list->selectedItems();
    for (int i=0;i<selected_items.length();i++){
        QList<QListWidgetItem *> already_exists=shutdown_list->findItems(selected_items[i]->text(), Qt::MatchExactly);
        if (already_exists.length()==0) shutdown_list->addItem(selected_items[i]->text());
    }
  }

//FUNKCJA OBSLUGUJACA USUNIECIE Z LISTY WYBRANYCH DO ZAMKNIECIA
void MainWindowProgram::move_back_handler(){
    QList<QListWidgetItem *> selected_items=shutdown_list->selectedItems();
    for (int i=0;i<selected_items.length();i++){
        int place=shutdown_list->row(selected_items[i]);
        shutdown_list->takeItem(place);
    }

}

//FUNKCJA OBSLUGUJACA WCISNIECIE PRZYCISKU REFRESH
void MainWindowProgram::load_handler(){
    buttons_freezing_hanler(true);
    statusBar()->showMessage("Loading...");

    devices_list->clear();
    char* empty_list = {};
    char* server = server_address->text().toLocal8Bit().data();

    handle_command(0, empty_list, server);
    string device_number;
    string device_address;
    for (int i=0; i<int(strlen(receiving_data));i++){
        char separate=',';
        if (receiving_data[i]!=separate) device_number+=receiving_data[i];
        else {
            device_address = device_number;
            //statusBar()->showMessage(QString::fromStdString(device_address));
            cout <<device_address<<"\n";
            QListWidgetItem *device_item=new QListWidgetItem();
            device_item->setText(QString::fromStdString(device_address));
            QString device_item_qstring=QString::fromStdString(device_address);

            QList<QListWidgetItem *> already_exists=devices_list->findItems(device_item_qstring, Qt::MatchExactly);
            if (already_exists.length()==0) devices_list->addItem(device_item);

            device_number="";
        }
    }
    statusBar()->hide();
    buttons_freezing_hanler(false);
}

MainWindowProgram::~MainWindowProgram()
{

}

