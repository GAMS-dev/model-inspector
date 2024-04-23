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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFileSystemWatcher>
#include <QMainWindow>
#include <QProcess>
#include <QSharedPointer>

class QLabel;

namespace Ui {
class MainWindow;
}

class GAMSLibProcess;
class GAMSProcess;

namespace gams {
namespace studio {
namespace mii {
class FilterDialog;
}
}
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void appendLogMessage(const QString &message);

    // File
    void on_actionOpen_triggered();
    void on_actionRun_triggered();
    void on_action_Quit_triggered();

    // Edit
    void on_action_Search_triggered();
    void searchHeaders();
    void editMenuAboutToShow();

    // View
    void on_actionFilters_triggered();
    void on_actionShow_search_result_triggered();
    void showAbsoluteValues();
    void on_actionShow_Output_triggered();
    void on_actionZoom_In_triggered();
    void on_actionZoom_Out_triggered();
    void on_actionZoom_Reset_triggered();

    // Help
    void on_actionAbout_Model_Inspector_triggered();
    void on_actionAbout_Qt_triggered();

    // Other
    void loadModelInstance(int exitCode, QProcess::ExitStatus exitStatus);
    void handleLibProcessResult(int exitCode, QProcess::ExitStatus exitStatus);
    void viewConfigUpdate();
    void setGlobalFiltersData();
    void searchResultSelectionChanged(const QModelIndex &index);
    void updateModelInstance();
    void viewChanged(int viewType);
    void scrDirectoryChanged();
    void scrFileChanged();

private:
    void setupConnections();

    void createProjectDirectory();

    QString aboutModelInspector() const;

    void loadSingleModelInstance(int exitCode, QProcess::ExitStatus exitStatus);

    void loadMultiModelInstance(int exitCode, QProcess::ExitStatus exitStatus);

    void loadGAMSModel(const QString &path);

    void showDialog(QDialog *dialog);

    QString workspace() const;

    QString projectDirectory() const;

    void updateScratchDataWatcher(const QString& scrdir);

    void setRunButtonState(bool enabled);

private:
    Ui::MainWindow *ui;
    GAMSLibProcess *mLibProcess;
    QSharedPointer<GAMSProcess> mProcess;
    gams::studio::mii::FilterDialog *mFilterDialog;
    QFileSystemWatcher mScrWatcher;
    const QString mScrUpdateWarning = "Warning: It looks like the scratch data has not been updated.";
    bool mScrFilesUpdated = false;
    bool mLoadScrFiles = false;
};

#endif // MAINWINDOW_H
