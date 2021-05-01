#include "karaokefile.h"
#include "karaokefileinfo.h"

void KaraokeFile::setTitle(const QString &title)
{
    m_title = title;
}

void KaraokeFile::setSongid(const QString &songid)
{
    m_songid = songid;
}

void KaraokeFile::setPath(const QString &path)
{
    m_path = path;
    m_info.setFileName(m_path);
}

uint32_t KaraokeFile::crc()
{
    if (m_crc_check_run)
        return m_crc32;
    m_crc32 = m_info.getCRC32checksum();
    m_crc_check_run = true;
    return m_crc32;
}

uint64_t KaraokeFile::duration()
{
    if (m_duration > 0)
        return m_duration;
    m_duration = m_info.getDuration();
    return m_duration;
}

void KaraokeFile::setArtist(const QString &artist)
{
    m_artist = artist;
}
