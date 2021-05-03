#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QDirIterator>
#include <QFileDialog>
#include <set>
#include <QTreeWidgetItem>
#include <QMessageBox>
#include <QProgressDialog>
#include <spdlog/spdlog.h>
#include <karaokefile.h>
#include <QFontMetrics>

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    ui->treeWidgetDuplicates->setColumnCount(3);
    ui->treeWidgetDuplicates->hideColumn(1);
    ui->treeWidgetDuplicates->setHeaderHidden(false);
    ui->treeWidgetDuplicates->setColumnWidth(2, QFontMetrics(QApplication::font()).size(Qt::TextSingleLine, "_Audio Bitrate_").width() + 10);
    ui->treeWidgetDuplicates->setColumnWidth(0, 200);
    ui->treeWidgetDuplicates->setHeaderLabels({"File", "Count", "Audio Bitrate"});
    workerThreadCrc = new QThread(nullptr);
    dfCrc = new DupeFinderCRC(nullptr);
    dfCrc->moveToThread(workerThreadCrc);
    connect(workerThreadCrc, &QThread::started, dfCrc, &DupeFinderCRC::findDupes);
    connect(dfCrc, &DupeFinderCRC::finished, workerThreadCrc, &QThread::quit);
    connect(ui->pushButtonBrowse, &QPushButton::clicked, [&]() { browseForPath(); });
    connect(ui->pushButtonCrc, &QPushButton::clicked, [&]() { runCrcScan(); });
    connect(ui->treeWidgetDuplicates, &QTreeWidget::customContextMenuRequested, this,
            &MainWindow::duplicatesContextMenuRequested);
    connect(ui->lineEditPath, &QLineEdit::textChanged, [&](const QString &text) {
        m_path = ui->lineEditPath->text();
    });

    connect(dfCrc, &DupeFinderCRC::foundDuplicate, this, [&](const QString &crc, const QStringList &paths) {
        auto *item = new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr), QStringList{
                "Matches: " + QString::number(paths.size()) + " - Checksum: " + crc});
        for (const auto &path : paths) {
            auto child = new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr), QStringList{path});
            KaraokeFile kFile;
            kFile.setPath(path);
            child->setData(2, Qt::DisplayRole, QString::number(kFile.getBitrate()) + "kbps");
            item->addChild(child);
        }
        item->setData(1, Qt::DisplayRole, paths.size());
        ui->treeWidgetDuplicates->addTopLevelItem(item);
    }, Qt::QueuedConnection);
    connect(dfCrc, &DupeFinderCRC::noDupesFound, this, [&]() {
        QMessageBox::information(this, "No duplicates found",
                                 "Scan did not find any files with identical CRC signatures.");
    }, Qt::QueuedConnection);
    connect(dfCrc, &DupeFinderCRC::finished, this, [&]() {
        ui->treeWidgetDuplicates->expandAll();
        ui->treeWidgetDuplicates->sortByColumn(1, Qt::DescendingOrder);
        for (int i = 0; i < ui->treeWidgetDuplicates->topLevelItemCount(); i++) {
            ui->treeWidgetDuplicates->topLevelItem(i)->sortChildren(0, Qt::AscendingOrder);
        }
        ui->treeWidgetDuplicates->resizeColumnToContents(0);
    }, Qt::QueuedConnection);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::browseForPath() {
    m_path = QFileDialog::getExistingDirectory(this, "Select karaoke library path", QString(),
                                               QFileDialog::DontUseNativeDialog | QFileDialog::ShowDirsOnly);
    ui->lineEditPath->setText(m_path);
    spdlog::info("User selected path: {}", m_path.toStdString());

}

void MainWindow::runCrcScan() {
    if (m_path == QString()) {
        QMessageBox::warning(this, "No directory selected", "You must select a directory before running a scan");
        return;
    }
    ui->treeWidgetDuplicates->clear();
    dfCrc->setPath(m_path);
    auto prgDlg = new QProgressDialog("Processing...", QString(), 0, 0, this);
    prgDlg->setWindowTitle("Processing...");
    prgDlg->setModal(true);
    prgDlg->show();
    connect(dfCrc, &DupeFinderCRC::newStepStarted, prgDlg, &QProgressDialog::setLabelText, Qt::QueuedConnection);
    connect(dfCrc, &DupeFinderCRC::progressValChanged, prgDlg, &QProgressDialog::setValue, Qt::QueuedConnection);
    connect(dfCrc, &DupeFinderCRC::stepMaxValChanged, prgDlg, &QProgressDialog::setMaximum, Qt::QueuedConnection);
    connect(dfCrc, &DupeFinderCRC::finished, prgDlg, &QProgressDialog::deleteLater, Qt::QueuedConnection);
    workerThreadCrc->start();
}


void MainWindow::duplicatesContextMenuRequested(const QPoint &pos) {
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
            if (QFile::remove(file))
                curItem->parent()->removeChild(curItem);
            else
                QMessageBox::warning(this, "Error deleting file",
                                     "An error occurred while deleting the file, please ensure the file isn't read-only and that you have the requisite permissions.");
        });
        contextMenu.addAction("Keep this file and delete others", [&]() {
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
    });
    contextMenu.exec(QCursor::pos());
}
