#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QDirIterator>
#include <QDebug>
#include <QFileDialog>
#include <algorithm>
#include <set>
#include <QTreeWidgetItem>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    workerThreadCrc = new QThread(nullptr);
    dfCrc = new DupeFinderCRC(nullptr);
    dfCrc->moveToThread(workerThreadCrc);
    connect(workerThreadCrc, &QThread::started, dfCrc, &DupeFinderCRC::findDupes);
    connect(dfCrc, &DupeFinderCRC::finished, workerThreadCrc, &QThread::quit);
    connect(ui->pushButtonBrowse, &QPushButton::clicked, [&] () { browseForPath(); });
    connect(ui->pushButtonCrc, &QPushButton::clicked, [&] () { runCrcScan(); });
    connect(dfCrc, &DupeFinderCRC::foundDuplicate, this, [&] (QString crc, QStringList paths) {
        QList<QTreeWidgetItem *> items;
        QTreeWidgetItem *item = new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr), QStringList{
                                                        "Matches: " + QString::number(paths.size()) + " - Checksum: " + crc});
        for ( auto path : paths )
        {
            item->addChild(new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr), QStringList{path}));
        }
        ui->treeWidgetDuplicates->addTopLevelItem(item);
        //ui->treeWidgetDuplicates->expandAll();
    }, Qt::QueuedConnection);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::browseForPath()
{
    m_path = QFileDialog::getExistingDirectory(this, "Select karaoke library path", QString(), QFileDialog::DontUseNativeDialog | QFileDialog::ShowDirsOnly);
    ui->lineEditPath->setText(m_path);
    qInfo() << "Path: " << m_path;
}

void MainWindow::runCrcScan()
{
    // todo: add warning dialog on no path
    if (m_path == QString())
        return;
    ui->treeWidgetDuplicates->clear();
    dfCrc->setPath(m_path);
    prgDlg = new QProgressDialog("Processing...", QString(), 0, 0, this);
    prgDlg->setWindowTitle("Processing...");
    connect(dfCrc, &DupeFinderCRC::findingFilesStarted, [&] () {
       prgDlg->setLabelText("Finding karaoke files");
       prgDlg->setMaximum(0);
       prgDlg->setModal(true);
       prgDlg->show();
    });
    connect(dfCrc, &DupeFinderCRC::gettingChecksumsStarted, [&] (int num) {
       prgDlg->setMaximum(num);
       prgDlg->setLabelText("Calculating CRC values");
    });
    connect(dfCrc, &DupeFinderCRC::gettingChecksumsProgress, [&] (int pos) {
        prgDlg->setValue(pos);
    });
    connect(dfCrc, &DupeFinderCRC::dupeFindStarted, [&] (int num) {
       prgDlg->setLabelText("Checking for duplicate signatures");
       prgDlg->setValue(0);
       prgDlg->setMaximum(num);
    });
    connect(dfCrc, &DupeFinderCRC::dupeFindProgress, [&] (int pos) {
        prgDlg->setValue(pos);
    });
    workerThreadCrc->start();
}


void MainWindow::on_treeWidgetDuplicates_customContextMenuRequested(const QPoint &pos)
{
    auto item = ui->treeWidgetDuplicates->itemAt(pos);
    if (!item)
        return;
    if (item->childCount() > 0)
    {
        QMenu contextMenu(this);
        contextMenu.addAction("Expand", [&] () {
            item->setExpanded(true);
        });
        contextMenu.addAction("Collapse", [&] () {
            item->setExpanded(false);
        });
        contextMenu.addAction("Expand All", [&] () {
            ui->treeWidgetDuplicates->expandAll();
        });
        contextMenu.addAction("Collapse All", [&] () {
            ui->treeWidgetDuplicates->collapseAll();
        });
        contextMenu.exec(QCursor::pos());
        return;
    }
    auto file = item->text(0);
    QMenu contextMenu(this);
    contextMenu.addAction("Delete this file", [&] () {
        qWarning() << "Would delete: " << file;
        // todo: add delete logic
    });
    contextMenu.addAction("Keep this file and delete others", [&] () {
        qWarning() << "Would keep: " << file;
        // todo: add delete logic
    });
    contextMenu.exec(QCursor::pos());
}
