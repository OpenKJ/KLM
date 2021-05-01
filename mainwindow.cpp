#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QDirIterator>
#include <QDebug>
#include <QFileDialog>
#include <set>
#include <QTreeWidgetItem>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    ui->treeWidgetDuplicates->setColumnCount(2);
    ui->treeWidgetDuplicates->hideColumn(1);
    workerThreadCrc = new QThread(nullptr);
    dfCrc = new DupeFinderCRC(nullptr);
    dfCrc->moveToThread(workerThreadCrc);
    connect(workerThreadCrc, &QThread::started, dfCrc, &DupeFinderCRC::findDupes);
    connect(dfCrc, &DupeFinderCRC::finished, workerThreadCrc, &QThread::quit);
    connect(ui->pushButtonBrowse, &QPushButton::clicked, [&]() { browseForPath(); });
    connect(ui->pushButtonCrc, &QPushButton::clicked, [&]() { runCrcScan(); });
    connect(dfCrc, &DupeFinderCRC::foundDuplicate, this, [&](const QString &crc, const QStringList &paths) {
        auto *item = new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr), QStringList{
                "Matches: " + QString::number(paths.size()) + " - Checksum: " + crc});
        for (const auto &path : paths) {
            item->addChild(new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr), QStringList{path}));
        }
        item->setData(1, Qt::DisplayRole, paths.size());
        ui->treeWidgetDuplicates->addTopLevelItem(item);
    }, Qt::QueuedConnection);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::browseForPath() {
    m_path = QFileDialog::getExistingDirectory(this, "Select karaoke library path", QString(),
                                               QFileDialog::DontUseNativeDialog | QFileDialog::ShowDirsOnly);
    ui->lineEditPath->setText(m_path);
    qInfo() << "Path: " << m_path;
}

void MainWindow::runCrcScan() {
    // todo: add warning dialog on no path
    if (m_path == QString())
        return;
    ui->treeWidgetDuplicates->clear();
    dfCrc->setPath(m_path);
    prgDlg = new QProgressDialog("Processing...", QString(), 0, 0, this);
    prgDlg->setWindowTitle("Processing...");
    connect(dfCrc, &DupeFinderCRC::findingFilesStarted, this, [&]() {
        prgDlg->setLabelText("Finding karaoke files");
        prgDlg->setMaximum(0);
        prgDlg->setModal(true);
        prgDlg->show();
    }, Qt::QueuedConnection);
    connect(dfCrc, &DupeFinderCRC::gettingChecksumsStarted, this, [&](int num) {
        prgDlg->setMaximum(num);
        prgDlg->setLabelText("Calculating CRC values");
    }, Qt::QueuedConnection);
    connect(dfCrc, &DupeFinderCRC::gettingChecksumsProgress, this, [&](int pos) {
        prgDlg->setValue(pos);
    }, Qt::QueuedConnection);
    connect(dfCrc, &DupeFinderCRC::dupeFindStarted, this, [&](int num) {
        prgDlg->setLabelText("Checking for duplicate signatures");
        prgDlg->setValue(0);
        prgDlg->setMaximum(num);
    }, Qt::QueuedConnection);
    connect(dfCrc, &DupeFinderCRC::dupeFindProgress, this, [&](int pos) {
        prgDlg->setValue(pos);
    }, Qt::QueuedConnection);
    connect(dfCrc, &DupeFinderCRC::finished, this, [&]() {
        ui->treeWidgetDuplicates->expandAll();
        ui->treeWidgetDuplicates->sortByColumn(1, Qt::DescendingOrder);
        for (int i = 0; i < ui->treeWidgetDuplicates->topLevelItemCount(); i++) {
            ui->treeWidgetDuplicates->topLevelItem(i)->sortChildren(0, Qt::AscendingOrder);
        }
        if (prgDlg)
            prgDlg->hide();
    }, Qt::QueuedConnection);
    connect(dfCrc, &DupeFinderCRC::noDupesFound, this, [&]() {
        QMessageBox::information(this, "No duplicates found",
                                 "Scan did not find any files with identical CRC signatures.");
    }, Qt::QueuedConnection);
    workerThreadCrc->start();
}


void MainWindow::on_treeWidgetDuplicates_customContextMenuRequested(const QPoint &pos) {
    auto selItems = ui->treeWidgetDuplicates->selectedItems();
    if (selItems.empty())
        return;
    auto curItem = ui->treeWidgetDuplicates->itemAt(pos);
    if (selItems.size() == 1 && curItem->childCount() > 0) {
        QMenu contextMenu(this);
        contextMenu.addAction("Expand", [&]() {
            curItem->setExpanded(true);
        });
        contextMenu.addAction("Collapse", [&]() {
            curItem->setExpanded(false);
        });
        contextMenu.addAction("Expand All", [&]() {
            ui->treeWidgetDuplicates->expandAll();
        });
        contextMenu.addAction("Collapse All", [&]() {
            ui->treeWidgetDuplicates->collapseAll();
        });
        contextMenu.exec(QCursor::pos());
        return;
    }
    if (selItems.size() == 1) {
        auto file = curItem->text(0);
        QMenu contextMenu(this);
        contextMenu.addAction("Delete this file", [&]() {
            qWarning() << "Would delete: " << file;
            if (QFile::remove(file))
                curItem->parent()->removeChild(curItem);
            else
                QMessageBox::warning(this, "Error deleting file",
                                     "An error occurred while deleting the file, please ensure the file isn't read-only and that you have the requisite permissions.");
        });
        contextMenu.addAction("Keep this file and delete others", [&]() {
            qWarning() << "Would keep: " << file;
            auto siblingCount = curItem->parent()->childCount();
            std::vector<QTreeWidgetItem *> siblings;
            for (int i = siblingCount - 1; i >= 0; i--) {
                auto item = curItem->parent()->child(i);
                if (item == curItem)
                    continue;
                if (!QFile::remove(item->text(0))) {
                    QMessageBox::warning(this, "Error deleting file",
                                         "An error occurred while deleting the file, please ensure the file isn't read-only and that you have the requisite permissions.\n\nFile: " +
                                         item->text(0));
                    continue;
                }
                curItem->parent()->removeChild(item);

            }
        });
        contextMenu.addAction("Keep this file and move others", [&]() {
            qWarning() << "Would keep: " << file;
            auto dest = QFileDialog::getExistingDirectory(this, "Move location", QString(),
                                                          QFileDialog::DontUseNativeDialog | QFileDialog::ShowDirsOnly);
            if (dest.isEmpty())
                return;
            auto siblingCount = curItem->parent()->childCount();
            std::vector<QTreeWidgetItem *> siblings;
            for (int i = siblingCount - 1; i >= 0; i--) {
                auto item = curItem->parent()->child(i);
                if (item == curItem)
                    continue;
                if (!QFile::rename(item->text(0), dest + QDir::separator() + QFileInfo(item->text(0)).fileName())) {
                    QMessageBox::warning(this, "Error deleting file",
                                         "An error occurred while moving file, please ensure the file isn't read-only and that you have the requisite permissions.\n\nFile: " +
                                         item->text(0));
                    continue;
                }
                curItem->parent()->removeChild(item);

            }
        });
        contextMenu.exec(QCursor::pos());
        return;
    }
    QMenu contextMenu(this);
    std::vector<QTreeWidgetItem *> filteredItems;
    for (const auto &item : selItems) {
        if (!item->text(0).contains("Checksum:"))
            filteredItems.push_back(item);
    }
    contextMenu.addAction("Delete files", [&]() {
        for (const auto &item : filteredItems) {
            if (!QFile::remove(item->text(0))) {
                QMessageBox::warning(this, "Error deleting file",
                                     "An error occurred while deleting the file, please ensure the file isn't read-only and that you have the requisite permissions.\n\nFile: " +
                                     item->text(0));
                continue;
            }
            item->parent()->removeChild(item);
        }
    });
    contextMenu.addAction("Move files", [&]() {
        auto dest = QFileDialog::getExistingDirectory(this, "Move location", QString(),
                                                      QFileDialog::DontUseNativeDialog | QFileDialog::ShowDirsOnly);
        if (dest.isEmpty())
            return;
        for (const auto &item : filteredItems) {
            if (!QFile::rename(item->text(0), dest + QDir::separator() + QFileInfo(item->text(0)).fileName())) {
                QMessageBox::warning(this, "Error moving file",
                                     "An error occurred while moving the file, please ensure the file isn't read-only and that you have the requisite permissions.\n\nFile: " +
                                     item->text(0));
                continue;
            }
            item->parent()->removeChild(item);
        }
        // todo: add multi move logic
    });
    contextMenu.exec(QCursor::pos());
}
