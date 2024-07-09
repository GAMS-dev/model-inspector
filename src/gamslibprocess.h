/**
 * GAMS Model Instance Inspector (MII)
 *
 * Copyright (c) 2023-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2023-2024 GAMS Development Corp. <support@gams.com>
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef GAMSLIBPROCESS_H
#define GAMSLIBPROCESS_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <QMutex>

class GAMSLibProcess
        : public QObject
{
    Q_OBJECT

public:
    GAMSLibProcess(QObject *parent = Q_NULLPTR);

    void setTargetDir(const QString &targetDir);
    QString targetDir() const;

    void setModelNumber(int modelNumber);
    int modelNumber() const;

    void setModelName(const QString &modelName);
    QString modelName() const;

    void execute();

    QProcess* process();

signals:
    void newStdChannelData(const QByteArray &data);

private:
    QString nativeAppPath();

    void readStdChannel(QProcess::ProcessChannel channel);
    void readStdOut();
    void readStdErr();

private:
    QProcess mProcess;
    QString mAppName;
    QString mTargetDir;
    int mModelNumber = -1;
    QString mModelName;

    QMutex mOutputMutex;
};

#endif // GAMSLIBPROCESS_H
