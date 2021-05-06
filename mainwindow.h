#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include "karaokefile.h"
#include "dupefindercrc.h"
#include "dupefinderat.h"
#include "dupefindersid.h"
#include "tablemodelpaths.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    Ui::MainWindow *ui;
    QString m_path;
    QThread *workerThreadCrc;
    DupeFinderCRC *dfCrc;
    QThread *workerThreadAT;
    DupeFinderAT *dfAT;
    QThread *workerThreadSID;
    DupeFinderSID *dfSID;
    KLM::KaraokePathList m_paths;
    TableModelPaths pathsModel;

    void runCrcScan();
    void runATScan();
    void runSIDScan();
    bool moveFile(const QString &filename, const QString &destPath);
    bool removeFile(const QString &filename);
    void savePaths();
    void restorePaths();

public slots:
    void onAddPathClicked();
    void onRemovePathClicked();
    void onChangePatternClicked();
    void onTableViewPathSelectionChanged();

private slots:
    void duplicatesContextMenuRequested(const QPoint &pos);
    void dfFinished();
    void dfNoDupesFound();
    void dfFoundBadFiles(const KLM::KaraokeFileList &badFiles);
    void dfFoundCRCDuplicates(const KLM::KaraokeFileList &dupFiles);
    void dfFoundATDuplicates(const KLM::KaraokeFileList &dupFiles);
    void dfFoundSIDDuplicates(const KLM::KaraokeFileList &dupFiles);

};
#endif // MAINWINDOW_H
