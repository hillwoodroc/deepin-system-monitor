/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2011 ~ 2017 Deepin, Inc.
 *               2011 ~ 2017 Wang Yong
 *
 * Author:     Wang Yong <wangyong@deepin.com>
 * Maintainer: Wang Yong <wangyong@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "constant.h"
#include "dthememanager.h"
#include "compact_cpu_monitor.h"
#include "smooth_curve_generator.h"
#include "utils.h"
#include <QDebug>
#include <QPainter>
#include <QApplication>
#include <DHiDPIHelper>

DWIDGET_USE_NAMESPACE

using namespace Utils;

CompactCpuMonitor::CompactCpuMonitor(QWidget *parent) : QWidget(parent)
{
    iconDarkImage = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("icon_cpu_dark.svg"));
    iconLightImage = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("icon_cpu_light.svg"));

    initTheme();

    connect(DThemeManager::instance(), &DThemeManager::themeChanged, this, &CompactCpuMonitor::changeTheme);

    int statusBarMaxWidth = Utils::getStatusBarMaxWidth();
    setFixedWidth(statusBarMaxWidth);
    setFixedHeight(160);

    pointsNumber = int(statusBarMaxWidth / 5.4);

    readSpeeds = new QList<double>();
    for (int i = 0; i < pointsNumber; i++) {
        readSpeeds->append(0);
    }
}

CompactCpuMonitor::~CompactCpuMonitor()
{
    delete readSpeeds;
}

void CompactCpuMonitor::initTheme()
{
    if (DThemeManager::instance()->theme() == "light") {
        textColor = "#303030";
        summaryColor = "#505050";

        iconImage = iconLightImage;
    } else {
        textColor = "#ffffff";
        summaryColor = "#909090";

        iconImage = iconDarkImage;
    }
}

void CompactCpuMonitor::changeTheme(QString )
{
    initTheme();
}

void CompactCpuMonitor::updateStatus(double tReadKbs, std::vector<double> cPercents)
{
    qDebug() << "--------------";
    for (unsigned int i = 0; i < cPercents.size(); i++) {
        qDebug() << cPercents[i];
    }
    
    totalReadKbs = tReadKbs;

    // Init read path.
    readSpeeds->append(totalReadKbs);

    if (readSpeeds->size() > pointsNumber) {
        readSpeeds->pop_front();
    }

    QList<QPointF> readPoints;

    double readMaxHeight = 0;
    for (int i = 0; i < readSpeeds->size(); i++) {
        if (readSpeeds->at(i) > readMaxHeight) {
            readMaxHeight = readSpeeds->at(i);
        }
    }

    for (int i = 0; i < readSpeeds->size(); i++) {
        if (readMaxHeight < readRenderMaxHeight) {
            readPoints.append(QPointF(i * 5, readSpeeds->at(i)));
        } else {
            readPoints.append(QPointF(i * 5, readSpeeds->at(i) * readRenderMaxHeight / readMaxHeight));
        }
    }

    readPath = SmoothCurveGenerator::generateSmoothCurve(readPoints);

    repaint();
}

void CompactCpuMonitor::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Draw background grid.
    painter.setRenderHint(QPainter::Antialiasing, false);
    QPen framePen;
    painter.setOpacity(0.1);
    framePen.setColor(QColor(textColor));
    framePen.setWidth(0.5);
    painter.setPen(framePen);

    int penSize = 1;
    int gridX = rect().x() + penSize;
    int gridY = rect().y() + gridRenderOffsetY + gridPaddingTop;
    int gridWidth = rect().width() - gridPaddingRight - penSize * 2;
    int gridHeight = readRenderMaxHeight + waveformRenderPadding;

    QPainterPath framePath;
    framePath.addRect(QRect(gridX, gridY, gridWidth, gridHeight));
    painter.drawPath(framePath);

    // Draw grid.
    QPen gridPen;
    QVector<qreal> dashes;
    qreal space = 3;
    dashes << 5 << space;
    painter.setOpacity(0.05);
    gridPen.setColor(QColor(textColor));
    gridPen.setWidth(0.5);
    gridPen.setDashPattern(dashes);
    painter.setPen(gridPen);

    int gridLineX = gridX;
    while (gridLineX < gridX + gridWidth - gridSize) {
        gridLineX += gridSize;
        painter.drawLine(gridLineX, gridY + 1, gridLineX, gridY + gridHeight - 1);
    }
    int gridLineY = gridY;
    while (gridLineY < gridY + gridHeight - gridSize) {
        gridLineY += gridSize;
        painter.drawLine(gridX + 1, gridLineY, gridX + gridWidth - 1, gridLineY);
    }
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.setOpacity(1);
    painter.translate((rect().width() - pointsNumber * 5) / 2 - 7, readWaveformsRenderOffsetY + gridPaddingTop);
    painter.scale(1, -1);

    qreal devicePixelRatio = qApp->devicePixelRatio();
    qreal diskCurveWidth = 1.6;
    if (devicePixelRatio > 1) {
        diskCurveWidth = 2;
    }
    painter.setPen(QPen(QColor(readColor), diskCurveWidth));
    painter.setBrush(QBrush());
    painter.drawPath(readPath);
}