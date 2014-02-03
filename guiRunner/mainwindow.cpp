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

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    scene(new QGraphicsScene)
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
    connect(ui->actionInitializeRoot, SIGNAL(triggered()), this, SLOT(initializeRoot()));
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
    customPlot->xAxis->setRange(x0, x1);
    customPlot->yAxis->setRange(-0.2, 1.2);

    customPlot->graph(0)->setPen(QPen(Qt::black));

    customPlot->addGraph();
    customPlot->graph(1)->setPen(QPen(Qt::green));

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
    rescale();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    ui->graphicsView->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
    QMainWindow::resizeEvent(event);
}

void MainWindow::initializeRoot()
{
    std::vector<real> boundaries {x0, x1};
    root = node_t::createRoot(boundaries, f_eval_triangle, node_t::level_t(g_level), node_t::bcPeriodic);

    // it is not clear if this gives the right result
    count_nodes = (1 << g_level)+1;

    root->optimizeTree();

    replot();
}

void MainWindow::actionRun()
{
    if(root) {
        int stepAtOnce = spinBox->value();
        real timeInterval = g_timestep*stepAtOnce;
        real runTime = 0;
        size_t counter = 0;
        do {
            runTime += root->timeStep();
            counter++;
        } while(runTime < timeInterval);
        qDebug() << "counter:" << counter++;

        replot();
    }
}

void MainWindow::replot()
{
    count_nodes_packed = 0;

    QVector<real> xvalues, yvalues, yvaluestheory;
    QVector<real> lvlvalues, lvlvirtualvalues, lvlsavetyvalues, lvlstaticvalues;
    std::for_each(node_iterator(root->boundary(node_t::posLeft)), node_iterator(), [&](node_base &node) {
        xvalues.push_back(node.center(node_t::dimX));
        yvalues.push_back(node.property());
#ifndef BURGERS
        yvaluestheory.push_back(node.propertyTheory());
#endif
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

    QString pack_rate = QString("pack rate: %1/%2 = %3").arg(count_nodes_packed).arg(count_nodes).arg(real(count_nodes_packed)/count_nodes);
    qDebug() << pack_rate;
    statusBar()->showMessage(pack_rate);

    customPlot->graph(0)->setData(xvalues, yvalues); // black
#ifndef BURGERS
    customPlot->graph(1)->setData(xvalues, yvaluestheory); // green
#endif
    bars[0]->setData(xvalues, lvlvalues); // blue
    bars[1]->setData(xvalues, lvlvirtualvalues); // red
    bars[2]->setData(xvalues, lvlsavetyvalues); // yellow
    bars[3]->setData(xvalues, lvlstaticvalues); // black

    customPlot->replot();

    scene->clear();
    blockBuilder(root);
    ui->graphicsView->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
}

void MainWindow::blockBuilder(node_base::node_p node)
{
    const static qreal height = 20;
    const qreal stretchX = 300;
    qreal width = stretchX/(1 << node->level());
    qreal center = node->center(node_t::dimX)*stretchX/2;
    qreal bottom = node->level()*height;
    qreal x = center-0.5*width;

    static QBrush brush(Qt::SolidPattern);
    if(node->isActive()) {
        brush.setColor(Qt::blue);
    } else if(node->isSavetyZone()) {
        brush.setColor(Qt::yellow);
    } else if(node->isVirtual()) {
        brush.setColor(Qt::red);
    } else {
        abort();
    }
    const QPen pen(Qt::transparent);
    scene->addRect(x, -bottom, width, height, pen, brush);
    scene->addLine(x, -bottom+height, x + width, -bottom+height, QPen(Qt::black));
    scene->addLine(center, -bottom+height, center+0.25*width, -bottom, QPen(Qt::black));
    scene->addLine(center, -bottom+height, center-0.25*width, -bottom, QPen(Qt::black));

    for(node_t::node_u const &child : node->childs()) {
        if(child) {
           blockBuilder(child.get());
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
