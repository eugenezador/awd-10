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
#include<bitset>
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
    std::bitset<16> mode_data;

    std::bitset<8> mode_status;

// Общие методы
    void serial_port_properties(const QString &value);

    unsigned char checkSumm(const unsigned char array[8]); // функция вычисления контрольной суммы

// Методы для параметров
    void command_formation(const QString &value, const int &param_num);// для кнопки записать параметры по умолчанию

    void command_formation(int param_num);// для кнопки считать параметры с платы

    int setData(int value, bool flag); // функция записи значения параметра в ячейки data1(flag = 1) или data2(flag = 0)

    void set_param_26_items();// выбор значений параметра 26

    void read_current_params();// для отлова всех параметров с платы


// Методы для режимов
    void set_mode_items();// Возможные режимы на выбор

    void set_mode_connections();// подача сигналов при изменении значений

// Методы для статуса
    void status_no_edit();// отключает возможность редактирования мышкой

    void status_read(const QByteArray &data);// выделяет и прочитанной команды значение статуса и выставляет соответствующие галочки

// Методы для регулятора
    void real_plot(const int &value);

    void slot_for_new_point();

    void slot_for_A_vx_1();

    void slot_for_A_vx_2();

private slots:  
// Параметры

    void on_write_all_by_default_clicked();

    void on_read_all_clicked();

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

    //void on_speed_slider_valueChanged(int value);

    void on_speed_horizontalSlider_valueChanged(int value);

    void on_stop_button_clicked();

// График
    void plot_settings();

    void on_btn_clear_clicked();


// Чтение и запись
    void writeData(const QByteArray &data);

    char16_t setReadDataValue(const QByteArray &data);

    void readData();

// Параметры слоты
    void on_param_new_value_0_editingFinished();

    void on_param_new_value_1_editingFinished();

    void on_param_new_value_2_editingFinished();

    void on_param_new_value_3_editingFinished();

    void on_param_new_value_4_editingFinished();

    void on_param_new_value_5_editingFinished();

    void on_param_new_value_6_editingFinished();

    void on_param_new_value_7_editingFinished();

    void on_param_new_value_8_editingFinished();

    void on_param_new_value_9_editingFinished();

    void on_param_new_value_10_editingFinished();

    void on_param_new_value_11_editingFinished();

    void on_param_new_value_12_editingFinished();

    void on_param_new_value_13_editingFinished();

    void on_param_new_value_14_editingFinished();

    void on_param_new_value_15_editingFinished();

    void on_param_new_value_16_editingFinished();

    void on_param_new_value_17_editingFinished();

    void on_param_new_value_18_editingFinished();

    void on_param_new_value_19_editingFinished();

    void on_param_new_value_20_editingFinished();

    void on_param_new_value_21_editingFinished();

    void on_param_new_value_22_editingFinished();

    void on_param_new_value_23_editingFinished();

    void on_param_new_value_24_editingFinished();

    void on_param_new_value_25_editingFinished();

    void on_param_new_value_27_editingFinished();

    void on_param_new_value_28_editingFinished();

    void on_param_new_value_29_editingFinished();

    void on_param_new_value_30_editingFinished();

    void on_param_new_value_31_editingFinished();

    void on_param_new_value_32_editingFinished();

    void on_param_new_value_33_editingFinished();

    void on_param_new_value_34_editingFinished();

    void on_param_new_value_35_editingFinished();

    void on_param_new_value_36_editingFinished();

    void on_param_new_value_26_currentTextChanged(const QString &arg1);

    // работа с файлами
    void on_save_to_file_clicked();

    void on_load_from_file_clicked();




private:
    Ui::awd *ui;

    QSerialPort *serial;// указатель на область памяти для экземпляра порта

    QTimer *timer;

    QVector<double> qv_x, qv_y;
};
#endif // AWD_H
