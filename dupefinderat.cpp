#include "dupefinderat.h"
#include <set>
#include <algorithm>
#include <QDirIterator>
#include <mutex>
#include "karaokefile.h"
#include <spdlog/spdlog.h>


void DupeFinderAT::setPaths(KLM::KaraokePathList paths)
{
    m_paths = paths;
}

DupeFinderAT::DupeFinderAT(QObject *parent) : QObject(parent) {
}

void DupeFinderAT::findDupes() {
    std::set<QString> atCombos;
    emit newStepStarted("Finding karaoke files...");
    emit stepMaxValChanged(0);
    KLM::KaraokeFileList kFiles;
    for ( auto path : m_paths )
    {
        spdlog::info("Getting files in path: {}", path->path().toStdString());
        kFiles.append(path->files());
        spdlog::info("Found {} karaoke files", path->files().size());
    }
    spdlog::info("Found a totoal of {} karaoke files in all paths, getting artist/title data...", kFiles.size());
    emit newStepStarted("Getting artist/title data...");
    emit stepMaxValChanged(kFiles.size());
    int processedFiles{0};
    for (const auto &kFile : kFiles) {
        emit progressValChanged(++processedFiles);
        atCombos.insert(kFile->atCombo());
    }
    spdlog::info("Got artist/title data, finding duplicates");
    emit newStepStarted("Scanning for duplicates...");
    emit stepMaxValChanged((int) atCombos.size());
    int processedCombos{0};
    uint dupesFound{0};
    spdlog::info("Checking for bad files...");
    KLM::KaraokeFileList badFiles;
    std::copy_if(kFiles.begin(), kFiles.end(), std::back_inserter(badFiles), [] (auto file) {
        return (file->getErrorCode() != KaraokeFile::ErrorCode::OK);
    });
    kFiles.erase(std::remove_if(kFiles.begin(), kFiles.end(), [] (auto file) {
        return (file->getErrorCode() != KaraokeFile::ErrorCode::OK);
    }), kFiles.end());
    if (!badFiles.empty()) {
        spdlog::warn("Found {} bad zip files", badFiles.size());
        emit foundBadFiles(badFiles);
    }
    else
        spdlog::info("No bad files detected");
    for (const auto &atCombo : atCombos) {
        KLM::KaraokeFileList newVec;
        if (atCombo == "") {
            emit progressValChanged(++processedCombos);
            continue;
        }
        std::copy_if(kFiles.begin(), kFiles.end(), std::back_inserter(newVec), [atCombo](auto file) {
            return (file->atCombo() == atCombo);
        });
        kFiles.erase(std::remove_if(kFiles.begin(), kFiles.end(), [atCombo](auto file) {
            return (file->atCombo() == atCombo);
        }), kFiles.end());
        if (newVec.size() > 1) {
            for (const auto& kFile : newVec)
                kFile->getBitrate();
            emit foundDuplicates(newVec);
            dupesFound++;
        }
        emit progressValChanged(++processedCombos);
    }
    if (dupesFound == 0)
            emit noDupesFound();
    spdlog::info("Processing complete");
    kFiles.clear();
    kFiles.shrink_to_fit();
    emit finished();
}
