#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //Initialize portname combo box
    on_refresh_available_arduino_clicked();
}

MainWindow::~MainWindow()
{
    delete ui;
}

/********** Message Browser Related **********/

void MainWindow::logger(const QString &message)
{
    ui->message_browser->append(message);
}

void MainWindow::on_clear_message_button_clicked()
{
    ui->message_browser->clear();
}

/********** Serial Port Communication Related **********/

void MainWindow::on_refresh_available_arduino_clicked()
{
    ui->portname_box->clear();
    for(const QSerialPortInfo &info: QSerialPortInfo::availablePorts()) //Find out available arduino boards.
        if(info.description().contains("Arduino")) //Add them in to combobox and display.
            ui->portname_box->addItem(info.portName()+": "+info.description());
}

void MainWindow::on_portname_box_activated(const QString &arg)
{
    if(arg.isEmpty())
        logger("Set Port Failed.");
    else
    {
        arduino.setPortName(arg.section(':',0,0));
        logger("Set Port " + arg);
    }
}

void MainWindow::on_baudrate_box_activated(const QString &arg)
{
    bool check = arduino.setBaudRate(arg.toInt());
    logger("Set Baud Rate: " + arg + (check?" - Successed.":" - Failed."));
}

void MainWindow::on_databits_box_activated(const QString &arg)
{
    bool check;
    switch (arg[0].toLatin1())
    {
        case '5':
        check = arduino.setDataBits(QSerialPort::Data5);
        break;
        case '6':
        check = arduino.setDataBits(QSerialPort::Data6);
        break;
        case '7':
        check = arduino.setDataBits(QSerialPort::Data7);
        break;
        case '8':
        check = arduino.setDataBits(QSerialPort::Data8);
        break;
    }
    logger("Set Data Bits: " + arg + (check?" - Successed.":" - Failed."));
}

void MainWindow::on_stopbits_box_activated(const QString &arg)
{
    bool check;
    switch (arg[0].toLatin1())
    {
        case '1':
        check = arduino.setStopBits(QSerialPort::OneStop);
        break;
        case '2':
        check = arduino.setStopBits(QSerialPort::TwoStop);
        break;
        case '3':
        check = arduino.setStopBits(QSerialPort::OneAndHalfStop);
        break;
    }
    logger("Set Stop Bits: " + arg.section(' ',2) + (check?" - Successed.":" - Failed."));
}

void MainWindow::on_parity_box_activated(const QString &arg)
{
    bool check;
    switch (arg[0].toLatin1())
    {
        case '0':
        check = arduino.setParity(QSerialPort::NoParity);
        break;
        case '2':
        check = arduino.setParity(QSerialPort::EvenParity);
        break;
        case '3':
        check = arduino.setParity(QSerialPort::OddParity);
        break;
        case '4':
        check = arduino.setParity(QSerialPort::SpaceParity);
        break;
        case '5':
        check = arduino.setParity(QSerialPort::MarkParity);
        break;
    }
    logger("Set Parity: " + arg.section(' ',2) + (check?" - Successed.":" - Failed."));
}

void MainWindow::on_port_switch_button_clicked(bool checked)
{
    if(checked)
    {
        //set port
        on_portname_box_activated(ui->portname_box->currentText());
        on_baudrate_box_activated(ui->baudrate_box->currentText());
        on_databits_box_activated(ui->databits_box->currentText());
        on_stopbits_box_activated(ui->stopbits_box->currentText());
        on_parity_box_activated(ui->parity_box->currentText());
        //open port
        if(!arduino.open(QIODevice::ReadWrite)) //check accessibility
        {//if inaccessible
            ui->port_switch_button->setChecked(false);
            logger("Port: Connection Failed.");
            return;
        }
        //connect message signal/slot
        QObject::connect(&arduino, &QSerialPort::readyRead,
                         this, &MainWindow::read_message);
        //refresh ui
        ui->portname_box->setEnabled(false);
        ui->port_switch_button->setText("Close Port");
        logger("Port: Arduino connected.");
    }
    else
    {
        //close port
        arduino.clear();
        arduino.close();
        //break message signal/slot
        QObject::disconnect(&arduino, &QSerialPort::readyRead,
                         this, &MainWindow::read_message);
        //refresh ui
        ui->portname_box->setEnabled(true);
        ui->port_switch_button->setText("Open Port");
        logger("Port: Arduino disconnected.");
    }
}

void MainWindow::read_message()
{
    if(arduino.canReadLine())
        logger("<<< "+arduino.readLine().trimmed());
}

void MainWindow::send_message(const QString &message)
{
    if(!message.isEmpty())
    {
        arduino.write(message.toLatin1());
        logger(">>> "+message);
    }
}

void MainWindow::on_command_line_edit_editingFinished()
{
    send_message(ui->command_line_edit->text());
}

/********** Automatic Executor Related **********/

void MainWindow::on_load_button_clicked()
{
    QString workflow_filename = QFileDialog::getOpenFileName(this,"Open File","","JSON (*.json)");
    if (workflow_filename.isEmpty()) return; //return if cancelled

    QFile loader(workflow_filename);
    if(!loader.open(QIODevice::ReadOnly))
    {//if the file hasn't been occupied
        logger("Loader: Could not open the file.");
        return;
    }

    QJsonParseError parse_error; //verify Json format
    QJsonDocument reader = QJsonDocument(QJsonDocument::fromJson(loader.readAll(),&parse_error));
    if(parse_error.error!=QJsonParseError::NoError)
    {
        logger("Loader: Parse error.");
        return;
    }

    //store workflow
    workflow = reader.object().value("Workflow").toArray();

    //parse and display outline
    logger("Workflow Description: " + reader.object().value("Description").toString()); //display meta data
    for(QJsonArray::iterator iterator = workflow.begin(); iterator != workflow.end(); iterator++) //display each operation
        //QJsonArray::iterator does not support range for
        display_operation(iterator);
}

void MainWindow::display_operation(QJsonArray::iterator iterator)
{
    logger("Operation: " + iterator->toObject().value("Operation").toString()+
           "\tTime: " + QString::number(iterator->toObject().value("Time").toDouble()));
}

int MainWindow::workflow_executor(QJsonArray::iterator iterator)
{
    QJsonObject operation = iterator->toObject();
    logger("Operation: " + operation.value("Operation").toString());
    for(const QJsonValue &command: operation.value("Commands").toArray())
        send_message(command.toString());
        //logger(command.toString());
    return (int)1000*(operation.value("Time").toDouble());
}

void MainWindow::on_start_button_clicked()
{
    if(workflow.empty()) return;
    if(workflow_iterator == workflow.begin())
    {   //The work after first manual click
        //connect function and timer
        connect(&workflow_executor_timer, &QTimer::timeout,
                this,&MainWindow::on_start_button_clicked);
        //refresh ui
        ui->start_button->setEnabled(false); //disable manual click
        ui->stop_button->setEnabled(true);
        ui->pause_button->setEnabled(true);
        ui->skip_work_button->setEnabled(true);
        ui->last_work_button->setEnabled(true);
        ui->next_work_button->setEnabled(true);
        logger("Executor Start.");
    }
    if(workflow_iterator != workflow.end())
    {   //Works executed after both manual and automatic click
        //execute work and set timer for next execution
        workflow_executor_timer.start(workflow_executor(workflow_iterator));
        //move iterator
        workflow_iterator++;
    }
    else
        //Terminator
        on_stop_button_clicked();
}

void MainWindow::on_stop_button_clicked()
{
    //Terminate timer and reset iterator
    workflow_executor_timer.stop();
    workflow_iterator = workflow.begin();
    //disconnect function and timer
    disconnect(&workflow_executor_timer, &QTimer::timeout,
               this,&MainWindow::on_start_button_clicked);
    //refresh ui
    ui->start_button->setEnabled(true);
    ui->stop_button->setEnabled(false);
    ui->pause_button->setEnabled(false);
    if(ui->pause_button->isChecked())
        ui->pause_button->setChecked(false);
    ui->skip_work_button->setEnabled(false);
    ui->last_work_button->setEnabled(false);
    ui->next_work_button->setEnabled(false);
    logger("Executor Stop.");
}

void MainWindow::on_pause_button_clicked(bool checked)
{
    if(checked)
    {
        //save remaining time
        workflow_executor_remaining_time = workflow_executor_timer.remainingTime();
        workflow_executor_timer.stop();
        logger("Remaining Time " +
               QString::number((double)workflow_executor_remaining_time/1000) + "s");
    }
    else
        //restart workflow with saved time
        workflow_executor_timer.start(workflow_executor_remaining_time);
}

void MainWindow::on_skip_work_button_clicked()
{
    workflow_executor_timer.setInterval(0); //for running workflow
    if(ui->pause_button->isChecked()) //for paused workflow
    {
        ui->pause_button->setChecked(false); //refresh pause button ui
        workflow_executor_remaining_time = 0; //clear remaining time
        on_pause_button_clicked(false); //resume workflow
    }
}

void MainWindow::on_last_work_button_clicked()
{
    if(workflow_iterator != workflow.begin())
        workflow_iterator--;
    logger("Next Operation:");
    display_operation(workflow_iterator);
}

void MainWindow::on_next_work_button_clicked()
{
    if(workflow_iterator != workflow.end())
        workflow_iterator++;
    if(workflow_iterator != workflow.end())
    {//will not display anything at the end
        logger("Next Operation:");
        display_operation(workflow_iterator);
    }
}
