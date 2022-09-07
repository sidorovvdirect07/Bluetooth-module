#include "callib.h"

CalLib::CalLib() {
}

QPair<QVector<long>, QVector<double>>&&
CalLib::calibrate(QVector<double>& x, QVector<double>& y, QVector<double>& z) {
    matrices::Matrix<double> X(x.toStdVector());
    matrices::Matrix<double> Y(y.toStdVector());
    matrices::Matrix<double> Z(z.toStdVector());
    QVector<double> data;
    data.append(x);
    data.append(y);
    data.append(z);
    data.append(
        QVector<double>::fromStdVector((Y.simpleMul(Y) * -1).toVector()));
    data.append(
        QVector<double>::fromStdVector((Z.simpleMul(Z) * -1).toVector()));
    data.append(QVector<double>(x.size(), 1));
    for (int i = 0; i < x.size() - 6; ++i) {
        data.append(QVector<double>(x.size(), 0));
    }

    const int N = x.size();
    matrices::Matrix<double> H(data.toStdVector());
    H.reshape(N, N);

    auto W = X.simpleMul(X).transpose();
    auto solutions = (H.transpose() * H).invert() * H.transpose() * W;

    double OSx = solutions.getAt(0, 0) / 2;
    double OSy = solutions.getAt(0, 1) / (2 * solutions.getAt(0, 3));
    double OSz = solutions.getAt(0, 2) / (2 * solutions.getAt(0, 4));

    double A = solutions.getAt(0, 5) + std::pow(OSx, 2) +
        solutions.getAt(0, 3) * std::pow(OSy, 2) +
        solutions.getAt(0, 4) * std::pow(OSz, 2);
    double B = A / solutions.getAt(0, 3);
    double C = A / solutions.getAt(0, 4);

    double SCx = std::sqrt(A);
    double SCy = std::sqrt(B);
    double SCz = std::sqrt(C);

    // type conversion from numpy.float64 to standard python floats
    QVector<long> offsets = {(long) std::round(OSx), (long) std::round(OSy),
                             (long) std::round(OSz)};
    QVector<double> scale = {SCx, SCy, SCz};

    return std::move(qMakePair(offsets, scale));
}

QPair<QVector<long>, QVector<double>>&&
CalLib::calibrate_from_file(QString file_name) {
    QFile file(file_name);
    file.open(QFile::ReadOnly);
    QVector<double> samples_x;
    QVector<double> samples_y;
    QVector<double> samples_z;
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        auto reading = line.split(" ");
        if (reading.size() == 3) {
            samples_x.append(reading[0].toDouble());
            samples_y.append(reading[1].toDouble());
            samples_z.append(reading[2].toDouble());
        }
    }
    return calibrate(samples_x, samples_y, samples_z);
}

QVector<QVector<double>>
CalLib::compute_calibrate_data(QVector<QVector<double>>& data,
                               QVector<long>& offsets, QVector<double>& scale) {
    QVector<QVector<double>> output(3);
    for (int i = 0; i < data[0].size(); ++i) {
        output[0].append((data[0][i] - offsets[0]) / scale[0]);
        output[1].append((data[1][i] - offsets[1]) / scale[1]);
        output[2].append((data[2][i] - offsets[2]) / scale[2]);
    }
    return output;
}
