#ifndef FREEIMUCAL_H
#define FREEIMUCAL_H

#include "callib.h"
#include <QFile>
#include <QFileDialog>
#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QSettings>
#include <QThread>
#include <QVector2D>
#include <memory.h>

#define acc_file_name "acc.txt"
#define magn_file_name "magn.txt"
#define calibration_h_file_name "calibration.h"
#define word 2
#define acc_range 25000
#define magn_range 1500

class SerialWorker;

namespace Ui {
class FreeIMUCal;
}

class FreeIMUCal : public QMainWindow {
    Q_OBJECT

public:
    FreeIMUCal(QWidget* parent = nullptr);
    ~FreeIMUCal();

    void set_status(QString);
    void serial_connect();
    void serial_disconnect();
    void sampling_start();
    void sampling_end();
    void calibrate();
    void save_calibration_header();
    void save_calibration_eeprom();
    void clear_calibration_eeprom();
    void newData(QVector<short>);

private:
    Ui::FreeIMUCal* ui{nullptr};
    QSettings* settings{nullptr};
    QVector<QVector<double>> acc_data;
    QVector<QVector<double>> magn_data;
    QString serial_port;
    std::shared_ptr<QSerialPort> ser{nullptr};
    SerialWorker* serWorker{nullptr};
    QVector<long> acc_offset;
    QVector<double> acc_scale;
    QVector<long> magn_offset;
    QVector<double> magn_scale;
    QVector<QVector<double>> acc_cal_data;
    QVector<QVector<double>> magn_cal_data;
};

class SerialWorker : public QThread {
    Q_OBJECT
public:
    SerialWorker(std::shared_ptr<QSerialPort> ser, QObject* parent = nullptr);
    ~SerialWorker();
    void run();

    bool getExiting() const;
    void setExiting(bool newExiting);

signals:
    void new_data_signal(QVector<short>);

private:
    std::shared_ptr<QSerialPort> ser{nullptr};
    bool exiting;
    QFile acc_file;
    QFile magn_file;
};
#endif // FREEIMUCAL_H
