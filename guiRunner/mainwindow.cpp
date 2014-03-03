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
{
    ui->setupUi(this);
    // ui->splitter->setSizes(QList<int>() << 100 << 100);

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

    N = (1 << g_level);
    N2 = N*N;

    colorMap = new QCPColorMap(customPlot->xAxis, customPlot->yAxis);
    customPlot->addPlottable(colorMap);

    colorMap->data()->setSize(N, N);

    colorMap->data()->setRange(QCPRange(g_x0[dimX], g_x1[dimX]),
                               QCPRange(g_x0[dimY], g_x1[dimY]));
    colorMap->setDataRange(QCPRange(0, 1));
    /*
    QCPColorScale *colorScale = new QCPColorScale(customPlot);
    customPlot->plotLayout()->addElement(0, 1, colorScale);
    colorMap->setColorScale(colorScale);
    */
    colorMap->setGradient(QCPColorGradient::gpThermal);


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

void MainWindow::initializeGrids()
{
    deleteGrids();

    m_grid_mono  = new monores_grid_t(g_level);
    // m_grid_multi = new multires_grid_t(g_level);

    replot();
}

void MainWindow::actionRun()
{
    int stepAtOnce = spinBox->value();
    real timeInterval = g_timestep*stepAtOnce;
    if(m_grid_mono) {
        real runTime = 0;
        size_t counter = 0;
        do {
            runTime += m_grid_mono->timeStep();
            counter++;
        } while(runTime < timeInterval);
        qDebug() << "counter:" << counter++;
    }

    /*
    if(m_grid_mono) {
        size_t counter = 0;
        do {
            m_grid_mono->timeStep();
            counter++;
        } while(m_grid_mono->getTime() < m_grid_multi->getTime());
        qDebug() << "counter_regular:" << counter++;
    }
    */

    replot();
}

void MainWindow::replot()
{
    for (const point_t &point: *m_grid_mono) {
        colorMap->data()->setCell(point.m_index[dimX], point.m_index[dimY], point.m_phi);
    }
    // colorMap->rescaleDataRange(true);
    // customPlot->rescaleAxes();
    customPlot->replot();

    // statistics
    /*
    count_nodes_packed = m_grid_multi->size();
    QString pack_rate_time = QString("pack rate: %1/%2 = %3, time: %4").arg(count_nodes_packed).arg(N2).arg(real(count_nodes_packed)/N2).arg(m_grid_multi->getTime());
    qDebug() << pack_rate_time;
    statusBar()->showMessage(pack_rate_time);
    */
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
