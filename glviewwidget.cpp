#include "glviewwidget.h"

GLViewWidget::GLViewWidget(QWidget* parent)
    : QWidget(parent) {
    graph = new QtDataVisualization::Q3DScatter();
    series = new QtDataVisualization::QScatter3DSeries();
    data = new QtDataVisualization::QScatterDataArray();
    series->dataProxy()->resetArray(data);
    graph->addSeries(series);
    container = QWidget::createWindowContainer(graph);
    layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(container);
    this->setLayout(layout);
}

GLViewWidget::~GLViewWidget() {
    delete data;
    delete graph;
    delete container;
    delete layout;
}

void GLViewWidget::setSize(long x, long y, long z) {
    graph->axisX()->setSegmentCount(x);
    graph->axisY()->setSegmentCount(y);
    graph->axisZ()->setSegmentCount(z);
}

void GLViewWidget::plot(QVector<qreal>& X, QVector<qreal>& Y, QVector<qreal>& Z,
                        QString color, long size) {
    data->clear();
    for (long i = 0; i < std::min(X.size(), Y.size(), Z.size()); ++i) {
        *data << QVector3D(X[i], Y[i], Z[i]);
    }
    series->setBaseColor(color);
    series->setItemSize(size);
    series->dataProxy()->resetArray(data);
}
