#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLineEdit>
#include <QListWidget>
#include <iostream>
#include "twolistselection.h"

class MainWindowProgram : public QMainWindow
{
    Q_OBJECT
    public:
        MainWindowProgram(QWidget *parent = 0);
        ~MainWindowProgram();
    private:
        QPushButton * shutdown_button;
        QPushButton * load_button;
        QPushButton * move_forward_button;
        QPushButton * move_back_button;
        QListWidget * devices_list;
        QListWidget * shutdown_list;
        QLineEdit * server_address;

    private slots:
      void shutdown_handler();
      void load_handler();
      void move_forward_handler();
      void move_back_handler();
      void buttons_freezing_hanler(bool state);
};


#endif // MAINWINDOW_H
