#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProgressDialog>
#include <QThread>
#include "karaokefile.h"
#include "dupefindercrc.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void on_treeWidgetDuplicates_customContextMenuRequested(const QPoint &pos);

private:
    Ui::MainWindow *ui;
    QString m_path;
    void browseForPath();
    void runCrcScan();
    QThread *workerThreadCrc;
    DupeFinderCRC *dfCrc;
    QProgressDialog *prgDlg{nullptr};
};
#endif // MAINWINDOW_H
