#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include "karaokefile.h"
#include "dupefindercrc.h"

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

    void browseForPath();
    void runCrcScan();
    bool moveFile(const QString &filename, const QString &destPath);
    bool removeFile(const QString &filename);

private slots:
    void duplicatesContextMenuRequested(const QPoint &pos);

};
#endif // MAINWINDOW_H
