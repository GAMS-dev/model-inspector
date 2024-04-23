/**
 * GAMS Model Instance Inspector (MII)
 *
 * Copyright (c) 2023 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2023 GAMS Development Corp. <support@gams.com>
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
 *
 */
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "commonpaths.h"
#include "gamsprocess.h"
#include "gamslibprocess.h"
#include "mii/filterdialog.h"
#include "mii/modelinspector.h"
#include "mii/searchresultmodel.h"
#include "mii/common.h"
#include "mii/viewconfigurationprovider.h"

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>

#include <QDebug>

using namespace gams::studio;
using gams::studio::mii::FilterDialog;
using gams::studio::mii::ModelInspector;
using gams::studio::mii::SearchResultModel;
using gams::studio::mii::ViewHelper;
using gams::studio::mii::CmdParser;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , mLibProcess(new GAMSLibProcess(this))
    , mProcess(new GAMSProcess(this))
    , mFilterDialog(new FilterDialog(this))
    , mScrWatcher(this)
{
    ui->setupUi(this);
    ui->modelInspector->setWorkspace(workspace());
    ui->modelInspector->setSystemDirectory(CommonPaths::systemDir());
    ui->paramsEdit->setText(QString("MIIMode=singleMI scrdir=%1/scratch").arg(workspace()));
    ui->searchResultView->setModel(new SearchResultModel(ui->searchResultView));
    ui->actionAggregation->setEnabled(false);
    setWindowTitle(windowTitle() + " " + QApplication::applicationVersion());
    setupConnections();
    setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
    createProjectDirectory();
    ui->actionZoom_In->setShortcut(QKeySequence::ZoomIn);
    ui->actionZoom_Out->setShortcut(QKeySequence::ZoomOut);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    ui->modelInspector->cancelRun();
    QMainWindow::closeEvent(event);
}

void MainWindow::appendLogMessage(const QString &message)
{
    ui->logEdit->appendPlainText(message.trimmed());
}

void MainWindow::on_actionOpen_triggered()
{
    QString workingDir = workspace();
    QFileInfo current(ui->modelEdit->text());
    if (ui->modelEdit->text() != current.fileName()) {
        auto dir = current.absoluteDir();
        if (dir.exists())
            workingDir = dir.cleanPath(dir.absolutePath());
    }
    auto fileName = QFileDialog::getOpenFileName(this,
                                                 tr("Open Model File"),
                                                 workingDir,
                                                 tr("GAMS source (*.gms *.dmp *.dat)"));
    if (fileName.isEmpty())
        return;
    ui->modelEdit->setText(fileName);
    ui->gamslibCheckBox->setChecked(false);
    QFileInfo fi(ui->modelEdit->text());
    ui->logEdit->appendPlainText("Loading scratch data from: " + fi.dir().path());
    if (ui->modelEdit->text().endsWith(".dat")) {
        auto dir = fi.dir().path();
        ui->paramsEdit->setText(QString("MIIMode=singleMI scrdir=%1").arg(dir));
    }
}

void MainWindow::on_actionRun_triggered()
{
    setRunButtonState(false);
    auto path = workspace();
    QDir dir(path);
    if (!dir.mkpath(path)) {
        ui->logEdit->appendPlainText("Error: Could not create workspace " + path);
        setRunButtonState(true);
        return;
    }
    mLoadScrFiles = false;
    mScrFilesUpdated = false;
    ui->modelInspector->setModelFilePath(ui->modelEdit->text());
    ui->modelInspector->setShowOutput(ui->actionShow_Output->isChecked());
    if (ui->modelEdit->text().endsWith(".dat")) {
        mLoadScrFiles = true;
        QFileInfo fi(ui->modelEdit->text());
        auto dir = fi.dir().path();
        ui->modelInspector->setWorkspace(dir + "/..");
        ui->modelInspector->setScratchDir(dir);
        ui->modelInspector->setBaseScratchDir(dir);
        loadModelInstance(0, QProcess::NormalExit);
    } else {
        CmdParser cmdParser;
        cmdParser.parse(ui->paramsEdit->text());
        if (!cmdParser.scratchDir().isEmpty()) {
            QDir dir(cmdParser.scratchDir());
            dir.removeRecursively();
            dir.mkdir(dir.absolutePath());
            ui->modelInspector->setScratchDir(cmdParser.scratchDir());
            ui->modelInspector->setBaseScratchDir(cmdParser.scratchDir());
            updateScratchDataWatcher(cmdParser.scratchDir());
        } else {
            ui->logEdit->appendPlainText("Error: No scrach direcetory specified.");
            setRunButtonState(true);
            return;
        }
        if (ui->gamslibCheckBox->isChecked()) {
            loadGAMSModel(path);
        } else {
            auto fp = ui->modelEdit->text();
            QFileInfo fi(fp);
            if (fp.indexOf(fi.fileName()) > 0) {
                auto dir = fi.absoluteDir();
                if (dir.exists()) {
                    path = dir.absolutePath();
                }
            }
        }
        mProcess->setModel(ui->modelEdit->text());
        mProcess->setParameters(cmdParser.parameters());
        mProcess->setWorkingDir(path);
        mProcess->execute();
    }
}

void MainWindow::on_action_Quit_triggered()
{
    close();
}

void MainWindow::on_action_Search_triggered()
{
    ui->searchEdit->setFocus();
}

void MainWindow::searchHeaders()
{
    const auto& result = ui->modelInspector->searchHeaders(ui->searchEdit->text(),
                                                           ui->searchRegexBox->isChecked());
    static_cast<SearchResultModel*>(ui->searchResultView->model())->updateData(result);
    ui->searchResultView->resizeColumnsToContents();
    ui->searchResultView->resizeRowsToContents();
}

void MainWindow::editMenuAboutToShow()
{
    auto states = ui->modelInspector->viewActionStates();
    ui->actionSaveView->setEnabled(states.SaveEnabled);
    ui->actionRemoveView->setEnabled(states.RemoveEnabled);
    ui->actionRenameView->setEnabled(states.RenameEnabled);
}

void MainWindow::on_actionFilters_triggered()
{
    setGlobalFiltersData();
    showDialog(mFilterDialog);
}

void MainWindow::on_actionShow_search_result_triggered()
{
    ui->dockWidget->show();
}

void MainWindow::showAbsoluteValues()
{
    ui->modelInspector->setShowAbsoluteValuesGlobal(ui->actionShow_Absolute->isChecked());
    setGlobalFiltersData();
}

void MainWindow::on_actionShow_Output_triggered()
{
    ui->modelInspector->setShowOutput(ui->actionShow_Output->isChecked());
    ui->modelInspector->setShowAbsoluteValuesGlobal(ui->actionShow_Absolute->isChecked());
    ui->modelInspector->reloadModelInstance();
}

void MainWindow::on_actionZoom_In_triggered()
{
    ui->logEdit->zoomIn(2);
    ui->searchResultView->zoomIn(2);
    ui->modelInspector->zoomIn();
}

void MainWindow::on_actionZoom_Out_triggered()
{
    ui->logEdit->zoomOut(2);
    ui->searchResultView->zoomOut(2);
    ui->modelInspector->zoomOut();
}

void MainWindow::on_actionZoom_Reset_triggered()
{
    ui->modelInspector->resetZoom();
    ui->searchResultView->resetZoom();
    ui->logEdit->resetZoom();
}

void MainWindow::on_actionAbout_Model_Inspector_triggered()
{
    QMessageBox about(this);
    about.setWindowTitle(tr("GAMS Model Inspector"));
    about.setTextFormat(Qt::RichText);
    about.setText("<b><big>GAMS Model Inspector " + QApplication::applicationVersion() + "</big></b>");
    about.setInformativeText(aboutModelInspector());
    about.setIconPixmap(QIcon(":/img/miilogo").pixmap(QSize(64, 64)));
    about.addButton(QMessageBox::Ok);
    about.exec();
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    qApp->aboutQt();
}

void MainWindow::loadModelInstance(int exitCode, QProcess::ExitStatus exitStatus)
{
    auto miiMode = CmdParser::miiMode(ui->paramsEdit->text());
    ui->modelInspector->setMiiMode(miiMode);
    switch (miiMode) {
    case ViewHelper::MiiModeType::Single:
        loadSingleModelInstance(exitCode, exitStatus);
        break;
    case ViewHelper::MiiModeType::Multi:
        loadMultiModelInstance(exitCode, exitStatus);
        break;
    default:
        ui->logEdit->appendPlainText("Error: Wrong MiiMode parameter: " + ui->paramsEdit->text());
        break;
    }
}

void MainWindow::handleLibProcessResult(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus);
    if (exitCode != 0) {
        setRunButtonState(true);
    }
}

void MainWindow::viewConfigUpdate()
{
    static_cast<SearchResultModel*>(ui->searchResultView->model())->updateData({});
    ui->modelInspector->updateFilters();
}

void MainWindow::searchResultSelectionChanged(const QModelIndex &index)
{
    if (!index.isValid())
        return;
    auto entry = static_cast<SearchResultModel*>(ui->searchResultView->model())->entry(index.row());
    ui->modelInspector->setSearchSelection(entry);
}

void MainWindow::updateModelInstance()
{
    static_cast<SearchResultModel*>(ui->searchResultView->model())->updateData({});
}

void MainWindow::viewChanged(int viewType)
{
    if (viewType == (int)ViewHelper::ViewDataType::Unknown) {
        ui->action_Search->setEnabled(false);
        ui->searchEdit->setEnabled(false);
        ui->actionFilters->setEnabled(false);
    } else {
        ui->action_Search->setEnabled(true);
        ui->searchEdit->setEnabled(true);
        ui->actionFilters->setEnabled(true);
        static_cast<SearchResultModel*>(ui->searchResultView->model())->updateData({});
        setGlobalFiltersData();
    }
    const auto& searchResult = ui->modelInspector->searchResult();
    static_cast<SearchResultModel*>(ui->searchResultView->model())->updateData(searchResult);
    ui->searchEdit->setText(searchResult.Term);
    ui->searchRegexBox->setChecked(searchResult.IsRegEx);
    ui->searchResultView->resizeColumnsToContents();
    ui->searchResultView->resizeRowsToContents();
}

void MainWindow::scrDirectoryChanged()
{
    mScrFilesUpdated = true;
}

void MainWindow::scrFileChanged()
{
    mScrFilesUpdated = true;
}

void MainWindow::setupConnections()
{
    connect(mProcess->process(), QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &MainWindow::loadModelInstance);
    connect(ui->modelInspector, &ModelInspector::newLogMessage,
            this, &MainWindow::appendLogMessage);
    connect(mLibProcess, &GAMSLibProcess::newStdChannelData,
            this, &MainWindow::appendLogMessage);
    connect(mLibProcess->process(), QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &MainWindow::handleLibProcessResult);
    connect(mProcess.get(), &GAMSProcess::newStdChannelData,
            this, &MainWindow::appendLogMessage);
    connect(ui->searchEdit, &QLineEdit::returnPressed,
            this, &MainWindow::searchHeaders);
    connect(ui->openButton, &QPushButton::clicked,
            this, &MainWindow::on_actionOpen_triggered);
    connect(ui->runButton, &QPushButton::clicked,
            this, &MainWindow::on_actionRun_triggered);
    connect(mFilterDialog, &FilterDialog::viewConfigUpdated,
            this, &MainWindow::viewConfigUpdate);
    connect(ui->modelInspector, &ModelInspector::viewChanged,
            this, &MainWindow::viewChanged);
    connect(ui->modelInspector, &ModelInspector::filtersChanged,
            this, [this]{
        static_cast<SearchResultModel*>(ui->searchResultView->model())->updateData({});
        setGlobalFiltersData();
    });
    connect(ui->searchResultView, &QTableView::doubleClicked,
            this, &MainWindow::searchResultSelectionChanged);
    connect(ui->actionSaveView, &QAction::triggered,
            ui->modelInspector, &ModelInspector::saveModelView);
    connect(ui->actionRemoveView, &QAction::triggered,
            ui->modelInspector, &ModelInspector::removeModelView);
    connect(ui->menu_Edit, &QMenu::aboutToShow,
            this, &MainWindow::editMenuAboutToShow);
    connect(ui->actionShow_Absolute, &QAction::triggered,
            this, &MainWindow::showAbsoluteValues);
    connect(ui->paramsEdit, &QLineEdit::returnPressed,
            this, &MainWindow::on_actionRun_triggered);
    connect(ui->modelEdit, &QLineEdit::returnPressed,
            this, &MainWindow::on_actionRun_triggered);
    connect(&mScrWatcher, &QFileSystemWatcher::directoryChanged,
            this, &MainWindow::scrDirectoryChanged);
    connect(&mScrWatcher, &QFileSystemWatcher::fileChanged,
            this, &MainWindow::scrFileChanged);
    connect(ui->modelInspector, &ModelInspector::dataLoaded,
            this, [this]{
        setRunButtonState(true);
    });
    connect(ui->modelInspector, &ModelInspector::openFilterDialog,
            this, &MainWindow::on_actionFilters_triggered);
}

void MainWindow::createProjectDirectory()
{
    auto path = projectDirectory();
    QDir dir(path);
    if (!dir.mkpath(path))
        ui->logEdit->appendPlainText("Error: Could not create project directory " + path);
}

QString MainWindow::aboutModelInspector() const
{
    QString about = "Release: GAMS Model Inspector " + QApplication::applicationVersion() + " ";
    about += QString(sizeof(void*)==8 ? "64" : "32") + " bit<br/>";
    about += "Build Date: " __DATE__ " " __TIME__ "<br/><br/>";
    about += "Copyright (c) 2023 GAMS Software GmbH <support@gams.com><br/>";
    about += "Copyright (c) 2023 GAMS Development Corp. <support@gams.com><br/><br/>";
    about += "This program is free software: you can redistribute it and/or modify ";
    about += "it under the terms of the GNU General Public License as published by ";
    about += "the Free Software Foundation, either version 3 of the License, or ";
    about += "(at your option) any later version.<br/><br/>";
    about += "This program is distributed in the hope that it will be useful, ";
    about += "but WITHOUT ANY WARRANTY; without even the implied warranty of ";
    about += "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the ";
    about += "GNU General Public License for more details.<br/><br/>";
    about += "You should have received a copy of the GNU General Public License ";
    about += "along with this program. If not, see ";
    about += "<a href=\"http://www.gnu.org/licenses/\">http://www.gnu.org/licenses/</a>.<br/><br/>";
    about += "The source code of the program can be accessed at ";
    about += "<a href=\"https://github.com/GAMS-dev/model-inspector\">https://github.com/GAMS-dev/model-inspector</a></p>.";
    return about;
}

void MainWindow::loadSingleModelInstance(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus);
    if (exitCode != 0) {
        appendLogMessage("The GAMSProcess reported an issue. The exit code is " +
                         QString().number(exitCode));
        setRunButtonState(true);
        return;
    }
    if (!mLoadScrFiles) {
        if (mScrFilesUpdated) {
            ui->logEdit->appendPlainText("The scratch directory files have been updated.");
        } else {
            ui->logEdit->appendPlainText(mScrUpdateWarning);
        }
    }
    QDir scrdir(ui->modelInspector->scratchDir());
    if (scrdir.count() == 0) {
        ui->logEdit->appendPlainText("Error: No scratch files found at " + ui->modelInspector->scratchDir());
        setRunButtonState(true);
        return;
    }
    int datFileCount = 0;
    for (const auto& file : scrdir.entryList()) {
        if (file.endsWith(mii::FileHelper::GamsCntr, Qt::CaseInsensitive) ||
            file.endsWith(mii::FileHelper::GamsDict, Qt::CaseInsensitive) ||
            file.endsWith(mii::FileHelper::Gamsmatr, Qt::CaseInsensitive) ||
            file.endsWith(mii::FileHelper::GamsSolu, Qt::CaseInsensitive) ||
            file.endsWith(mii::FileHelper::GamsStat, Qt::CaseInsensitive)) {
            ++datFileCount;
        }
    }
    if (datFileCount != 5) {
        ui->logEdit->appendPlainText("Error: It looks like the scratch files are incomplete. Loading scratch files aborted. Please check your model and " +
                                     ui->modelInspector->scratchDir());
        setRunButtonState(true);
        return;
    }
    ui->modelInspector->loadModelInstance(true);
    setGlobalFiltersData();
    emit ui->actionShow_Absolute->triggered();
}

void MainWindow::loadMultiModelInstance(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus);
    if (exitCode != 0) {
        appendLogMessage("The GAMSProcess reported an issue. The exit code is " +
                         QString().number(exitCode));
        setRunButtonState(true);
        return;
    }
    QDir scrdir(ui->modelInspector->scratchDir());
    if (!scrdir.isEmpty()) {
        ui->logEdit->appendPlainText("The scratch directory files have been updated.");
    } else {
        ui->logEdit->appendPlainText("Error: No scratch files found at " + ui->modelInspector->scratchDir());
    }
    ui->modelInspector->loadModelInstance(true);
    setGlobalFiltersData();
    emit ui->actionShow_Absolute->triggered();
}

void MainWindow::loadGAMSModel(const QString &path)
{
    QFileInfo fi(path + "/" + ui->modelEdit->text());
    if (fi.exists()) {
        auto result = QMessageBox::question(this, "Existing Model File",
                                            QString("The model (%1) already exists. Do you want to replace it?")
                                                .arg(fi.absoluteFilePath()));
        if (result == QMessageBox::No)
            return;
    }
    mLibProcess->setTargetDir(path);
    QString model = ui->modelEdit->text();
    mLibProcess->setModelName(model.replace(".gms", "").trimmed());
    mLibProcess->execute();
}

void MainWindow::setGlobalFiltersData()
{
    mFilterDialog->setViewConfig(ui->modelInspector->viewConfig());
}

void MainWindow::showDialog(QDialog *dialog)
{
    // needed for KDE to raise the dialog when minimized
    if (dialog->isMinimized())
        dialog->setWindowState(Qt::WindowMaximized);
    if (dialog->isVisible()) {
        // needed for macOS to rasise dialog when minimized
        dialog->raise();
        dialog->activateWindow();
    } else {
        dialog->show();
    }
}

QString MainWindow::workspace() const
{
    auto path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) +
            "/GAMS/ModelInspector/workspace";
    return path;
}

QString MainWindow::projectDirectory() const
{
    auto path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) +
            "/GAMS/ModelInspector/projects";
    return path;
}

void MainWindow::updateScratchDataWatcher(const QString& scrdir)
{
    if (!mScrWatcher.files().isEmpty()) {
        mScrWatcher.removePaths(mScrWatcher.files());
    }
    if (!mScrWatcher.directories().isEmpty()) {
        mScrWatcher.removePaths(mScrWatcher.directories());
    }
    mScrWatcher.addPath(scrdir + "/" + mii::FileHelper::GamsCntr);
    mScrWatcher.addPath(scrdir + "/" + mii::FileHelper::GamsDict);
    mScrWatcher.addPath(scrdir + "/" + mii::FileHelper::Gamsmatr);
    mScrWatcher.addPath(scrdir + "/" + mii::FileHelper::GamsSolu);
    mScrWatcher.addPath(scrdir + "/" + mii::FileHelper::GamsStat);
    if (mScrWatcher.files().count() < 5) {
        mScrWatcher.addPath(scrdir);
    }
}

void MainWindow::setRunButtonState(bool enabled)
{
    ui->runButton->setEnabled(enabled);
    ui->actionRun->setEnabled(enabled);
}
