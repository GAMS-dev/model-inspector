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
#include "gamsprocess.h"
#include "commonpaths.h"

#include <QDebug>
#include <QDir>

#ifdef _WIN32
#include "windows.h"
#endif

GAMSProcess::GAMSProcess(QObject *parent)
    : QObject(parent)
    , mProcess(this)
    , mAppName("gams")
{
    connect(&mProcess, &QProcess::readyReadStandardOutput,
            this, &GAMSProcess::readStdOut);
    connect(&mProcess, &QProcess::readyReadStandardError,
            this, &GAMSProcess::readStdErr);
}

void GAMSProcess::execute()
{
    mProcess.setWorkingDirectory(mWorkingDir);

    QStringList args { "\""+mModel+"\"" };
    args << mParameters;
#if defined(__unix__) || defined(__APPLE__)
    mProcess.start(nativeAppPath(), args);
#else
    mProcess.setNativeArguments(args.join(" "));
    mProcess.setProgram(nativeAppPath());
    mProcess.start();
#endif
}

void GAMSProcess::setWorkingDir(const QString &workingDir)
{
    mWorkingDir = workingDir;
}

QString GAMSProcess::workingDir() const
{
    return mWorkingDir;
}

void GAMSProcess::interrupt()
{
    QString pid = QString::number(mProcess.processId());
#ifdef _WIN32
    //IntPtr receiver;
    COPYDATASTRUCT cds;
    const char* msgText = "GAMS Message Interrupt";

    QString windowName("___GAMSMSGWINDOW___");
    windowName += pid;
    HWND receiver = FindWindowA(nullptr, windowName.toUtf8().constData());

    cds.dwData = (ULONG_PTR) 1;
    cds.lpData = (PVOID) msgText;
    cds.cbData = (DWORD) (strlen(msgText) + 1);

    SendMessageA(receiver, WM_COPYDATA, 0, (LPARAM)(LPVOID)&cds);
#else // Linux and Mac OS X
    QProcess proc;
    proc.setProgram("/bin/bash");
    QStringList args { "-c", "kill -2 " + pid};
    proc.setArguments(args);
    proc.start();
    proc.waitForFinished(-1);
#endif
}

void GAMSProcess::stop()
{
    mProcess.kill();
}

void GAMSProcess::setModel(const QString &model)
{
    mModel = model;
}

void GAMSProcess::setParameters(const QStringList &parameters)
{
    mParameters = parameters;
}

QProcess* GAMSProcess::process()
{
    return &mProcess;
}

QString GAMSProcess::nativeAppPath()
{
    QString systemDir = CommonPaths::systemDir();
    if (systemDir.isEmpty())
        return QString();
    auto appPath = QDir(systemDir).filePath(mAppName);
    return QDir::toNativeSeparators(appPath);
}

void GAMSProcess::readStdChannel(QProcess::ProcessChannel channel)
{
    mOutputMutex.lock();
    mProcess.setReadChannel(channel);
    bool avail = mProcess.bytesAvailable();
    mOutputMutex.unlock();

    while (avail) {
        mOutputMutex.lock();
        mProcess.setReadChannel(channel);
        emit newStdChannelData(mProcess.readLine().constData());
        avail = mProcess.bytesAvailable();
        mOutputMutex.unlock();
    }
}

void GAMSProcess::readStdOut()
{
    readStdChannel(QProcess::StandardOutput);
}

void GAMSProcess::readStdErr()
{
    readStdChannel(QProcess::StandardError);
}
