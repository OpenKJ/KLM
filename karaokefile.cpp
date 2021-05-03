#include "karaokefile.h"
#include <quazip/quazip.h>
#include <quazip/quazipfile.h>
#include <spdlog/spdlog.h>
#include <taglib/taglib/tag.h>
#include <taglib/taglib/mpeg/mpegfile.h>
#include <taglib/taglib/mpeg/mpegproperties.h>
#include <taglib/taglib/mpeg/id3v2/id3v2framefactory.h>
#include <taglib/taglib/toolkit/tbytevectorstream.h>

struct membuf : std::streambuf {
    membuf(char const *base, size_t size) {
        char *p(const_cast<char *>(base));
        this->setg(p, p, p + size);
    }
};

struct imemstream : virtual membuf, std::istream {
    imemstream(char const *base, size_t size) :
            membuf(base, size),
            std::istream(static_cast<std::streambuf *>(this)) {
    }
};

void KaraokeFile::setPath(const QString &path) {
    m_path = path;
}

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

void KaraokeFile::scanFile() {
    QuaZip qz(m_path);
    if (!qz.open(QuaZip::mdUnzip)) {
        m_crc32 = 0;
        m_duration = 0;
        m_fileScanned = true;
        spdlog::warn("Error occurred while opening zip file: {}", m_path.toStdString());
        return;
    };
    auto files = qz.getFileInfoList();
    for (const auto &file : files)
        if (file.name.endsWith("cdg", Qt::CaseInsensitive)) {
            m_crc32 = file.crc;
            m_duration = ((file.uncompressedSize / 96) / 75) * 1000;
        };
    m_fileScanned = true;
}

uint KaraokeFile::getBitrate() {
    QuaZip qz(m_path);
    if (!qz.open(QuaZip::mdUnzip)) {
        spdlog::warn("Error occurred while opening zip file: {}", m_path.toStdString());
        return 0;
    };
    auto files = qz.getFileInfoList();
    for (const auto &file : files)
        if (file.name.endsWith("mp3", Qt::CaseInsensitive)) {
            qz.setCurrentFile(file.name);
            QuaZipFile mp3File(&qz);
            mp3File.open(QIODevice::ReadOnly);
            auto data = mp3File.readAll();
            TagLib::ByteVector bvec(data.constData(), data.size());
            TagLib::ByteVectorStream tios(bvec);
            TagLib::MPEG::File tref = TagLib::MPEG::File(&tios, TagLib::ID3v2::FrameFactory::FrameFactory::instance());
            spdlog::info("Bitrate: {} Filename: {}", tref.audioProperties()->bitrate(), file.name.toStdString());
            return tref.audioProperties()->bitrate();
        };
    return 0;
}
