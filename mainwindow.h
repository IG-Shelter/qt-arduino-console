#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
//serial port
#include <QSerialPort>
#include <QSerialPortInfo>
//json pharse
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
//time and timer
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    /********** Message Browser Related **********/
    void on_clear_message_button_clicked();
    /********** Serial Port Communication Related **********/
    void on_refresh_available_arduino_clicked();
    void on_portname_box_activated(const QString &arg);
    void on_baudrate_box_activated(const QString &arg);
    void on_databits_box_activated(const QString &arg);
    void on_stopbits_box_activated(const QString &arg);
    void on_parity_box_activated(const QString &arg);
    void on_port_switch_button_clicked(bool checked);
    void read_message();
    void send_message(const QString &message);
    void on_command_line_edit_editingFinished();
    /********** Automatic Executor Related **********/
    void on_load_button_clicked();
    void display_operation(QJsonArray::iterator iterator);
    int workflow_executor(QJsonArray::iterator iterator);
    void on_start_button_clicked();
    void on_stop_button_clicked();
    void on_pause_button_clicked(bool checked);
    void on_skip_work_button_clicked();
    void on_last_work_button_clicked();
    void on_next_work_button_clicked();

private:
    /********** Message Browser Related **********/
    inline void logger(const QString& message); //inline slot will cause LNK2019

    Ui::MainWindow *ui;
    /********** Serial Port Communication Related **********/
    QSerialPort arduino;
    /********** Automatic Executor Related **********/
    QJsonArray workflow; //load by - void on_load_button_clicked();
    QJsonArray::iterator workflow_iterator = workflow.begin();
    QTimer workflow_executor_timer;
    int workflow_executor_remaining_time; //store the remaining time after on_pause_button_clicked
};
#endif // MAINWINDOW_H
