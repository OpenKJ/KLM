#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include "karaokefile.h"
#include "dupefindercrc.h"
#include "dupefinderat.h"
#include "dupefindersid.h"

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

    void browseForPath();
    void runCrcScan();
    void runATScan();
    void runSIDScan();
    bool moveFile(const QString &filename, const QString &destPath);
    bool removeFile(const QString &filename);

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
