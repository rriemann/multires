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

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

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
    connect(ui->actionInitializeRoot, SIGNAL(triggered()), this, SLOT(initializeRoot()));

    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    qDebug();
    qDebug() << QString("max level: %1").arg(g_level);

    customPlot->addGraph();
    customPlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 3));
    // give the axes some labels:
    customPlot->xAxis->setLabel("x");
    customPlot->yAxis->setLabel("y");
    // set axes ranges, so we see all data:
    customPlot->xAxis->setRange(x0, x1);
    customPlot->yAxis->setRange(-0.2, 1.2);

    customPlot->graph(0)->setPen(QPen(Qt::green));

    for(size_t i = 0; i < bars.size() ; ++i) {
        bars[i] = new QCPBars(customPlot->xAxis, customPlot->yAxis);
        customPlot->addPlottable(bars[i]);
        bars[i]->setPen(QPen(Qt::transparent));
        bars[i]->setWidth(0.005);
    }
    bars[0]->setName("Active Elements");
    bars[0]->setBrush(QBrush(Qt::blue));

    bars[1]->setName("Virtual Elements");
    bars[1]->setBrush(QBrush(Qt::red));

    bars[2]->setName("Savety Zone");
    bars[2]->setBrush(QBrush(Qt::yellow));

    bars[3]->setName("Static Elements");
    bars[3]->setBrush(QBrush(Qt::black));

    initializeRoot();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initializeRoot()
{
    std::vector<real> boundaries {x0, x1};
    root = node_t::createRoot(boundaries, f_eval_gauss, node_t::level_t(g_level), node_t::bcPeriodic);

    // it is not clear if this gives the right result
    count_nodes = std::distance(node_iterator(root->boundary(node_t::posLeft)), node_iterator(root->boundary(node_t::posRight)));

    root->optimizeTree();

    replot();
}

void MainWindow::actionRun()
{
    if(root) {
        int stepAtOnce = spinBox->value();
        for(int i = 0; i < stepAtOnce; ++i) {
            root->timeStep();
        }
        replot();
    }
}

void MainWindow::replot()
{
    count_nodes_packed = 0;

    QVector<real> xvalues, yvalues;
    QVector<real> lvlvalues, lvlvirtualvalues, lvlsavetyvalues, lvlstaticvalues;
    std::for_each(node_iterator(root->boundary(node_t::posLeft)), node_iterator(), [&](node_base &node) {
        xvalues.push_back(node.center(node_t::dimX));
        yvalues.push_back(node.property());

        if(node.level() > node_t::lvlRoot) {
            real bar = pow(2,-node.level());
            if(node.is(node_t::typeActive)) {
                lvlvalues       .push_back(bar);
                lvlsavetyvalues .push_back(0);
                lvlvirtualvalues.push_back(0);
            } else if(node.is(node_t::typeSavetyZone)) {
                lvlvalues       .push_back(0);
                lvlsavetyvalues .push_back(bar);
                lvlvirtualvalues.push_back(0);
            } else if(node.is(node_t::typeVirtual)) {
                lvlvalues       .push_back(0);
                lvlsavetyvalues .push_back(0);
                lvlvirtualvalues.push_back(bar);
            }

            lvlstaticvalues.push_back(0);
        } else {
            lvlvalues       .push_back(0);
            lvlsavetyvalues .push_back(0);
            lvlvirtualvalues.push_back(0);

            lvlstaticvalues .push_back(1);
        }
        ++count_nodes_packed;
    });

    qDebug() << QString("pack rate: %1/%2 = %3").arg(count_nodes_packed).arg(count_nodes).arg(real(count_nodes_packed)/count_nodes);

    customPlot->graph(0)->setData(xvalues, yvalues);
    bars[0]->setData(xvalues, lvlvalues); // blue
    bars[1]->setData(xvalues, lvlvirtualvalues); // red
    bars[2]->setData(xvalues, lvlsavetyvalues); // yellow
    bars[3]->setData(xvalues, lvlstaticvalues); // black

    customPlot->rescaleAxes();
    customPlot->replot();
}

void MainWindow::autoPlayToggled(bool checked)
{
    if(checked) {
        timer->start();
    } else {
        timer->stop();
    }
}
