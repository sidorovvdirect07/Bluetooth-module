#include "plotwidget.h"

PlotWidget::PlotWidget(QWidget* parent)
    : QChartView(parent) {

    setRenderHint(QPainter::Antialiasing);

    chart = new QtCharts::QChart();
    series = new QtCharts::QLineSeries();

    chart->setAxisX(new QtCharts::QValueAxis(), series);
    chart->setAxisY(new QtCharts::QValueAxis(), series);

    chart->addSeries(series);
    setChart(chart);
}

PlotWidget::~PlotWidget() {
    delete series;
    delete chart;
}

void PlotWidget::setXRange(double min, double max) {
    chart->axisX()->setRange(min, max);
}

void PlotWidget::setYRange(double min, double max) {
    chart->axisY()->setRange(min, max);
}

void PlotWidget::plot(QVector<double>& X, QVector<double>& Y, QString pen,
                      bool clear) {
    if (clear) {
        series->clear();
    }
    for (long i = 0; i < std::min(X.size(), Y.size()); ++i) {
        series->append(X[i], Y[i]);
    }
    series->setColor(QColor(pen));
}
