#ifndef CALLIB_H
#define CALLIB_H

#include "matrix.h"
#include <QFile>
#include <QTextStream>
#include <QVector>
#include <vector>

class CalLib {
public:
    CalLib();
    static QPair<QVector<long>, QVector<double>>&&
    calibrate(QVector<double>& x, QVector<double>& y, QVector<double>& z);

    static QPair<QVector<long>, QVector<double>>&&
    calibrate_from_file(QString file_name);

    static QVector<QVector<double>>
    compute_calibrate_data(QVector<QVector<double>>& data,
                           QVector<long>& offsets, QVector<double>& scale);
};

#endif // CALLIB_H
