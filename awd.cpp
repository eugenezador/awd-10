#include "awd.h"
#include "ui_awd.h"

awd::awd(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::awd)
{
    ui->setupUi(this);

    serial = new QSerialPort(this);

//чтение доступных портов
//
    foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts())
       {
           ui->portName->addItem(serialPortInfo.portName());
       }
    connect(ui->portName, &QComboBox::currentTextChanged, this, &awd::serial_port_properties);

// параметры порта
    serial_port_properties(this->ui->portName->currentText());

// Параметры
    set_param_26_items();

// Режим
    set_mode_items();
    set_mode_connections();
// Статус
    status_no_edit();

// График
    plot_settings();

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &awd::slot_for_new_point);// соединение для создания новой точки скорости
    connect(timer, &QTimer::timeout, this, &awd::slot_for_status); // для чтения статуса
    timer->start(500);
}

awd::~awd()
{
    delete timer;
    serial->close();
    delete serial;
    delete ui;
}

void awd::serial_port_properties(const QString &value)
{
    serial->setPortName(value);// Имя порта
    serial->open(QIODevice::ReadWrite);
    serial->setBaudRate(QSerialPort::Baud9600);// Скорость обмена
    serial->setDataBits(QSerialPort::Data8);// Биты данных
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);
}

unsigned char awd::checkSumm(const unsigned char array[8])
{
    unsigned char summ = 0;
    for(int i = 0; i < 8; i++){
        summ -= array[i];
    }
    return summ;
}

void awd::command_formation(const QString &value,const int &param_num)
{
    command[1] = 0x78;
    command[2] = param_num;
    command[3] = 0x00;
    command[4] = (value.toInt() >> 8) & 0xFF;//setData(value.toInt(),1);
    command[5] = value.toInt() & 0xFF;//setData(value.toInt(),0);
    command[6] = 0x00;
    command[7] = checkSumm(command);

    writeData((QByteArray::fromRawData((const char*)command, sizeof (command))));
}

void awd::command_formation(int param_num)
{
    command[1] = 0x87;
    command[2] = param_num;
    command[3] = 0x00;
    command[4] = 0x00;
    command[5] = 0x00;
    command[6] = 0x00;
    command[7] = checkSumm(command);

    writeData((QByteArray::fromRawData((const char*)command, sizeof (command))));
}


int awd::setData(int value, bool flag)
{
    if (flag)
    {
        return (value >> 8) & 0xFF;
    } else
    {
        return value & 0xFF;
    }
}
/*
int awd::checkSumm(int *array)
{
    int summ = 0;
    for(int i = 0; i < 8; i++){
        summ -= array[i];
    }
    return summ;
}
*/
QByteArray awd::int_to_QByteArray(int *array, int size)
{
    QByteArray ba;
    ba.resize(size);
    for (int i = 0; i <size ; i++ )
    {
        ba[i] = array[i];
    }
    return ba;
}

int awd::setReadDataValue(QByteArray data)
{
    std::bitset<8> data1 = data.at(4);
    std::bitset<8> data0 = data.at(5);
    std::bitset<16> speed_value;
    int k = 0;

    for(int i = 0; i < 8; i++ )
    {
        speed_value[k] = data0[i];
        k++;
    }
    for(int i = 0; i < 8; i++)
    {
        speed_value[k] = data1[i];
        k++;

    }

    return (speed_value).to_ulong();
}

void awd::set_param_26_items()
{
    ui->param_new_value_26->addItem("16");
    ui->param_new_value_26->addItem("1");
    ui->param_new_value_26->addItem("2");
    ui->param_new_value_26->addItem("4");
    ui->param_new_value_26->addItem("8");
}

void awd::set_mode_items()
{
    ui->comboBox_mode->addItem("Стабилизация скор.по ЭДС");
    ui->comboBox_mode->addItem("Стабилизация скор.по энкодеру");
    ui->comboBox_mode->addItem("Слежение за внешним сигналом");
    ui->comboBox_mode->addItem("Ограничение момента");
}

void awd::set_mode_connections()
{
    connect(ui->SkipLim, &QCheckBox::stateChanged, this, &awd::change_state);
    connect(ui->LimDrop, &QCheckBox::stateChanged, this, &awd::change_state);
    connect(ui->StopDrop, &QCheckBox::stateChanged, this, &awd::change_state);
    connect(ui->IntrfEN, &QCheckBox::stateChanged, this, &awd::change_state);
    connect(ui->IntrfVal, &QCheckBox::stateChanged, this, &awd::change_state);
    connect(ui->IntrfDir, &QCheckBox::stateChanged, this, &awd::change_state);
    connect(ui->SrcParam, &QCheckBox::stateChanged, this, &awd::change_state);
}

void awd::status_no_edit()
{
    ui->StLimFrw->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->StLimFrw->setFocusPolicy(Qt::NoFocus);

    ui->StLimRev->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->StLimRev->setFocusPolicy(Qt::NoFocus);

    ui->StinFrw->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->StinFrw->setFocusPolicy(Qt::NoFocus);

    ui->StinRev->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->StinRev->setFocusPolicy(Qt::NoFocus);

    ui->StMotAct->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->StMotAct->setFocusPolicy(Qt::NoFocus);

    ui->StDirFrwRev->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->StDirFrwRev->setFocusPolicy(Qt::NoFocus);

    ui->StMaxPWM->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->StMaxPWM->setFocusPolicy(Qt::NoFocus);

    ui->StOverCur->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->StOverCur->setFocusPolicy(Qt::NoFocus);
}

void awd::status_read(const QByteArray &data)
{
    mode_status = data.at(6);

    if(mode_status[0] == 1) ui->StLimFrw->setChecked(1);
    else ui->StLimFrw->setChecked(0);

    if(mode_status[1] == 1) ui->StLimRev->setChecked(1);
    else ui->StLimRev->setChecked(0);

    if(mode_status[2] == 1) ui->StinFrw->setChecked(1);
    else ui->StinFrw->setChecked(0);

    if(mode_status[3] == 1) ui->StinRev->setChecked(1);
    else ui->StinRev->setChecked(0);

    if(mode_status[4] == 1) ui->StMotAct->setChecked(1);
    else ui->StMotAct->setChecked(0);

    if(mode_status[5] == 1) ui->StDirFrwRev->setChecked(1);
    else ui->StDirFrwRev->setChecked(0);

    if(mode_status[6] == 1) ui->StMaxPWM->setChecked(1);
    else ui->StMaxPWM->setChecked(0);

    if(mode_status[7] == 1) ui->StOverCur->setChecked(1);
    else ui->StOverCur->setChecked(0);

    mode_status.reset();
}

void awd::slot_for_status()
{
    if(ui->status_checkBox->isChecked()){
        status_read((QByteArray::fromRawData((const char*)command, sizeof (command))));
        qDebug() << "status read";
    }
     else qDebug() << "no status";
}

void awd::real_plot(const int &value)
{
    static QTime time(QTime::currentTime());
    //static QElapsedTimer eltimer ;
    //eltimer.start();
    //double key = eltimer.elapsed()/1000.0;
    double key = time.elapsed()/1000.0;
        static double lastPointKey = 0;
        if(key - lastPointKey > 0.002)
        {
            //addPoint(key,value);
            qv_x.append(key);
            qv_y.append(value);
            qDebug() << qv_x << ":" << qv_y;
            //plot();
            ui->plot->graph(0)->setData(qv_x, qv_y);
            ui->plot->replot();
            ui->plot->update();
            lastPointKey = key;
        }

        /* make key axis range scroll right with the data at a constant range of 8. */
        //ui->plot->graph(0)->rescaleValueAxis();
        //ui->plot->xAxis->setRange(key, 8, Qt::AlignRight);
        ui->plot->replot();
}

void awd::slot_for_new_point()
{
    if(ui->speed_checkBox->isChecked())
    {
        command[1] = 0x3c;
        command[2] = 0x05;
        command[3] = 0x00;
        command[4] = 0x00;
        command[5] = 0x00;
        command[6] = 0x00;
        command[7] = 0xba;

        writeData((QByteArray::fromRawData((const char*)command, sizeof (command))));
    }
}

void awd::slot_for_A_vx_1()
{
    if(ui->A_vx_1_checkBox->isChecked())
    {
        command[1] = 0x3c;
        command[2] = 0x00;
        command[3] = 0x00;
        command[4] = 0x00;
        command[5] = 0x00;
        command[6] = 0x00;
        command[7] = 0xba;

        writeData((QByteArray::fromRawData((const char*)command, sizeof (command))));
    }
}

void awd::slot_for_A_vx_2()
{
    if(ui->A_vx_2_checkBox->isChecked())
    {
        command[1] = 0x3c;
        command[2] = 0x01;
        command[3] = 0x00;
        command[4] = 0x00;
        command[5] = 0x00;
        command[6] = 0x00;
        command[7] = 0xba;

        writeData((QByteArray::fromRawData((const char*)command, sizeof (command))));
    }
}

void awd::on_write_all_by_default_clicked()
{
/*    for(int i = 0; i < 38; i++){
        command_formation(QString::number(by_default[i]),i);
        param_current_value[i]->setText(QString::number(by_default[i]));
    }
*/
    command_formation(QString::number(by_default[0]),0);
    ui->param_current_value_0->setText(QString::number(by_default[0]));

    command_formation(QString::number(by_default[1]),1);
    ui->param_current_value_1->setText(QString::number(by_default[1]));

    command_formation(QString::number(by_default[2]),2);
    ui->param_current_value_2->setText(QString::number(by_default[2]));

    command_formation(QString::number(by_default[3]),3);
    ui->param_current_value_3->setText(QString::number(by_default[3]));

    command_formation(QString::number(by_default[4]),4);
    ui->param_current_value_4->setText(QString::number(by_default[4]));

    command_formation(QString::number(by_default[5]),5);
    ui->param_current_value_5->setText(QString::number(by_default[5]));

    command_formation(QString::number(by_default[6]),6);
    ui->param_current_value_6->setText(QString::number(by_default[6]));

    command_formation(QString::number(by_default[7]),7);
    ui->param_current_value_7->setText(QString::number(by_default[7]));

    command_formation(QString::number(by_default[8]),8);
    ui->param_current_value_8->setText(QString::number(by_default[8]));

    command_formation(QString::number(by_default[9]),9);
    ui->param_current_value_9->setText(QString::number(by_default[9]));

    command_formation(QString::number(by_default[10]),10);
    ui->param_current_value_10->setText(QString::number(by_default[10]));

    command_formation(QString::number(by_default[11]),11);
    ui->param_current_value_11->setText(QString::number(by_default[11]));

    command_formation(QString::number(by_default[12]),12);
    ui->param_current_value_12->setText(QString::number(by_default[12]));

    command_formation(QString::number(by_default[13]),13);
    ui->param_current_value_13->setText(QString::number(by_default[13]));

    command_formation(QString::number(by_default[14]),14);
    ui->param_current_value_14->setText(QString::number(by_default[14]));

    command_formation(QString::number(by_default[15]),15);
    ui->param_current_value_15->setText(QString::number(by_default[15]));

    command_formation(QString::number(by_default[16]),16);
    ui->param_current_value_16->setText(QString::number(by_default[16]));

    command_formation(QString::number(by_default[17]),17);
    ui->param_current_value_17->setText(QString::number(by_default[17]));

    command_formation(QString::number(by_default[18]),18);
    ui->param_current_value_18->setText(QString::number(by_default[18]));

    command_formation(QString::number(by_default[19]),19);
    ui->param_current_value_19->setText(QString::number(by_default[19]));

    command_formation(QString::number(by_default[20]),20);
    ui->param_current_value_20->setText(QString::number(by_default[20]));

    command_formation(QString::number(by_default[21]),21);
    ui->param_current_value_21->setText(QString::number(by_default[21]));

    command_formation(QString::number(by_default[22]),22);
    ui->param_current_value_22->setText(QString::number(by_default[22]));

    command_formation(QString::number(by_default[23]),23);
    ui->param_current_value_23->setText(QString::number(by_default[23]));

    command_formation(QString::number(by_default[24]),24);
    ui->param_current_value_24->setText(QString::number(by_default[24]));

    command_formation(QString::number(by_default[25]),25);
    ui->param_current_value_25->setText(QString::number(by_default[25]));

    command_formation(QString::number(by_default[26]),26);
    ui->param_current_value_26->setText(QString::number(by_default[26]));

    command_formation(QString::number(by_default[27]),27);
    ui->param_current_value_27->setText(QString::number(by_default[27]));

    command_formation(QString::number(by_default[28]),28);
    ui->param_current_value_28->setText(QString::number(by_default[28]));

    command_formation(QString::number(by_default[29]),29);
    ui->param_current_value_29->setText(QString::number(by_default[29]));

    command_formation(QString::number(by_default[30]),30);
    ui->param_current_value_30->setText(QString::number(by_default[30]));

    command_formation(QString::number(by_default[31]),31);
    ui->param_current_value_31->setText(QString::number(by_default[31]));

    command_formation(QString::number(by_default[32]),32);
    ui->param_current_value_32->setText(QString::number(by_default[32]));

    command_formation(QString::number(by_default[33]),33);
    ui->param_current_value_33->setText(QString::number(by_default[33]));

    command_formation(QString::number(by_default[34]),34);
    ui->param_current_value_34->setText(QString::number(by_default[34]));

    command_formation(QString::number(by_default[35]),35);
    ui->param_current_value_35->setText(QString::number(by_default[35]));

    command_formation(QString::number(by_default[36]),36);
    ui->param_current_value_36->setText(QString::number(by_default[36]));

}

void awd::on_read_all_clicked()
{
    for(int i = 0; i <= 36; i++){
        command_formation(i);
    }
}

void awd::change_state()
{
    //SkipLim
    if(ui->SkipLim->isChecked()) ui->SkipLim->setText("Не использовать сигналы от концевых выключателей");
    else ui->SkipLim->setText("Использовать сигналы от концевых выключателей");

    //LimDrop
    if(ui->LimDrop->isChecked()) ui->LimDrop->setText("Не удерживать двигатель при срабатывании концевого выключателя");
    else ui->LimDrop->setText("Удерживать двигатель при срабатывании концевого выключателя");

    //StopDrop
    if(ui->StopDrop->isChecked()) ui->StopDrop->setText("Не Удерживать двигатель при остановке вращения");
    else ui->StopDrop->setText("Удерживать двигатель при остановке вращения");

    //IntrfEN
    if(ui->IntrfEN->isChecked()) ui->IntrfEN->setText("Управлять разрешением вращения через интерфейс RS485");
    else ui->IntrfEN->setText("Управлять разрешением вращения с помощью внешнего сигнала");

    //IntrfVal
    if(ui->IntrfVal->isChecked()) ui->IntrfVal->setText("Управлять скоростью вращения через интерфейс RS485");
    else ui->IntrfVal->setText("Управлять сокростью с помощью внешних сигналов");

    //IntrfDir
    if(ui->IntrfDir->isChecked()) ui->IntrfDir->setText("Управлять направлением через интерфейс RS485");
    else ui->IntrfDir->setText("Управлять направлением с помощью внешних сигналов");

    //SrcParam
    if(ui->SrcParam->isChecked()) ui->SrcParam->setText("Использовать аналоговый выход №2");
    else ui->SrcParam->setText("Использовать параметр №31");

}

void awd::on_read_button_regime_clicked()
{
    command[1] = 0x87;
    command[2] = 0x1c;
    command[3] = 0x00;
    command[4] = 0x00;
    command[5] = 0x00;
    command[6] = 0x00;
    command[7] = checkSumm(command);

    writeData((QByteArray::fromRawData((const char*)command, sizeof (command))));
}

void awd::on_write_button_regime_clicked()
{
    command[1] = 0x78;
    command[2] = 0x1c;
    command[3] = 0x00;

    if(ui->SkipLim->isChecked()) mode_data[14] = 1;
    if(ui->LimDrop->isChecked()) mode_data[13] = 1;
    if(ui->StopDrop->isChecked()) mode_data[12] = 1;
    if(ui->IntrfEN->isChecked()) mode_data[11] = 1;
    if(ui->IntrfVal->isChecked()) mode_data[10] = 1;
    if(ui->IntrfDir->isChecked()) mode_data[9] = 1;
    if(ui->SrcParam->isChecked()) mode_data[8] = 1;

    if(ui->comboBox_mode->currentText() == "Стабилизация скор.по ЭДС")
    {
        mode_data[3] = 0;
        mode_data[2] = 0;
        mode_data[1] = 0;
        mode_data[0] = 0;
    }

    if(ui->comboBox_mode->currentText() == "Стабилизация скор.по энкодеру")
    {
        mode_data[3] = 0;
        mode_data[2] = 0;
        mode_data[1] = 0;
        mode_data[0] = 1;
    }

    if(ui->comboBox_mode->currentText() == "Слежение за внешним сигналом")
    {
        mode_data[3] = 0;
        mode_data[2] = 0;
        mode_data[1] = 1;
        mode_data[0] = 0;
    }

    if(ui->comboBox_mode->currentText() == "Ограничение момента")
    {
        mode_data[3] = 0;
        mode_data[2] = 0;
        mode_data[1] = 1;
        mode_data[0] = 1;
    }

    command[4] = setData(mode_data.to_ulong(),1);//
    command[5] = setData(mode_data.to_ulong(),0);//
    command[6] = 0x00;
    command[7] = checkSumm(command);

    writeData((QByteArray::fromRawData((const char*)command, sizeof (command))));
    //status_read(int_to_QByteArray(command,8));
    ///////////////
    mode_data.reset();
}

void awd::on_stop_mode_button_clicked()
{
    command[1] = 0x4b;
    command[2] = 0x0a;
    command[3] = 0x00;
    command[4] = 0x00;
    command[5] = 0x00;
    command[6] = 0x00;
    command[7] = checkSumm(command);

    writeData((QByteArray::fromRawData((const char*)command, sizeof (command))));
}

void awd::on_start_tracking_clicked()
{
    command[1] = 0x4b;
    command[2] = 0x0b;
    command[3] = 0x00;
    command[4] = 0x00;
    command[5] = 0x00;
    command[6] = 0x00;
    command[7] = checkSumm(command);

    writeData((QByteArray::fromRawData((const char*)command, sizeof (command))));
}

void awd::on_Kp_valueChanged(int value)
{
    command[1] = 0x78;
    command[2] = 0x0F;
    command[3] = 0x00;
    command[4] = (value >> 8) & 0xFF;
    command[5] = value & 0xFF;
    command[6] = 0x00;
    command[7] = checkSumm(command); //вычисление контрольной суммы

    writeData((QByteArray::fromRawData((const char*)command, sizeof (command))));
}

void awd::on_Ki_valueChanged(int value)
{
    command[1] = 0x78;
    command[2] = 0x10;
    command[3] = 0x00;
    command[4] = (value >> 8) & 0xFF;
    command[5] = value & 0xFF;
    command[6] = 0x00;
    command[7] = checkSumm(command);//вычисление контрольной суммы

    writeData((QByteArray::fromRawData((const char*)command, sizeof (command))));
}

void awd::on_Kd_valueChanged(int value)
{
    command[1] = 0x78;
    command[2] = 0x11;
    command[3] = 0x00;
    command[4] = (value >> 8) & 0xFF;
    command[5] = value & 0xFF;
    command[6] = 0x00;
    command[7] = checkSumm(command);//вычисление контрольной суммы

    writeData((QByteArray::fromRawData((const char*)command, sizeof (command))));
}

void awd::on_speed_slider_valueChanged(int value)
{
    command[1] = 0x4b;
    command[2] = 0x08;
    command[3] = 0x00;
    command[4] = (value >> 8) & 0xFF;
    command[5] = value & 0xFF;
    command[6] = 0x00;
    command[7] = checkSumm(command);//вычисление контрольной суммы

    writeData((QByteArray::fromRawData((const char*)command, sizeof (command))));
}

void awd::on_stop_button_clicked()
{
    command[1] = 0x4b;
    command[2] = 0x08;
    command[3] = 0x00;
    command[4] = 0x00;
    command[5] = 0x00;
    command[6] = 0x00;
    command[7] = checkSumm(command);//вычисление контрольной суммы

    ui->speed_spinBox->setValue(0);

    writeData((QByteArray::fromRawData((const char*)command, sizeof (command))));
}

void awd::plot_settings()
{
    ui->plot->setInteraction(QCP::iRangeDrag, true);//включаем взаимодействие удаления/приближения графика
    ui->plot->setInteraction(QCP::iRangeZoom, true);//включаем взвимодействие перетаскивания графика
    ui->plot->addGraph();
    ui->plot->graph(0)->setScatterStyle(QCPScatterStyle::ssCircle);
    ui->plot->graph(0)->setLineStyle(QCPGraph::lsNone);

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s");
    ui->plot->xAxis->setTicker(timeTicker);
    ui->plot->axisRect()->setupFullAxesBox();
}

void awd::on_btn_clear_clicked()
{
    //clearData
    qv_x.clear();
    qv_y.clear();
    // plot
    ui->plot->graph(0)->setData(qv_x, qv_y);
    ui->plot->replot();
    ui->plot->update();
}

void awd::writeData(const QByteArray &data)
{
    if(serial->isOpen() == true){
        serial->write(data);
        serial->waitForBytesWritten();
        if (serial->isWritable())
            qDebug() << "write: " << data;
        ui->write_label->setText(data.toHex( ' ' ));

        readData();
    }
    else qDebug() << "not open";
}

char16_t awd::setReadDatavalue_2(const QByteArray &data)
{
    return  (( ( data[4] << 8  ) | ( data[5] & 0xff ) ));
}

void awd::readData()
{
    QByteArray data;
    if(serial->isReadable())
    {
       while (serial->waitForReadyRead(reply_time))
       {
           // результат чтения накапливается в переменную data
           data.append(serial->readAll());
       }
       qDebug() << "read: " << data;


// Скорость
          if(data[1] == (char)0x3c && data[2] == (char)0x05 )
          {
              real_plot(setReadDataValue(data));
              qDebug() << setReadDataValue(data);
          }
// А.вх 1, 2
          if(data[1] == (char)0x3c && data[2] == (char)0x00)
          {
              //real_plot(setReadDataValue(data));
              //qDebug() << setReadDataValue(data);
          }
          if(data[1] == (char)0x3c && data[2] == (char)0x01)
          {
              //real_plot(setReadDataValue(data));
              //qDebug() << setReadDataValue(data);
          }

// Параметры
       if(data[1] == (char)0x87 && data[2] == (char)0x00)
       {
           ui->param_current_value_0->setText(QString::number(setReadDataValue(data)));
           current_param_value[0] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x01)
       {
           ui->param_current_value_1->setText(QString::number(setReadDataValue(data)));
           current_param_value[1] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x02)
       {
           ui->param_current_value_2->setText(QString::number(setReadDataValue(data)));
           current_param_value[2] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x03)
       {
           ui->param_current_value_3->setText(QString::number(setReadDataValue(data)));
           current_param_value[3] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x04)
       {
           ui->param_current_value_4->setText(QString::number(setReadDataValue(data)));
           current_param_value[4] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x05)
       {
           ui->param_current_value_5->setText(QString::number(setReadDataValue(data)));
           current_param_value[5] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x06)
       {
           ui->param_current_value_6->setText(QString::number(setReadDataValue(data)));
           current_param_value[6] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x07)
       {
           ui->param_current_value_7->setText(QString::number(setReadDataValue(data)));
           current_param_value[7] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x08)
       {
           ui->param_current_value_8->setText(QString::number(setReadDataValue(data)));
           current_param_value[8] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x09)
       {
           ui->param_current_value_9->setText(QString::number(setReadDataValue(data)));
           current_param_value[9] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x0a)
       {
           ui->param_current_value_10->setText(QString::number(setReadDataValue(data)));
           current_param_value[10] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x0b)
       {
           ui->param_current_value_11->setText(QString::number(setReadDataValue(data)));
           current_param_value[11] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x0c)
       {
           ui->param_current_value_12->setText(QString::number(setReadDataValue(data)));
           current_param_value[12] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x0d)
       {
           ui->param_current_value_13->setText(QString::number(setReadDataValue(data)));
           current_param_value[13] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x0e)
       {
           ui->param_current_value_14->setText(QString::number(setReadDataValue(data)));
           current_param_value[14] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x0f)
       {
           ui->param_current_value_15->setText(QString::number(setReadDataValue(data)));
           current_param_value[15] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x10)
       {
           ui->param_current_value_16->setText(QString::number(setReadDataValue(data)));
           current_param_value[16] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x11)
       {
           ui->param_current_value_17->setText(QString::number(setReadDataValue(data)));
           current_param_value[17] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x12)
       {
           ui->param_current_value_18->setText(QString::number(setReadDataValue(data)));
           current_param_value[18] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x13)
       {
           ui->param_current_value_19->setText(QString::number(setReadDataValue(data)));
           current_param_value[19] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x14)
       {
           ui->param_current_value_20->setText(QString::number(setReadDataValue(data)));
           current_param_value[20] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x15)
       {
           ui->param_current_value_21->setText(QString::number(setReadDataValue(data)));
           current_param_value[21] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x16)
       {
           ui->param_current_value_22->setText(QString::number(setReadDataValue(data)));
           current_param_value[22] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x17)
       {
           ui->param_current_value_23->setText(QString::number(setReadDataValue(data)));
           current_param_value[23] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x18)
       {
           ui->param_current_value_24->setText(QString::number(setReadDataValue(data)));
           current_param_value[24] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x19)
       {
           ui->param_current_value_25->setText(QString::number(setReadDataValue(data)));
           current_param_value[25] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x1a)
       {
           ui->param_current_value_26->setText(QString::number(setReadDataValue(data)));
           current_param_value[26] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x1b)
       {
           ui->param_current_value_27->setText(QString::number(setReadDataValue(data)));
           current_param_value[27] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x1c)
       {
           ui->param_current_value_28->setText(QString::number(setReadDataValue(data)));
           current_param_value[28] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x1d)
       {
           ui->param_current_value_29->setText(QString::number(setReadDataValue(data)));
           current_param_value[29] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x1e)
       {
           ui->param_current_value_30->setText(QString::number(setReadDataValue(data)));
           current_param_value[30] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x1f)
       {
           ui->param_current_value_31->setText(QString::number(setReadDataValue(data)));
           current_param_value[31] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x20)
       {
           ui->param_current_value_30->setText(QString::number(setReadDataValue(data)));
           current_param_value[32] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x21)
       {
           ui->param_current_value_31->setText(QString::number(setReadDataValue(data)));
           current_param_value[33] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x22)
       {
           ui->param_current_value_30->setText(QString::number(setReadDataValue(data)));
           current_param_value[34] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x23)
       {
           ui->param_current_value_31->setText(QString::number(setReadDataValue(data)));
           current_param_value[35] = setReadDataValue(data);
       }if(data[1] == (char)0x87 && data[2] == (char)0x24)
       {
           ui->param_current_value_30->setText(QString::number(setReadDataValue(data)));
           current_param_value[36] = setReadDataValue(data);
       }


       ui->read_label->setText(data.toHex( ' ' ));
    } else qDebug() << "no answer";
}

void awd::on_param_new_value_0_editingFinished()
{
    command_formation(ui->param_new_value_0->text(),0);
    ui->param_current_value_0->setText(ui->param_new_value_0->text());
    current_param_value[0] = (ui->param_new_value_0->text()).toInt();
}

void awd::on_param_new_value_1_editingFinished()
{
    command_formation(ui->param_new_value_1->text(),1);
    ui->param_current_value_1->setText(ui->param_new_value_1->text());
    current_param_value[1] = (ui->param_new_value_1->text()).toInt();
}

void awd::on_param_new_value_2_editingFinished()
{
    command_formation(ui->param_new_value_2->text(),2);
    ui->param_current_value_2->setText(ui->param_new_value_2->text());
    current_param_value[2] = (ui->param_new_value_2->text()).toInt();
}

void awd::on_param_new_value_3_editingFinished()
{
    command_formation(ui->param_new_value_3->text(),3);
    ui->param_current_value_3->setText(ui->param_new_value_3->text());
    current_param_value[3] = (ui->param_new_value_3->text()).toInt();
}

void awd::on_param_new_value_4_editingFinished()
{
    command_formation(ui->param_new_value_4->text(),4);
    ui->param_current_value_4->setText(ui->param_new_value_4->text());
    current_param_value[4] = (ui->param_new_value_4->text()).toInt();
}

void awd::on_param_new_value_5_editingFinished()
{
    command_formation(ui->param_new_value_5->text(),5);
    ui->param_current_value_5->setText(ui->param_new_value_5->text());
    current_param_value[5] = (ui->param_new_value_5->text()).toInt();
}

void awd::on_param_new_value_6_editingFinished()
{
    command_formation(ui->param_new_value_6->text(),6);
    ui->param_current_value_6->setText(ui->param_new_value_6->text());
    current_param_value[6] = (ui->param_new_value_6->text()).toInt();
}

void awd::on_param_new_value_7_editingFinished()
{
    command_formation(ui->param_new_value_7->text(),7);
    ui->param_current_value_7->setText(ui->param_new_value_7->text());
    current_param_value[7] = (ui->param_new_value_7->text()).toInt();
}

void awd::on_param_new_value_8_editingFinished()
{
    command_formation(ui->param_new_value_8->text(),8);
    ui->param_current_value_8->setText(ui->param_new_value_8->text());
    current_param_value[8] = (ui->param_new_value_8->text()).toInt();
}

void awd::on_param_new_value_9_editingFinished()
{
    command_formation(ui->param_new_value_9->text(),9);
    ui->param_current_value_9->setText(ui->param_new_value_9->text());
    current_param_value[9] = (ui->param_new_value_9->text()).toInt();
}

void awd::on_param_new_value_10_editingFinished()
{
    command_formation(ui->param_new_value_10->text(),10);
    ui->param_current_value_10->setText(ui->param_new_value_10->text());
    current_param_value[10] = (ui->param_new_value_10->text()).toInt();
}

void awd::on_param_new_value_11_editingFinished()
{
    command_formation(ui->param_new_value_11->text(),11);
    ui->param_current_value_11->setText(ui->param_new_value_11->text());
    current_param_value[11] = (ui->param_new_value_11->text()).toInt();
}

void awd::on_param_new_value_12_editingFinished()
{
    command_formation(ui->param_new_value_12->text(),12);
    ui->param_current_value_12->setText(ui->param_new_value_12->text());
    current_param_value[12] = (ui->param_new_value_12->text()).toInt();
}

void awd::on_param_new_value_13_editingFinished()
{
    command_formation(ui->param_new_value_13->text(),13);
    ui->param_current_value_13->setText(ui->param_new_value_13->text());
    current_param_value[13] = (ui->param_new_value_13->text()).toInt();
}

void awd::on_param_new_value_14_editingFinished()
{
    command_formation(ui->param_new_value_14->text(),14);
    ui->param_current_value_14->setText(ui->param_new_value_14->text());
    current_param_value[14] = (ui->param_new_value_14->text()).toInt();
}

void awd::on_param_new_value_15_editingFinished()
{
    command_formation(ui->param_new_value_15->text(),15);
    ui->param_current_value_15->setText(ui->param_new_value_15->text());
    current_param_value[15] = (ui->param_new_value_15->text()).toInt();
}

void awd::on_param_new_value_16_editingFinished()
{
    command_formation(ui->param_new_value_16->text(),16);
    ui->param_current_value_16->setText(ui->param_new_value_16->text());
    current_param_value[16] = (ui->param_new_value_16->text()).toInt();
}

void awd::on_param_new_value_17_editingFinished()
{
    command_formation(ui->param_new_value_17->text(),17);
    ui->param_current_value_17->setText(ui->param_new_value_17->text());
    current_param_value[17] = (ui->param_new_value_17->text()).toInt();
}

void awd::on_param_new_value_18_editingFinished()
{
    command_formation(ui->param_new_value_18->text(),18);
    ui->param_current_value_18->setText(ui->param_new_value_18->text());
    current_param_value[18] = (ui->param_new_value_18->text()).toInt();
}

void awd::on_param_new_value_19_editingFinished()
{
    command_formation(ui->param_new_value_19->text(),19);
    ui->param_current_value_19->setText(ui->param_new_value_19->text());
    current_param_value[19] = (ui->param_new_value_19->text()).toInt();
}

void awd::on_param_new_value_20_editingFinished()
{
    command_formation(ui->param_new_value_20->text(),20);
    ui->param_current_value_20->setText(ui->param_new_value_20->text());
    current_param_value[20] = (ui->param_new_value_20->text()).toInt();
}

void awd::on_param_new_value_21_editingFinished()
{
    command_formation(ui->param_new_value_21->text(),21);
    ui->param_current_value_21->setText(ui->param_new_value_21->text());
    current_param_value[21] = (ui->param_new_value_21->text()).toInt();
}

void awd::on_param_new_value_22_editingFinished()
{
    command_formation(ui->param_new_value_22->text(),22);
    ui->param_current_value_22->setText(ui->param_new_value_22->text());
    current_param_value[22] = (ui->param_new_value_22->text()).toInt();
}

void awd::on_param_new_value_23_editingFinished()
{
    command_formation(ui->param_new_value_23->text(),23);
    ui->param_current_value_23->setText(ui->param_new_value_23->text());
    current_param_value[23] = (ui->param_new_value_23->text()).toInt();
}

void awd::on_param_new_value_24_editingFinished()
{
    command_formation(ui->param_new_value_24->text(),24);
    ui->param_current_value_24->setText(ui->param_new_value_24->text());
    current_param_value[24] = (ui->param_new_value_24->text()).toInt();
}

void awd::on_param_new_value_25_editingFinished()
{
    command_formation(ui->param_new_value_25->text(),25);
    ui->param_current_value_25->setText(ui->param_new_value_25->text());
    current_param_value[25] = (ui->param_new_value_25->text()).toInt();
}

void awd::on_param_new_value_27_editingFinished()
{
    command_formation(ui->param_new_value_27->text(),27);
    ui->param_current_value_27->setText(ui->param_new_value_27->text());
    current_param_value[27] = (ui->param_new_value_27->text()).toInt();
}

void awd::on_param_new_value_28_editingFinished()
{
    command_formation(ui->param_new_value_28->text(),28);
    ui->param_current_value_28->setText(ui->param_new_value_28->text());
    current_param_value[28] = (ui->param_new_value_28->text()).toInt();
}

void awd::on_param_new_value_29_editingFinished()
{
    command_formation(ui->param_new_value_29->text(),29);
    ui->param_current_value_29->setText(ui->param_new_value_29->text());
    current_param_value[29] = (ui->param_new_value_29->text()).toInt();
}

void awd::on_param_new_value_30_editingFinished()
{
    command_formation(ui->param_new_value_30->text(),30);
    ui->param_current_value_30->setText(ui->param_new_value_30->text());
    current_param_value[30] = (ui->param_new_value_30->text()).toInt();
}

void awd::on_param_new_value_31_editingFinished()
{
    command_formation(ui->param_new_value_31->text(),31);
    ui->param_current_value_31->setText(ui->param_new_value_31->text());
    current_param_value[31] = (ui->param_new_value_31->text()).toInt();
}

void awd::on_param_new_value_32_editingFinished()
{
    command_formation(ui->param_new_value_32->text(),32);
    ui->param_current_value_32->setText(ui->param_new_value_32->text());
    current_param_value[32] = (ui->param_new_value_32->text()).toInt();
}

void awd::on_param_new_value_33_editingFinished()
{
    command_formation(ui->param_new_value_33->text(),33);
    ui->param_current_value_33->setText(ui->param_new_value_33->text());
    current_param_value[33] = (ui->param_new_value_33->text()).toInt();
}

void awd::on_param_new_value_34_editingFinished()
{
    command_formation(ui->param_new_value_34->text(),34);
    ui->param_current_value_34->setText(ui->param_new_value_34->text());
    current_param_value[34] = (ui->param_new_value_34->text()).toInt();
}

void awd::on_param_new_value_35_editingFinished()
{
    command_formation(ui->param_new_value_35->text(),35);
    ui->param_current_value_35->setText(ui->param_new_value_35->text());
    current_param_value[35] = (ui->param_new_value_35->text()).toInt();
}

void awd::on_param_new_value_36_editingFinished()
{
    command_formation(ui->param_new_value_36->text(),36);
    ui->param_current_value_36->setText(ui->param_new_value_36->text());
    current_param_value[36] = (ui->param_new_value_36->text()).toInt();
}

void awd::on_param_new_value_26_currentTextChanged(const QString &arg1)
{
    command_formation(arg1,26);
    current_param_value[26] = arg1.toInt();
    ui->param_current_value_26->setText(arg1);
}

void awd::on_save_to_file_clicked()
{
    QString filter = "AllFile (*.*) ;; Text File (*.txt)";
    QString file_name = QFileDialog::getOpenFileName(this,  "opena file", "С:://", filter);
    QFile file(file_name);
    //QFile file(".//params.txt");

    if(!file.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox::warning(this, "title", "file not open");
    }

    QTextStream out(&file);
    out << "#\t" << "Name\t" << "Comment Code\t" << "Min\t" << "Max\t" << "Default\t\n";// создание заголовка для файла
    // ввод значений
    for (int i = 0; i < 37; i++ )
    {
        out << kod[i] << "\t" << param_name[i] << "\t" << min_value[i] << "\t" << max_value[i] << "\t" << by_default[i] << "\t\n";
    }

    file.flush();
    file.close();
}

void awd::on_load_from_file_clicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(this,
                               "My title",
                               "This operation will owerwrite all parameters.\n"
                               "Are you sure you want to perform this this operation?",
                                QMessageBox::Yes | QMessageBox::No);
    if(reply == QMessageBox::Yes)
{
    QString filter = "AllFile (*.*) ;; Text File (*.txt)";
    QString file_name = QFileDialog::getOpenFileName(this,  "opena file", "С:://", filter);
    QFile file(file_name);

    if(!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox::warning(this, "Error", "File not open");
    }

    QTextStream in(&file);

    //QString text /*= in.readAll()*/;

    while (!in.atEnd())
    {
        for ( int i = 0; i < 38 ; i++ )
        {
           QString text = in.readLine();
           if (!text.isEmpty())
           {
              QStringList line = text.split('\t');
              if(i != 0)
              {
                 test_array[i] = QString(line[0]).toInt();
                 param_name[i] = line[1];
                 min_value[i] = QString(line[2]).toInt();
                 max_value[i] = QString(line[3]).toInt();
                 by_default[i] = QString(line[4]).toInt();
              }
           }
        }
    }
    file.close();
}

}
