#ifndef PLOTWIDGET_H
#define PLOTWIDGET_H

#include <QChartView>
#include <QLineSeries>
#include <QObject>
#include <QValueAxis>

class PlotWidget : public QtCharts::QChartView {
public:
    PlotWidget(QWidget* parent);
    ~PlotWidget();
    void setXRange(double min, double max);
    void setYRange(double min, double max);
    //    void setAspectLocked();
    void plot(QVector<double>& X, QVector<double>& Y, QString pen = "#ff0000",
              bool clear = true);

private:
    QtCharts::QChart* chart;
    QtCharts::QLineSeries* series;
};

#endif // PLOTWIDGET_H
