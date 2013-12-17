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

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->actionRun, SIGNAL(triggered()), this, SLOT(actionRun()));


    std::vector<real> boundaries = {x0, x1};
    node_tp root = node_t::createRoot(boundaries);

    // create children in memory
    root->unpack(node_t::level_t(level));

    size_t count_nodes = 0;
    std::for_each(node_iterator(root->boundary(node_t::posLeft)), node_iterator(), [&](node_base &node) {
        ++count_nodes;
        node.m_property = f_eval5(node.center());
    });

    for(size_t timestep = 0; timestep < 1; ++timestep) {
        root->isActive();
        root->cleanUp();
//        root->flow();
    }

    size_t count_nodes_packed = 0;

    // output file
    QVector<real> xvalues, yvalues;
    QVector<real> lvlvalues, lvlvirtualvalues;
    std::for_each(node_iterator(root->boundary(node_t::posLeft)), node_iterator(), [&](node_base &node) {
        xvalues.push_back(node.center());
        yvalues.push_back(node.property());
        lvlvalues.push_back(node.active() ? ((node.level() > node_t::lvlRoot) ? (pow(2,-node.level())) : 1) : 0);
        lvlvirtualvalues.push_back(!node.active() ? ((node.level() > node_t::lvlRoot) ? (pow(2,-node.level())) : 1) : 0);
        ++count_nodes_packed;
        std::cout << node << std::endl;
    });

    QCustomPlot *customPlot = ui->customPlot;

    customPlot->addGraph();
    customPlot->graph(0)->setData(xvalues, yvalues);
    // give the axes some labels:
    customPlot->xAxis->setLabel("x");
    customPlot->yAxis->setLabel("y");
    // set axes ranges, so we see all data:
    customPlot->xAxis->setRange(x0, x1);
    customPlot->yAxis->setRange(0, 1);
    customPlot->replot();

    QCPBars *bars = new QCPBars(customPlot->xAxis, customPlot->yAxis);
    customPlot->addPlottable(bars);
    bars->setName("Nonvirtual Elements");
    bars->setBrush(QBrush(Qt::blue));
    bars->setPen(QPen(Qt::transparent));
    bars->setWidth(0.005);
    bars->setData(xvalues, lvlvalues);
    QCPBars *vbars = new QCPBars(customPlot->xAxis, customPlot->yAxis);
    customPlot->addPlottable(vbars);
    vbars->setName("Virtual Elements");
    vbars->setBrush(QBrush(Qt::red));
    vbars->setPen(QPen(Qt::transparent));
    vbars->setWidth(0.005);
    vbars->setData(xvalues, lvlvirtualvalues);

    customPlot->rescaleAxes();
    customPlot->replot();


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::actionRun()
{
    qDebug() << "triggered";
}
