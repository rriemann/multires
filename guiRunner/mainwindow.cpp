/***************************************************************************************
 * Copyright (c) 2013 Robert Riemann <robert@riemann.cc>                                *
 *                                                                                      *
 * This program is free software; you can redistribute it and/or modify it under        *
 * the terms of the GNU General Public License as published by the Free Software        *
 * Foundation; either version 2 of the License, or (at your option) any later           *
 * version.                                                                             *
 *                                                                                      *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
 *                                                                                      *
 * You should have received a copy of the GNU General Public License along with         *
 * this program.  If not, see <http://www.gnu.org/licenses/>.                           *
 ****************************************************************************************/

#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "functions.h"

#include <QDebug>
#include <QSpinBox>
#include <QTimer>

#include <QGraphicsScene>
#include <QGraphicsRectItem>

#include "multires/multires_grid.hpp"
#include "multires/node.hpp"
#include "monores/monores_grid.hpp"
#include "theory.hpp"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , scene(new QGraphicsScene)
{
    ui->setupUi(this);
    ui->splitter->setSizes(QList<int>() << 100 << 100);
    ui->graphicsView->setScene(scene);

    spinBox = new QSpinBox(this);
    spinBox->setMinimum(1);
    spinBox->setValue(1);
    ui->mainToolBar->addWidget(spinBox);

    timer = new QTimer(this);
    timer->setInterval(100);

    customPlot = ui->customPlot;

    connect(ui->actionRun, SIGNAL(triggered()), this, SLOT(actionRun()));
    connect(ui->actionAutoPlay, SIGNAL(toggled(bool)), this, SLOT(autoPlayToggled(bool)));
    connect(timer, SIGNAL(timeout()), this, SLOT(actionRun()));
    connect(ui->actionInitializeRoot, SIGNAL(triggered()), this, SLOT(initializeGrids()));
    connect(ui->actionRescale, SIGNAL(triggered()), this, SLOT(rescale()));

    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    qDebug();
    qDebug() << QString("max level: %1").arg(g_level);

    customPlot->addGraph();
    customPlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 3));
    // give the axes some labels:
    customPlot->xAxis->setLabel("x");
    customPlot->yAxis->setLabel("y");
    // set axes ranges, so we see all data:
    customPlot->xAxis->setRange(g_x0[dimX], g_x1[dimX]);
    customPlot->yAxis->setRange(-0.2, 1.2);

    customPlot->graph(0)->setPen(QPen(Qt::black));

    customPlot->addGraph();
    customPlot->graph(1)->setPen(QPen(Qt::green));

    customPlot->addGraph();
    customPlot->graph(2)->setPen(QPen(Qt::magenta));

    for(size_t i = 0; i < bars.size() ; ++i) {
        bars[i] = new QCPBars(customPlot->xAxis, customPlot->yAxis);
        customPlot->addPlottable(bars[i]);
        bars[i]->setPen(QPen(Qt::transparent));
        bars[i]->setWidth(0.005);
    }
    bars[0]->setName("Active Elements");
    bars[0]->setBrush(QBrush(Qt::blue));
    /*
    bars[1]->setName("Virtual Elements");
    bars[1]->setBrush(QBrush(Qt::red));

    bars[2]->setName("Savety Zone");
    bars[2]->setBrush(QBrush(Qt::yellow));

    bars[3]->setName("Static Elements");
    bars[3]->setBrush(QBrush(Qt::black));
    */

    m_theory = new theory_t(g_level);
    initializeGrids();
    rescale();
}

MainWindow::~MainWindow()
{
    deleteGrids();
    delete m_theory;
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    ui->graphicsView->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
    QMainWindow::resizeEvent(event);
}

void MainWindow::initializeGrids()
{
    deleteGrids();

    m_grid_mono  = new monores_grid_t(g_level);
    m_grid_multi = new multires_grid_t(g_level);

    count_nodes = (1 << g_level);

    replot();
}

void MainWindow::actionRun()
{
    int stepAtOnce = spinBox->value();
    real timeInterval = g_timestep*stepAtOnce;
    if(m_grid_multi) {
        real runTime = 0;
        size_t counter = 0;
        do {
            runTime += m_grid_multi->timeStep();
            counter++;
        } while(runTime < timeInterval);
        qDebug() << "counter:" << counter++;
    }

    if(m_grid_mono) {
        size_t counter = 0;
        do {
            m_grid_mono->timeStep();
            counter++;
        } while(m_grid_mono->getTime() < m_grid_multi->getTime());
        qDebug() << "counter_regular:" << counter++;
    }

    replot();
}

void MainWindow::replot()
{

    QVector<real> xvalues, yvalues, yvaluestheory, lvlvalues;
    for(const point_t &point: *m_grid_multi) {
        xvalues << point.m_x[dimX];
        yvalues << point.m_phi;
        yvaluestheory << m_theory->at(point.m_index, m_grid_multi->getTime());
        lvlvalues << pow(2,-point.getLevel(g_level));
    }
    count_nodes_packed = m_grid_multi->size();
    QString pack_rate_time = QString("pack rate: %1/%2 = %3, time: %4").arg(count_nodes_packed).arg(count_nodes).arg(real(count_nodes_packed)/count_nodes).arg(m_grid_multi->getTime());
    qDebug() << pack_rate_time;
    statusBar()->showMessage(pack_rate_time);

    customPlot->graph(0)->setData(xvalues, yvalues); // black
    customPlot->graph(1)->setData(xvalues, yvaluestheory); // green
    bars[0]->setData(xvalues, lvlvalues); // blue


    QVector<real> xvalues_regular, yvalues_regular;
    for(const point_t &point: *m_grid_mono) {
        xvalues_regular << point.m_x[dimX];
        yvalues_regular << point.m_phi;
    }
    customPlot->graph(2)->setData(xvalues_regular, yvalues_regular); // magenta

    customPlot->replot();

    scene->clear();
    blockBuilder(m_grid_multi->getRootNode());
    ui->graphicsView->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
}

void MainWindow::blockBuilder(const node_t *node)
{
    const point_t *point = node->getPoint();
    const int level = point->getLevel(g_level);
    const static qreal height = 20;
    const qreal stretchX = 300;
    qreal width = stretchX/(1 << level);
    qreal center = point->m_x[dimX]*stretchX/2;
    qreal bottom = level*height;
    qreal x = center-0.5*width;

    static QBrush brush(Qt::SolidPattern);
    brush.setColor(Qt::blue);
    const QPen pen(Qt::transparent);
    scene->addRect(x, -bottom, width, height, pen, brush);
    scene->addLine(x, -bottom+height, x + width, -bottom+height, QPen(Qt::black));
    scene->addLine(center, -bottom+height, center+0.25*width, -bottom, QPen(Qt::black));
    scene->addLine(center, -bottom+height, center-0.25*width, -bottom, QPen(Qt::black));

    if (node->getChilds()) {
        for(node_t const &child : *node->getChilds()) {
            blockBuilder(&child);
        }
    }
}

void MainWindow::rescale()
{
    customPlot->rescaleAxes();
    replot();
}

void MainWindow::autoPlayToggled(bool checked)
{
    if(checked) {
        timer->start();
    } else {
        timer->stop();
    }
}

void MainWindow::deleteGrids()
{
    if (m_grid_mono) {
        delete m_grid_mono;
        m_grid_mono = nullptr;
    }
    if (m_grid_multi) {
        delete m_grid_multi;
        m_grid_multi = nullptr;
    }
}
