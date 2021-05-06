#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QDirIterator>
#include <QFileDialog>
#include <set>
#include <QTreeWidgetItem>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QProgressDialog>
#include <spdlog/spdlog.h>
#include <karaokefile.h>
#include <QFontMetrics>
#include <QInputDialog>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    QApplication::setOrganizationName("OpenKJ Project");
    QApplication::setApplicationName("KLM");
    QApplication::setApplicationDisplayName("Karaoke Library Manager");

    ui->treeWidgetDuplicates->setColumnCount(3);
    ui->treeWidgetDuplicates->hideColumn(1);
    ui->treeWidgetDuplicates->setHeaderHidden(false);
    ui->treeWidgetDuplicates->setColumnWidth(2, QFontMetrics(QApplication::font()).size(Qt::TextSingleLine,
                                                                                        "_Audio Bitrate_").width() +
                                                10);
    ui->treeWidgetDuplicates->setColumnWidth(0, 200);
    ui->treeWidgetDuplicates->setHeaderLabels({"File", "Count", "Audio Bitrate"});
    ui->treeWidgetDuplicates->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeWidgetDuplicates->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->pushButtonChangePattern->setEnabled(false);
    ui->pushButtonRemovePath->setEnabled(false);
    ui->tableViewPaths->setModel(&pathsModel);
    ui->tableViewPaths->horizontalHeader()->setSectionResizeMode(TableModelPaths::PATH, QHeaderView::Stretch);
    ui->tableViewPaths->horizontalHeader()->setSectionResizeMode(TableModelPaths::PATTERN, QHeaderView::Interactive);
    ui->tableViewPaths->setColumnWidth(TableModelPaths::PATTERN, QFontMetrics(QApplication::font()).horizontalAdvance("_SongID - Artist - Title_"));

    workerThreadCrc = new QThread(nullptr);
    workerThreadAT = new QThread(nullptr);
    workerThreadSID = new QThread(nullptr);
    workerThreadCrc->setObjectName("DupeFinderCRC");
    workerThreadAT->setObjectName("DupeFinderAT");
    workerThreadSID->setObjectName("DupeFinderSID");
    dfCrc = new DupeFinderCRC(nullptr);
    dfAT = new DupeFinderAT(nullptr);
    dfSID = new DupeFinderSID(nullptr);
    dfCrc->moveToThread(workerThreadCrc);
    dfAT->moveToThread(workerThreadAT);
    dfSID->moveToThread(workerThreadSID);
    
    // UI connections
    connect(ui->pushButtonCrc, &QPushButton::clicked, [&]() { runCrcScan(); });
    connect(ui->pushButtonAT, &QPushButton::clicked, [&]() { runATScan(); });
    connect(ui->pushButtonSongId, &QPushButton::clicked, [&]() { runSIDScan(); });
    connect(ui->treeWidgetDuplicates, &QTreeWidget::customContextMenuRequested, this,
            &MainWindow::duplicatesContextMenuRequested);
    connect(ui->pushButtonAddPath, &QPushButton::clicked, this, &MainWindow::onAddPathClicked);
    connect(ui->pushButtonRemovePath, &QPushButton::clicked, this, &MainWindow::onRemovePathClicked);
    connect(ui->pushButtonChangePattern, &QPushButton::clicked, this, &MainWindow::onChangePatternClicked);
    connect(ui->tableViewPaths, &QTableView::clicked, this, &MainWindow::onTableViewPathSelectionChanged);

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

    // AT scan connections
    connect(workerThreadSID, &QThread::started, dfSID, &DupeFinderSID::findDupes);
    connect(dfSID, &DupeFinderSID::finished, workerThreadSID, &QThread::quit);
    connect(dfSID, &DupeFinderSID::foundDuplicates, this, &MainWindow::dfFoundSIDDuplicates, Qt::QueuedConnection);
    connect(dfSID, &DupeFinderSID::foundBadFiles, this, &MainWindow::dfFoundBadFiles, Qt::QueuedConnection);
    connect(dfSID, &DupeFinderSID::noDupesFound, this, &MainWindow::dfNoDupesFound, Qt::QueuedConnection);
    connect(dfSID, &DupeFinderSID::finished, this, &MainWindow::dfFinished, Qt::QueuedConnection);

    restorePaths();

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
    item->setExpanded(true);
}

void MainWindow::dfFoundSIDDuplicates(const KLM::KaraokeFileList &dupFiles) {
    auto *item = new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr), QStringList{
            "Matches: " + QString::number(dupFiles.size()) + " - SongID: \"" + dupFiles.at(0)->songid() + "\""});
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

void MainWindow::runCrcScan() {
    if (pathsModel.rowCount() == 0) {
        QMessageBox::warning(this, "No paths to scan", "You must add at least one karaoke path before running a scan");
        return;
    }
    ui->treeWidgetDuplicates->clear();
    dfCrc->setPaths(pathsModel.paths());
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
    if (pathsModel.rowCount() == 0) {
        QMessageBox::warning(this, "No paths to scan", "You must add at least one karaoke path before running a scan");
        return;
    }
    ui->treeWidgetDuplicates->clear();
    dfAT->setPaths(pathsModel.paths());
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

void MainWindow::runSIDScan() {
    if (pathsModel.rowCount() == 0) {
        QMessageBox::warning(this, "No paths to scan", "You must add at least one karaoke path before running a scan");
        return;
    }
    ui->treeWidgetDuplicates->clear();
    dfSID->setPaths(pathsModel.paths());
    auto prgDlg = new QProgressDialog("Processing...", QString(), 0, 0, this);
    prgDlg->setWindowTitle("Processing...");
    prgDlg->setModal(true);
    prgDlg->show();
    connect(dfSID, &DupeFinderSID::newStepStarted, prgDlg, &QProgressDialog::setLabelText, Qt::QueuedConnection);
    connect(dfSID, &DupeFinderSID::progressValChanged, prgDlg, &QProgressDialog::setValue, Qt::QueuedConnection);
    connect(dfSID, &DupeFinderSID::stepMaxValChanged, prgDlg, &QProgressDialog::setMaximum, Qt::QueuedConnection);
    connect(dfSID, &DupeFinderSID::finished, prgDlg, &QProgressDialog::deleteLater, Qt::QueuedConnection);
    workerThreadSID->start();
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

void MainWindow::savePaths()
{
    QSettings settings;
    auto paths = pathsModel.paths();
    settings.remove("karaokePaths");
    settings.beginWriteArray("karaokePaths");
    int idx{0};
    for (auto &path : paths)
    {
        settings.setArrayIndex(idx++);
        settings.setValue("path", path->path());
        settings.setValue("patternName", path->pattern().name);
        settings.setValue("patternSep", path->pattern().sep);
        settings.setValue("patternArtistPos", path->pattern().artistPos);
        settings.setValue("patternTitlePos", path->pattern().titlePos);
        settings.setValue("patternSongIdPos", path->pattern().songIdPos);
    }
    settings.endArray();
}

void MainWindow::restorePaths()
{
    QSettings settings;
    int size = settings.beginReadArray("karaokePaths");
    for (int i = 0; i < size; ++i) {
        QString path, patternName, patternSep;
        int patternArtistPos, patternTitlePos, patternSongIdPos;
        settings.setArrayIndex(i);
        path = settings.value("path").toString();
        patternName = settings.value("patternName").toString();
        patternSep = settings.value("patternSep").toString();
        patternArtistPos = settings.value("patternArtistPos").toInt();
        patternTitlePos = settings.value("patternTitlePos").toInt();
        patternSongIdPos = settings.value("patternSongIdPos").toInt();
        pathsModel.addPath(
                    QSharedPointer<KaraokePath>(
                        new KaraokePath(
                            path,
                            NamingPattern{
                                patternName,
                                patternSep,
                                patternSongIdPos,
                                patternArtistPos,
                                patternTitlePos
                            })));

    }
    settings.endArray();
}



void MainWindow::onAddPathClicked()
{
    auto path = QFileDialog::getExistingDirectory(this, "Select karaoke library path", QString(),
                                               QFileDialog::DontUseNativeDialog | QFileDialog::ShowDirsOnly);
    if (path.isEmpty())
        return;

    if (pathsModel.exists(path))
    {
        QMessageBox::warning(this, "Unable to add", "Sorry that path already exists in the paths list");
        return;
    }

    QStringList patterns{
        "SongID - Artist - Title",
        "SongID - Title - Artist",
        "Artist - Title - SongID",
        "Title - Artist - SongID",
        "Artist - Title",
        "Title - Artist"
    };
    auto selection = QInputDialog::getItem(this, "Naming Pattern", "Naming pattern of files in this directory", patterns, -1, false);
    if (selection.isEmpty())
        return;
    int idx = patterns.indexOf(selection);
    NamingPattern pattern;
    switch (idx) {
    case 0:
        pattern = NamingPattern::PatternSAT();
        break;
    case 1:
        pattern = NamingPattern::PatternSTA();
        break;
    case 2:
        pattern = NamingPattern::PatternATS();
        break;
    case 3:
        pattern = NamingPattern::PatternTAS();
        break;
    case 4:
        pattern = NamingPattern::PatternAT();
        break;
    case 5:
        pattern = NamingPattern::PatternTA();
    default:
        spdlog::error("User somehow picked a non-existent naming pattern, something is very wrong");
        return;
    }
    pathsModel.addPath(QSharedPointer<KaraokePath>(new KaraokePath(path, pattern)));
    savePaths();
}

void MainWindow::onRemovePathClicked()
{
    if (ui->tableViewPaths->selectionModel()->selectedIndexes().isEmpty())
        return;
    QString path = ui->tableViewPaths->selectionModel()->selectedRows(0).at(0).data(Qt::DisplayRole).toString();
    pathsModel.remove(path);
    savePaths();
}

void MainWindow::onChangePatternClicked()
{
    if (ui->tableViewPaths->selectionModel()->selectedIndexes().isEmpty())
        return;
    QString path = ui->tableViewPaths->selectionModel()->selectedRows(0).at(0).data(Qt::DisplayRole).toString();
    if (!pathsModel.exists(path))
        return;
    QStringList patterns{
        "SongID - Artist - Title",
        "SongID - Title - Artist",
        "Artist - Title - SongID",
        "Title - Artist - SongID",
        "Artist - Title",
        "Title - Artist"
    };
    auto selection = QInputDialog::getItem(this, "Change Naming Pattern", "Naming pattern of files in this directory", patterns, -1, false);
    if (selection.isEmpty())
        return;
    int idx = patterns.indexOf(selection);
    NamingPattern pattern;
    switch (idx) {
    case 0:
        pattern = NamingPattern::PatternSAT();
        break;
    case 1:
        pattern = NamingPattern::PatternSTA();
        break;
    case 2:
        pattern = NamingPattern::PatternATS();
        break;
    case 3:
        pattern = NamingPattern::PatternTAS();
        break;
    case 4:
        pattern = NamingPattern::PatternAT();
        break;
    case 5:
        pattern = NamingPattern::PatternTA();
    default:
        spdlog::error("User somehow picked a non-existent naming pattern, something is very wrong");
        return;
    }
    // todo: add code to do the change;
    pathsModel.modifyNamingPattern(path, pattern);
    savePaths();
}

void MainWindow::onTableViewPathSelectionChanged()
{
    if (ui->tableViewPaths->selectionModel()->selectedRows(0).isEmpty())
    {
        ui->pushButtonChangePattern->setEnabled(false);
        ui->pushButtonRemovePath->setEnabled(false);
    }
    else
    {
        ui->pushButtonChangePattern->setEnabled(true);
        ui->pushButtonRemovePath->setEnabled(true);
    }
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
