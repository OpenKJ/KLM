#include "karaokefile.h"
#include <quazip/quazip.h>
#include <quazip/quazipfile.h>
#include <spdlog/spdlog.h>
#include <taglib/taglib/tag.h>
#include <taglib/taglib/mpeg/mpegfile.h>
#include <taglib/taglib/mpeg/id3v2/id3v2framefactory.h>
#include <taglib/taglib/toolkit/tbytevectorstream.h>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <utility>
#include "qcrc32.h"


uint32_t KaraokeFile::crc() {
    if (!m_fileScanned)
        scanFile();
    return m_crc32;
}

uint64_t KaraokeFile::duration() {
    if (!m_fileScanned)
        scanFile();
    return m_duration;
}

void KaraokeFile::scanZipFile() {
    QuaZip qz(m_path);
    if (!qz.open(QuaZip::mdUnzip)) {
        m_crc32 = 0;
        m_duration = 0;
        m_fileScanned = true;
        m_errorCode = ErrorCode::BadZip;
        spdlog::warn("Error occurred while opening zip file: {}", m_path.toStdString());
        return;
    };
    auto files = qz.getFileInfoList();
    bool cdgFound{false};
    bool audioFound{false};
    for (const auto &file : files) {
        if (file.name.endsWith("cdg", Qt::CaseInsensitive)) {
            m_crc32 = file.crc;
            m_duration = getDurationFromCdgSize(file.uncompressedSize);
            cdgFound = true;
            if (file.uncompressedSize == 0)
                m_errorCode = ErrorCode::ZeroByteCdg;
            continue;
        };
        for (const auto &ext : supportedAFileExtensions()) {
            if (file.name.endsWith(ext, Qt::CaseInsensitive)) {
                audioFound = true;
                if (file.uncompressedSize == 0)
                    m_errorCode = ErrorCode::ZeroByteAudio;
            }
        }
    }
    if (!cdgFound)
        m_errorCode = ErrorCode::MissingCdg;
    if (!audioFound)
        m_errorCode = ErrorCode::MissingAudio;
    m_fileScanned = true;
    if (m_errorCode != ErrorCode::OK)
        spdlog::warn("Error {} while processing zip file: {}", getErrCodeStr(), m_path.toStdString());
}

uint KaraokeFile::getBitrate() {
    if (m_audioBitrate > 0)
        return m_audioBitrate;
    if (m_procMode == ProcessingMode::AudioGZip) {
        QuaZip qz(m_path);
        if (!qz.open(QuaZip::mdUnzip)) {
            spdlog::warn("Error occurred while opening zip file: {}", m_path.toStdString());
            m_errorCode = ErrorCode::BadZip;
            return 0;
        };
        auto files = qz.getFileInfoList();
        for (const auto &file : files) {
            if (file.name.endsWith("mp3", Qt::CaseInsensitive)) {
                qz.setCurrentFile(file.name);
                QuaZipFile mp3File(&qz);
                mp3File.open(QIODevice::ReadOnly);
                auto data = mp3File.readAll();
                TagLib::ByteVector byteVector(data.constData(), data.size());
                TagLib::ByteVectorStream tIoStream(byteVector);
                TagLib::MPEG::File tMpgFile = TagLib::MPEG::File(&tIoStream,
                                                                 TagLib::ID3v2::FrameFactory::FrameFactory::instance());
                spdlog::debug("Bitrate: {} Filename: {}", tMpgFile.audioProperties()->bitrate(),
                              file.name.toStdString());
                m_audioBitrate = tMpgFile.audioProperties()->bitrate();
            };
        };
    }
    else if (m_procMode == ProcessingMode::AudioG)
    {
        auto audioFile = findAudioFileForCdg();
        if (m_errorCode != ErrorCode::OK) {
            spdlog::warn("Error {} while processing cdg file: {}", getErrCodeStr(), m_path.toStdString());
            return 0;
        }
        TagLib::MPEG::File tMpgFile = TagLib::MPEG::File(audioFile.toStdString().c_str());
        spdlog::debug("Bitrate: {} Filename: {}", tMpgFile.audioProperties()->bitrate(), audioFile.toStdString());
        m_audioBitrate = tMpgFile.audioProperties()->bitrate();
    }
    return m_audioBitrate;
}

QString KaraokeFile::artist() const {

    return m_artist;
}

void KaraokeFile::applyNamingPattern() {
    auto parts = QFileInfo(m_path).completeBaseName().split(m_namingPattern.sep);
    if (m_namingPattern.songIdPos != -1 && m_namingPattern.songIdPos < parts.size()) {
        m_songid = parts.at(m_namingPattern.songIdPos);
        m_songid.replace('_', ' ');
    } else
        m_songid = "";
    if (m_namingPattern.artistPos != -1 && m_namingPattern.artistPos < parts.size()) {
        m_artist = parts.at(m_namingPattern.artistPos);
        m_artist.replace('_', ' ');
    } else
        m_artist = "";
    if (m_namingPattern.titlePos != -1 && m_namingPattern.titlePos < parts.size()) {
        m_title = parts.at(m_namingPattern.titlePos);
        m_title.replace('_', ' ');
    } else
        m_title = "";
}

void KaraokeFile::setNamingPattern(const NamingPattern &pattern) {
    m_namingPattern = pattern;
    applyNamingPattern();
}


KaraokeFile::KaraokeFile(QString path, QObject *parent) : m_path(std::move(path)), QObject(parent) {
    applyNamingPattern();
    scanFile();
}

void KaraokeFile::scanCdgFile() {
    QFileInfo fi(m_path);
    Crc32 crc;
    m_crc32 = crc.calculateFromFile(m_path);
    m_duration = getDurationFromCdgSize(fi.size());
    findAudioFileForCdg();
    if (m_errorCode != ErrorCode::OK)
        spdlog::warn("Error {} while processing cdg file: {}", getErrCodeStr(), m_path.toStdString());
    m_fileScanned = true;
}

std::string KaraokeFile::getErrCodeStr() const {
    switch (m_errorCode) {
        case ErrorCode::OK:
            return "ErrorCode::OK";
        case ErrorCode::BadZip:
            return "ErrorCode::BadZip";
        case ErrorCode::MissingCdg:
            return "ErrorCode::MissingCdg";
        case ErrorCode::MissingAudio:
            return "ErrorCode::MissingAudio";
        case ErrorCode::ZeroByteCdg:
            return "ErrorCode::ZeroByteCdg";
        case ErrorCode::ZeroByteAudio:
            return "ErrorCode::ZeroByteAudio";
        case ErrorCode::ZeroByte:
            return "ErrorCode::ZeroByte";
    }
    return std::string();
}

QString KaraokeFile::findAudioFileForCdg() {
    auto audioFile = findAudioFileForCdg(m_path);
    if (audioFile.isEmpty())
        m_errorCode = ErrorCode::MissingAudio;
    if (QFileInfo(audioFile).size() == 0)
        m_errorCode = ErrorCode::ZeroByteAudio;
    if (m_errorCode != ErrorCode::OK)
        spdlog::warn("Error {} while processing cdg file: {}", getErrCodeStr(), m_path.toStdString());
    return audioFile;
}

void KaraokeFile::scanFile() {
    if (m_fileScanned)
        return;
    if (m_path.endsWith("zip", Qt::CaseInsensitive)) {
        m_procMode = ProcessingMode::AudioGZip;
        scanZipFile();
    }
    else if (m_path.endsWith("cdg", Qt::CaseInsensitive)) {
        m_procMode = ProcessingMode::AudioG;
        scanCdgFile();
    }
    else
        m_procMode = ProcessingMode::VideoFile;
}

QString KaraokeFile::findAudioFileForCdg(const QString& cdgFilePath) {
    QFileInfo fi(cdgFilePath);
    for (const auto &ext : supportedAFileExtensions()) {
        QString afn = fi.absolutePath() + QDir::separator() + fi.completeBaseName() + ext;
        if (QFile::exists(afn)) {
            return afn;
        }
    }
    return QString();
}

