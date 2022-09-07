#include "freeimucal.h"
#include "ui_freeimu_cal.h"

FreeIMUCal::FreeIMUCal(QWidget* parent)
    : QMainWindow(parent),
      ui(new Ui::FreeIMUCal) {
    // Set up the user interface from Designer.
    ui->setupUi(this);

    ser = std::make_shared<QSerialPort>();

    // load user settings
    settings =
        new QSettings("FreeIMU Calibration Application", "Fabio Varesano");
    // restore previous serial port used
    ui->serialPortEdit->setText(
        settings->value("calgui/serialPortEdit", "COM1").toString());

    // when user hits enter, we generate the clicked signal to the button so
    // that connections
    connect(ui->serialPortEdit, &QLineEdit::returnPressed, ui->connectButton,
            &QPushButton::click);

    // Connect up the buttons to their functions
    connect(ui->connectButton, &QPushButton::clicked, this,
            &FreeIMUCal::serial_connect);
    connect(ui->samplingToggleButton, &QPushButton::clicked, this,
            &FreeIMUCal::sampling_start);
    set_status("Disconnected");

    // data storages
    acc_data.resize(3);
    magn_data.resize(3);

    // setup graphs
    ui->accXY->setXRange(-acc_range, acc_range);
    ui->accXY->setYRange(-acc_range, acc_range);

    ui->accYZ->setXRange(-acc_range, acc_range);
    ui->accYZ->setYRange(-acc_range, acc_range);
    ui->accZX->setXRange(-acc_range, acc_range);
    ui->accZX->setYRange(-acc_range, acc_range);

    //    ui->accXY->setAspectLocked();
    //    ui->accYZ->setAspectLocked();
    //    ui->accZX->setAspectLocked();

    ui->magnXY->setXRange(-magn_range, magn_range);
    ui->magnXY->setYRange(-magn_range, magn_range);
    ui->magnYZ->setXRange(-magn_range, magn_range);
    ui->magnYZ->setYRange(-magn_range, magn_range);
    ui->magnZX->setXRange(-magn_range, magn_range);
    ui->magnZX->setYRange(-magn_range, magn_range);

    //    ui->magnXY->setAspectLocked();
    //    ui->magnYZ->setAspectLocked();
    //    ui->magnZX->setAspectLocked();

    ui->accXY_cal->setXRange(-1.5, 1.5);
    ui->accXY_cal->setYRange(-1.5, 1.5);
    ui->accYZ_cal->setXRange(-1.5, 1.5);
    ui->accYZ_cal->setYRange(-1.5, 1.5);
    ui->accZX_cal->setXRange(-1.5, 1.5);
    ui->accZX_cal->setYRange(-1.5, 1.5);

    //    ui->accXY_cal->setAspectLocked();
    //    ui->accYZ_cal->setAspectLocked();
    //    ui->accZX_cal->setAspectLocked();

    ui->magnXY_cal->setXRange(-1.5, 1.5);
    ui->magnXY_cal->setYRange(-1.5, 1.5);
    ui->magnYZ_cal->setXRange(-1.5, 1.5);
    ui->magnYZ_cal->setYRange(-1.5, 1.5);
    ui->magnZX_cal->setXRange(-1.5, 1.5);
    ui->magnZX_cal->setYRange(-1.5, 1.5);

    //    ui->magnXY_cal->setAspectLocked();
    //    ui->magnYZ_cal->setAspectLocked();
    //    ui->magnZX_cal->setAspectLocked();

    //    ui->acc3D->opts['distance'] = 30000;
    // self.acc3D.show();

    //    ui->magn3D->opts['distance'] = 2000;
    // self.magn3D.show();

    ui->acc3D->setSize(20000, 20000, 20000);
    ui->magn3D->setSize(1000, 1000, 1000);

    // axis for the cal 3D graph
    ui->acc3D_cal->setSize(10000, 10000, 10000);
    ui->magn3D_cal->setSize(1000, 1000, 1000);
}

FreeIMUCal::~FreeIMUCal() {
    delete ui;
    delete settings;
    ser->close();
    delete serWorker;
}

void FreeIMUCal::set_status(QString status) {
    ui->statusbar->showMessage(status);
}

void FreeIMUCal::serial_connect() {
    serial_port = ui->serialPortEdit->text();
    // save serial value to user settings
    settings->setValue("calgui/serialPortEdit", serial_port);

    ui->connectButton->setEnabled(false);
    // waiting mouse cursor
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    set_status("Connecting to " + serial_port + " ...");

    // TODO : serial port field input validation !

    try {
        ser->setPort(QSerialPortInfo(serial_port));
        ser->setBaudRate(115200);
        ser->setParity(QSerialPort::Parity::NoParity);
        ser->setStopBits(QSerialPort::StopBits::OneStop);
        ser->setDataBits(QSerialPort::DataBits::Data8);

        if (ser->isOpen()) {

            qDebug() << "Arduino serial port opened correctly";
            set_status("Connection Successfull. Waiting for Arduino reset...");
            // wait for arduino reset on serial open
            QThread::sleep(3);
            // clear serial interface buffers
            ser->clear();
            QThread::msleep(100);
            ser->write("v", 1); // ask version

            set_status("Connected to: " + QString(ser->readLine(100000)));

            // TODO hangs if a wrong serial protocol has been loaded.To be
            // fixed.

            ui->connectButton->setText("Disconnect");
            connect(ui->connectButton, &QPushButton::clicked, this,
                    &FreeIMUCal::serial_disconnect);

            ui->serialPortEdit->setEnabled(false);
            ui->serialProtocol->setEnabled(false);

            ui->samplingToggleButton->setEnabled(true);

            ui->clearCalibrationEEPROMButton->setEnabled(true);
            connect(ui->clearCalibrationEEPROMButton, &QPushButton::clicked,
                    this, &FreeIMUCal::clear_calibration_eeprom);
        }
    } catch (...) {
        ui->connectButton->setEnabled(true);
        set_status("Impossible to connect: error id" +
                   QString::number(ser->error()));
    }

    // restore mouse cursor
    QApplication::restoreOverrideCursor();
    ui->connectButton->setEnabled(true);
}

void FreeIMUCal::serial_disconnect() {
    qDebug() << "Disconnecting from " + serial_port;
    ser->close();
    set_status("Disconnected");
    ui->serialPortEdit->setEnabled(true);
    ui->serialProtocol->setEnabled(true);

    ui->connectButton->setText("Connect");
    disconnect(ui->connectButton, &QPushButton::clicked, this,
               &FreeIMUCal::serial_disconnect);
    connect(ui->connectButton, &QPushButton::clicked, this,
            &FreeIMUCal::serial_connect);

    ui->samplingToggleButton->setEnabled(false);

    ui->clearCalibrationEEPROMButton->setEnabled(false);
    disconnect(ui->clearCalibrationEEPROMButton, &QPushButton::clicked, this,
               &FreeIMUCal::clear_calibration_eeprom);
}

void FreeIMUCal::sampling_start() {
    serWorker = new SerialWorker(ser);
    connect(serWorker, &SerialWorker::new_data_signal, this,
            &FreeIMUCal::newData);

    serWorker->start();
    qDebug() << "Starting SerialWorker";
    ui->samplingToggleButton->setText("Stop Sampling");
    disconnect(ui->samplingToggleButton, &QPushButton::clicked, this,
               &FreeIMUCal::sampling_start);
    connect(ui->samplingToggleButton, &QPushButton::clicked, this,
            &FreeIMUCal::sampling_end);
}

void FreeIMUCal::sampling_end() {
    serWorker->setExiting(true);
    serWorker->quit();
    serWorker->wait();
    ui->samplingToggleButton->setText("Start Sampling");
    disconnect(ui->samplingToggleButton, &QPushButton::clicked, this,
               &FreeIMUCal::sampling_end);
    connect(ui->samplingToggleButton, &QPushButton::clicked, this,
            &FreeIMUCal::sampling_start);

    ui->calibrateButton->setEnabled(true);
    ui->calAlgorithmComboBox->setEnabled(true);
    connect(ui->calibrateButton, &QPushButton::clicked, this,
            &FreeIMUCal::calibrate);
}

void FreeIMUCal::calibrate() {
    // read file and run calibration algorithm
    auto acc_params = CalLib::calibrate_from_file(acc_file_name);
    acc_offset = acc_params.first;
    acc_scale = acc_params.second;

    auto magn_params = CalLib::calibrate_from_file(magn_file_name);
    magn_offset = magn_params.first;
    magn_scale = magn_params.second;

    // show calibrated tab
    ui->tabWidget->setCurrentIndex(1);

    // populate acc calibration output on gui
    ui->calRes_acc_OSx->setText(QString::number(acc_offset[0]));
    ui->calRes_acc_OSy->setText(QString::number(acc_offset[1]));
    ui->calRes_acc_OSz->setText(QString::number(acc_offset[2]));

    ui->calRes_acc_SCx->setText(QString::number(acc_scale[0]));
    ui->calRes_acc_SCy->setText(QString::number(acc_scale[1]));
    ui->calRes_acc_SCz->setText(QString::number(acc_scale[2]));

    // populate acc calibration output on gui
    ui->calRes_magn_OSx->setText(QString::number(magn_offset[0]));
    ui->calRes_magn_OSy->setText(QString::number(magn_offset[1]));
    ui->calRes_magn_OSz->setText(QString::number(magn_offset[2]));

    ui->calRes_magn_SCx->setText(QString::number(magn_scale[0]));
    ui->calRes_magn_SCy->setText(QString::number(magn_scale[1]));
    ui->calRes_magn_SCz->setText(QString::number(magn_scale[2]));

    // compute calibrated data
    acc_cal_data =
        CalLib::compute_calibrate_data(acc_data, acc_offset, acc_scale);
    magn_cal_data =
        CalLib::compute_calibrate_data(magn_data, magn_offset, magn_scale);

    // populate 2D graphs with calibrated data
    ui->accXY_cal->plot(acc_cal_data[0], acc_cal_data[1], "#ff0000");
    ui->accYZ_cal->plot(acc_cal_data[1], acc_cal_data[2], "#008000");
    ui->accZX_cal->plot(acc_cal_data[2], acc_cal_data[0], "#0000ff");

    ui->magnXY_cal->plot(magn_cal_data[0], magn_cal_data[1], "#ff0000");
    ui->magnYZ_cal->plot(magn_cal_data[1], magn_cal_data[2], "#008000");
    ui->magnZX_cal->plot(magn_cal_data[2], magn_cal_data[0], "#0000ff");

    ui->acc3D_cal->plot(acc_cal_data[0], acc_cal_data[1], acc_cal_data[2],
                        "#000000");
    ui->magn3D_cal->plot(magn_cal_data[0], magn_cal_data[1], magn_cal_data[2],
                         "#000000");

    // enable calibration buttons to activate calibration storing functions
    ui->saveCalibrationHeaderButton->setEnabled(true);
    connect(ui->saveCalibrationHeaderButton, &QPushButton::clicked, this,
            &FreeIMUCal::save_calibration_header);

    ui->saveCalibrationEEPROMButton->setEnabled(true);
    connect(ui->saveCalibrationEEPROMButton, &QPushButton::clicked, this,
            &FreeIMUCal::save_calibration_eeprom);
}

void FreeIMUCal::save_calibration_header() {
    QString text =
        ""
        "/**"
        "* FreeIMU calibration header. Automatically generated by FreeIMU_GUI."
        "* Do not edit manually unless you know what you are doing."
        "*/"

        "#define CALIBRATION_H"

        "const int acc_off_x = %d;"
        "const int acc_off_y = %d;"
        "const int acc_off_z = %d;"
        "const float acc_scale_x = %f;"
        "const float acc_scale_y = %f;"
        "const float acc_scale_z = %f;"

        "const int magn_off_x = %d;"
        "const int magn_off_y = %d;"
        "const int magn_off_z = %d;"
        "const float magn_scale_x = %f;"
        "const float magn_scale_y = %f;"
        "const float magn_scale_z = %f;"
        "";
    QString calibration_h_text = QString::asprintf(
        text.toUtf8(), acc_offset[0], acc_offset[1], acc_offset[2],
        acc_scale[0], acc_scale[1], acc_scale[2], magn_offset[0],
        magn_offset[1], magn_offset[2], magn_scale[0], magn_scale[1],
        magn_scale[2]);

    QString calibration_h_folder = QFileDialog::getExistingDirectory(
        this, "Select the Folder to which save the calibration.h file");
    QFile calibration_h_file(calibration_h_folder + calibration_h_file_name);
    calibration_h_file.open(QFile::ReadOnly);
    calibration_h_file.write(calibration_h_text.toUtf8());
    calibration_h_file.close();

    set_status("Calibration saved to: " + calibration_h_folder +
               calibration_h_file_name +
               " .\nRecompile and upload the program using the "
               "FreeIMU library to your microcontroller.");
}

void FreeIMUCal::save_calibration_eeprom() {
    ser->write("c");
    // pack data into a string
    QString offsets =
        QString::asprintf("%04x%04x%04x%04x%04x%04x", (int16_t) acc_offset[0],
                          (int16_t) acc_offset[1], (int16_t) acc_offset[2],
                          (int16_t) magn_offset[0], (int16_t) magn_offset[1],
                          (int16_t) magn_offset[2]);

    QString scales = QString::asprintf(
        "%08a%08a%08a%08a%08a%08a", acc_scale[0], acc_scale[1], acc_scale[2],
        magn_scale[0], magn_scale[1], magn_scale[2]);
    // transmit to microcontroller
    ser->write(QByteArray::fromHex(offsets.toUtf8()));
    ser->write(QByteArray::fromHex(scales.toUtf8()));
    set_status("Calibration saved to microcontroller EEPROM.");
    // debug written values to console
    qDebug() << "Calibration values read back from EEPROM:";
    ser->write("C");
    for (int i = 0; i < 4; ++i) {
        qDebug() << ser->readLine(1000000);
    }
}

void FreeIMUCal::clear_calibration_eeprom() {
    ser->write("x");
    // no feedback expected.we assume success.
    set_status("Calibration cleared from microcontroller EEPROM.");
}

void FreeIMUCal::newData(QVector<short> reading) {
    // only display last reading in burst
    acc_data[0].append(reading[0]);
    acc_data[1].append(reading[1]);
    acc_data[2].append(reading[2]);

    magn_data[0].append(reading[6]);
    magn_data[1].append(reading[7]);
    magn_data[2].append(reading[8]);

    ui->accXY->plot(acc_data[0], acc_data[1], "#ff0000");
    ui->accYZ->plot(acc_data[1], acc_data[2], "#008000");
    ui->accZX->plot(acc_data[2], acc_data[0], "#0000ff");

    ui->magnXY->plot(magn_data[0], magn_data[1], "#ff0000");
    ui->magnYZ->plot(magn_data[1], magn_data[2], "#008000");
    ui->magnZX->plot(magn_data[2], magn_data[0], "#0000ff");

    ui->acc3D->plot(acc_data[0], acc_data[1], acc_data[2], "#000000");
    ui->magn3D->plot(magn_data[0], magn_data[1], magn_data[2], "#000000");
}

SerialWorker::SerialWorker(std::shared_ptr<QSerialPort> ser, QObject* parent)
    : QThread(parent),
      ser(ser),
      exiting(false) {
}

SerialWorker::~SerialWorker() {
    exiting = true;
    wait();
    qDebug() << "SerialWorker exits..";
}

void SerialWorker::run() {
    qDebug() << "sampling start..";
    acc_file.setFileName(acc_file_name);
    acc_file.open(QFile::WriteOnly);
    magn_file.setFileName(magn_file_name);
    magn_file.open(QFile::WriteOnly);
    int count = 100;
    int in_values = 9;
    QVector<int16_t> reading(in_values, 0);
    // read data for calibration
    while (!exiting) {
        // determine word size
        ser->write("b");
        ser->write(QString(QChar(count)).toUtf8());
        for (int j = 0; j < count; ++j) {
            for (int i = 0; i < in_values; ++i) {
                if (word == 4) {
                    QDataStream stream(ser->read(4));
                    stream >> reading[i];
                }
                if (word == 2) {
                    QDataStream stream(ser->read(2));
                    stream >> reading[i];
                }
            }
            ser->read(2);
            if (reading[8] == 0) {
                reading[6] = 1;
                reading[7] = 1;
                reading[8] = 1;
            }
            // prepare readings to store on file
            QString acc_readings_line =
                QString("%d %d %d\r\n").arg(reading[0], reading[1], reading[2]);
            acc_file.write(acc_readings_line.toUtf8());
            QString magn_readings_line =
                QString("%d %d %d\r\n").arg(reading[6], reading[7], reading[8]);
            magn_file.write(magn_readings_line.toUtf8());
        }

        // every count times we pass some data to the GUI
        emit new_data_signal(reading);
        qDebug() << ".";
    }

    // closing acc and magn files
    acc_file.close();
    magn_file.close();
    return;
}

bool SerialWorker::getExiting() const {
    return exiting;
}

void SerialWorker::setExiting(bool newExiting) {
    exiting = newExiting;
}
