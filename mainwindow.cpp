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
    ui->treeWidgetDuplicates->setColumnWidth(2, QFontMetrics(QApplication::font()).size(Qt::TextSingleLine,
                                                                                        "_Audio Bitrate_").width() +
                                                10);
    ui->treeWidgetDuplicates->setColumnWidth(0, 200);
    ui->treeWidgetDuplicates->setHeaderLabels({"File", "Count", "Audio Bitrate"});
    workerThreadCrc = new QThread(nullptr);
    workerThreadAT = new QThread(nullptr);
    workerThreadCrc->setObjectName("DupeFinderCRC");
    workerThreadAT->setObjectName("DupeFinderAT");
    dfCrc = new DupeFinderCRC(nullptr);
    dfAT = new DupeFinderAT(nullptr);
    dfCrc->moveToThread(workerThreadCrc);
    dfAT->moveToThread(workerThreadAT);
    
    // UI connections
    connect(ui->pushButtonBrowse, &QPushButton::clicked, [&]() { browseForPath(); });
    connect(ui->pushButtonCrc, &QPushButton::clicked, [&]() { runCrcScan(); });
    connect(ui->pushButtonAT, &QPushButton::clicked, [&]() { runATScan(); });
    connect(ui->treeWidgetDuplicates, &QTreeWidget::customContextMenuRequested, this,
            &MainWindow::duplicatesContextMenuRequested);
    connect(ui->lineEditPath, &QLineEdit::textChanged, [&](const QString &text) {
        m_path = ui->lineEditPath->text();
    });

    // CRC scan connections
    connect(workerThreadCrc, &QThread::started, dfCrc, &DupeFinderCRC::findDupes);
    connect(dfCrc, &DupeFinderCRC::finished, workerThreadCrc, &QThread::quit);    
    connect(dfCrc, &DupeFinderCRC::foundDuplicates, this, &MainWindow::dfFoundCRCDuplicates, Qt::QueuedConnection);
    connect(dfCrc, &DupeFinderCRC::foundBadFiles, this, &MainWindow::dfFoundBadFiles, Qt::QueuedConnection);
    connect(dfCrc, &DupeFinderCRC::noDupesFound, this, &MainWindow::dfNoDupesFound, Qt::QueuedConnection);
    connect(dfCrc, &DupeFinderCRC::finished, this, &MainWindow::dfFinished, Qt::QueuedConnection);

    // AT scan connections
    connect(workerThreadAT, &QThread::started, dfAT, &DupeFinderAT::findDupes);
    connect(dfAT, &DupeFinderAT::finished, workerThreadAT, &QThread::quit);
    connect(dfAT, &DupeFinderAT::foundDuplicates, this, &MainWindow::dfFoundATDuplicates, Qt::QueuedConnection);
    connect(dfAT, &DupeFinderAT::foundBadFiles, this, &MainWindow::dfFoundBadFiles, Qt::QueuedConnection);
    connect(dfAT, &DupeFinderAT::noDupesFound, this, &MainWindow::dfNoDupesFound, Qt::QueuedConnection);
    connect(dfAT, &DupeFinderAT::finished, this, &MainWindow::dfFinished, Qt::QueuedConnection);
}

void MainWindow::dfFoundCRCDuplicates(const KLM::KaraokeFileList &dupFiles) {
    auto *item = new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr), QStringList{
            "Matches: " + QString::number(dupFiles.size()) + " - Checksum: " +
            QString("%1").arg(dupFiles.at(0)->crc(), 0, 16, QLatin1Char('0'))});
    for (const auto &kFile : dupFiles) {
        auto child = new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr));
        child->setData(0, Qt::DisplayRole, kFile->path());
        child->setData(2, Qt::DisplayRole, QString::number(kFile->getBitrate()) + "kbps");
        item->addChild(child);
    }
    item->setData(1, Qt::DisplayRole, (uint) dupFiles.size());
    ui->treeWidgetDuplicates->addTopLevelItem(item);
}

void MainWindow::dfFoundATDuplicates(const KLM::KaraokeFileList &dupFiles) {
    auto *item = new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr), QStringList{
            "Matches: " + QString::number(dupFiles.size()) + " - Song: \"" + dupFiles.at(0)->atCombo() + "\""});
    for (const auto &kFile : dupFiles) {
        auto child = new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr));
        child->setData(0, Qt::DisplayRole, kFile->path());
        child->setData(2, Qt::DisplayRole, QString::number(kFile->getBitrate()) + "kbps");
        item->addChild(child);
    }
    item->setData(1, Qt::DisplayRole, (uint) dupFiles.size());
    ui->treeWidgetDuplicates->addTopLevelItem(item);
}

void MainWindow::dfFoundBadFiles(const KLM::KaraokeFileList &badFiles) {
    auto *item = new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr), QStringList{
            "Bad or corrupted files: " + QString::number(badFiles.size())});
    for (const auto &kFile : badFiles) {
        auto child = new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr));
        child->setData(0, Qt::DisplayRole, kFile->path());
        item->addChild(child);
    }
    item->setData(1, Qt::DisplayRole, 99999);
    ui->treeWidgetDuplicates->addTopLevelItem(item);
}

void MainWindow::dfNoDupesFound() {
    QMessageBox::information(this, "No duplicates found",
                             "Scan did not detect any duplicate files.");
}

void MainWindow::dfFinished() {
    ui->treeWidgetDuplicates->expandAll();
    ui->treeWidgetDuplicates->sortByColumn(1, Qt::DescendingOrder);
    for (int i = 0; i < ui->treeWidgetDuplicates->topLevelItemCount(); i++) {
        ui->treeWidgetDuplicates->topLevelItem(i)->sortChildren(0, Qt::AscendingOrder);
    }
    ui->treeWidgetDuplicates->resizeColumnToContents(0);
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

void MainWindow::runATScan() {
    if (m_path == QString()) {
        QMessageBox::warning(this, "No directory selected", "You must select a directory before running a scan");
        return;
    }
    ui->treeWidgetDuplicates->clear();
    dfAT->setPath(m_path);
    auto prgDlg = new QProgressDialog("Processing...", QString(), 0, 0, this);
    prgDlg->setWindowTitle("Processing...");
    prgDlg->setModal(true);
    prgDlg->show();
    connect(dfAT, &DupeFinderAT::newStepStarted, prgDlg, &QProgressDialog::setLabelText, Qt::QueuedConnection);
    connect(dfAT, &DupeFinderAT::progressValChanged, prgDlg, &QProgressDialog::setValue, Qt::QueuedConnection);
    connect(dfAT, &DupeFinderAT::stepMaxValChanged, prgDlg, &QProgressDialog::setMaximum, Qt::QueuedConnection);
    connect(dfAT, &DupeFinderAT::finished, prgDlg, &QProgressDialog::deleteLater, Qt::QueuedConnection);
    workerThreadAT->start();
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
            if (removeFile(file))
                curItem->parent()->removeChild(curItem);
        });
        contextMenu.addAction("Keep this file and delete others", [&]() {
            auto siblingCount = curItem->parent()->childCount();
            std::vector<QTreeWidgetItem *> siblings;
            for (int i = siblingCount - 1; i >= 0; i--) {
                auto item = curItem->parent()->child(i);
                if (item == curItem)
                    continue;
                if (removeFile(item->text(0)))
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
                if (moveFile(item->text(0), dest))
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
            if (removeFile(item->text(0)))
                item->parent()->removeChild(item);
        }
    });
    contextMenu.addAction("Move files", [&]() {
        auto dest = QFileDialog::getExistingDirectory(this, "Move location", QString(),
                                                      QFileDialog::DontUseNativeDialog | QFileDialog::ShowDirsOnly);
        if (dest.isEmpty())
            return;
        for (const auto &item : filteredItems) {
            if (moveFile(item->text(0), dest))
                item->parent()->removeChild(item);
        }
    });
    contextMenu.exec(QCursor::pos());
}

bool MainWindow::removeFile(const QString &filename) {
    if (!QFile::remove(filename)) {
        QMessageBox::warning(this, "Error deleting file",
                             "An error occurred while deleting the file, please ensure the file isn't read-only and that you have the requisite permissions.\n\nFile: " +
                             filename);
        return false;
    }
    if (filename.endsWith("cdg", Qt::CaseInsensitive)) {
        auto audioFile = KaraokeFile::findAudioFileForCdg(filename);
        if (audioFile.isEmpty())
        {
            QMessageBox::warning(this, "Possible problem with deletion",
                                 "The CDG file was successfully deleted, but KLM was unable to find a matching audio file.  "
                                 "  You will need to manually remove the audio file if one exists.");
        }
        if (!QFile::remove(audioFile)) {
            QMessageBox::warning(this, "Error deleting file!",
                                 "The CDG file was successfully deleted, but an error occurred while deleting its matching audio file!"
                                 "  You will need to manually remove the audio file.\n\nFile: " +
                                 audioFile);
        }
    }
    return true;
}

bool MainWindow::moveFile(const QString &filename, const QString &destPath) {
    if (!QFile::rename(filename, destPath + QDir::separator() + QFileInfo(filename).fileName())) {
        QMessageBox::warning(this, "Error moving file",
                             "An error occurred while moving the file, please ensure the file isn't read-only and that you have the requisite permissions.\n\nFile: " +
                             filename);
        spdlog::warn("Unable to move file: {} to destpath: {}", filename.toStdString(), destPath.toStdString());
        return false;
    }
    if (filename.endsWith("cdg", Qt::CaseInsensitive)) {
        auto audioFile = KaraokeFile::findAudioFileForCdg(filename);
        if (!QFile::rename(audioFile, destPath + QDir::separator() + QFileInfo(audioFile).fileName())) {
            QMessageBox::warning(this, "Error moving file",
                                 "An error occurred while moving the file.  The cdg file was successfully moved, but an error occurred while moving the matching audio "
                                 "file.  NOTE: You will need to move the audio file manually.\n\nFile: " +
                                 filename);
            spdlog::warn("Unable to move file: {} to destpath: {}", audioFile.toStdString(), destPath.toStdString());
        }
    }
    return true;
}
