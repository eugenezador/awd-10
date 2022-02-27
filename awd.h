#ifndef AWD_H
#define AWD_H

#include<QMainWindow>
#include<QTimer>
#include<QSerialPort>
#include<QSerialPortInfo>
#include<QDebug>
#include<QMessageBox>
#include<QFile>
#include<QTextStream>
//#include<bitset>
#include<qcustomplot.h>


QT_BEGIN_NAMESPACE
namespace Ui { class awd; }
QT_END_NAMESPACE

class awd : public QMainWindow
{
    Q_OBJECT

public:
    awd(QWidget *parent = nullptr);
    ~awd();

    unsigned char adress = '\x05';

    int reply_time = 90;

    unsigned char command[8] = {adress,'\x0','\x0','\x0','\x0','\x0','\x0','\x0'};

    QLabel* param_current_value_array[38];

//переменные для параметров
    QString param_name[38] = {"Сетевой адрес","Смещение нуля внешнего аналогового входа 1", "Смещение нуля внешнего аналогового входа 2",
                              "Смещение нуля аналогового входа <<противо-ЭДС>>", "Смещение нуля аналогового входа  <<ток>>",
                              "Резерв","Резерв","Резерв","Резерв","Резерв","Резерв","Резерв","Резерв","Ограничение минимального значения ШИМ",
                              "Ограничение максимального значения ШИМ","Пропорциональный коэффициент ПИД-регулятора","Интегральный коэффициент ПИД-регулятора",
                              "Дифференциальный коэффицииент ПИД-регулятора", "Резерв","Резерв","Резерв","Ограничение пропорциональной части ПИД-регулятора",
                              "Ограничение интегральной части ПИД-регулятора", "Ограничение диффференциальной части ПИД-регулятора",
                              "Коэффициент периода вычисления ПИД-регулятора","Коэффициент времемни задержки перед измерением ЭДС двигателя",
                              "Количество измерений ЭДС двигателя","Ограничение максимального тока двигателя", "Режим работы платы",
                              "Максимальная частота вращения вала энкодера","Количество импульсов на оборот энкодера","Дифференциальное значение",
                              "<<Зона нечувствительности>>", "Скорость, при которой изменяется направление вращения",
                              "Скорость вращения при принудительном управлении в режиме Сл", "Коэффициент усиления","Ограничение максимальной скорости в режиме М"};

    unsigned int by_default[38] = {5,16,16,16,16,1023,1023,1023,1023,1023,1023,1023,1023,5,
                          991,768,160,0,1023,1023,1023,1023,768,1023,160,5,16,15,0,
                          30,64,1023,20,80,512,255,1020};

    unsigned int current_param_value[38];


// переменные для режимов
    //std::bitset<16> mode_data;
    unsigned char mode_data1 = 0;
    unsigned char mode_data0 = 0;

    //std::bitset<8> mode_status;
    unsigned char mode_status = 0;

// Общие методы
    void serial_port_properties(const QString &text);

    unsigned char checkSumm(const unsigned char array[8]); // функция вычисления контрольной суммы

// Методы для параметров
    void command_formation(const QString &value, const int &param_num);// для кнопки записать параметры по умолчанию

    void command_formation(int param_num);// для кнопки считать параметры с платы

    void set_param_26_items();// выбор значений параметра 26

    void read_current_params();// для отлова всех параметров с платы

    void set_labels_array();


// Методы для режимов
    void set_mode_items();// Возможные режимы на выбор

    void set_mode_connections();// подача сигналов при изменении значений

// Методы для статуса
    void status_no_edit();// отключает возможность редактирования мышкой

    void status_read(const QByteArray &data);// выделяет и прочитанной команды значение статуса и выставляет соответствующие галочки

// Методы для регулятора
    void real_plot(const QByteArray &data);

    void slot_for_new_point();

    void chart_update_period(const int &value);

    //void slotMousePress(QMouseEvent * event);

    //void slot_for_A_vx_1();

    //void slot_for_A_vx_2();

private slots:  
// Параметры

    void on_write_all_by_default_clicked();

    void on_read_all_clicked();

// Работа с файлами
    void on_save_to_file_clicked();

    void on_load_from_file_clicked();

    void on_write_selected_clicked();

// Режим
    void change_state();

    void on_read_button_regime_clicked();

    void on_write_button_regime_clicked();

    void on_stop_mode_button_clicked();

    void on_start_tracking_clicked();

// Регулятор
    void on_Kp_valueChanged(int value);

    void on_Ki_valueChanged(int value);

    void on_Kd_valueChanged(int value);

    void on_speed_horizontalSlider_valueChanged(int value);

    void on_stop_button_clicked();

    void on_Kp_spinBox_valueChanged(const QString &arg1);

    void on_Ki_spinBox_valueChanged(const QString &arg1);

    void on_Kd_spinBox_valueChanged(const QString &arg1);

// График
    void plot_settings();

    void on_btn_clear_clicked();

// Чтение и запись
    void writeData(const QByteArray &data);

    char16_t setReadDataValue(const QByteArray &data);// считывает значения с позиций дата0 и дата1

    void readData();

    void on_spinBox_period_editingFinished();

    void on_export_button_clicked();

private:
    Ui::awd *ui;

    QSerialPort *serial;// указатель на область памяти для экземпляра порта

    QString currentPortName;// для записи предыдущего значения порта

    QTimer *timer;

    //QCPItemTracer *tracer;

    QVector<double> qv_x, qv_y;//вектор скорости

    QVector<double> qavx1_x, qavx1_y;// вектор Aвх1

    QVector<double> qavx2_x, qavx2_y;// вектор Aвх2

};
#endif // AWD_H
