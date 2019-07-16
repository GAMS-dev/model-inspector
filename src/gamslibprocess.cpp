/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "gamslibprocess.h"
#include "commonpaths.h"

#include <QDebug>
#include <QDir>

GAMSLibProcess::GAMSLibProcess(QObject *parent)
    : QObject(parent),
      mAppName("gamslib")
{

}

void GAMSLibProcess::setTargetDir(const QString &targetDir)
{
    mTargetDir = targetDir;
}

QString GAMSLibProcess::targetDir() const
{
    return mTargetDir;
}

void GAMSLibProcess::setModelNumber(int modelNumber)
{
    mModelNumber = modelNumber;
}

int GAMSLibProcess::modelNumber() const
{
    return mModelNumber;
}

void GAMSLibProcess::setModelName(const QString &modelName)
{
    mModelName = modelName;
}

QString GAMSLibProcess::modelName() const
{
    return mModelName;
}

void GAMSLibProcess::execute()
{
    QStringList args;
    args << "-lib";
    args << QDir::toNativeSeparators(mGlbFile);
    args << (mModelName.isEmpty() ? QString::number(mModelNumber) : mModelName);
    args << QDir::toNativeSeparators(mTargetDir);
    qDebug() << "model name >> " << mModelName;
    qDebug() << "model dir  >> " << mTargetDir;
    mProcess.start(nativeAppPath(), args);
}

void GAMSLibProcess::setGlbFile(const QString &glbFile)
{
    mGlbFile = glbFile;
}

void GAMSLibProcess::printOutputToDebug()
{
    if (mProcess.waitForFinished()) {
        qDebug() << mProcess.readAllStandardOutput();
    }
}

QString GAMSLibProcess::nativeAppPath()
{
    QString systemDir = CommonPaths::systemDir();
    if (systemDir.isEmpty())
        return QString();
    auto appPath = QDir(systemDir).filePath(mAppName);
    return QDir::toNativeSeparators(appPath);
}
