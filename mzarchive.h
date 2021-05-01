/*
 * Copyright (c) 2013-2016 Thomas Isaac Lightburn
 *
 *
 * This file is part of OpenKJ.
 *
 * OpenKJ is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef MZARCHIVE_H
#define MZARCHIVE_H

#include <QObject>
#include <QStringList>
#include <okarchive.h>


class MzArchive : public QObject
{
    Q_OBJECT
public:
    explicit MzArchive(const QString &ArchiveFile, QObject *parent = 0);
    explicit MzArchive(QObject *parent = 0);
    unsigned int getSongDuration();

    void setArchiveFile(const QString &value);
    bool checkCDG();
    bool checkAudio();
    QString audioExtension();
    bool extractAudio(const QString &destPath, const QString &destFile);
    bool extractCdg(const QString &destPath, const QString &destFile);
    bool isValidKaraokeFile();
    QString getLastError();
    uint32_t cdgCRC32() const;
private:
    QString archiveFile;
    QString cdgFileName;
    QString audioFileName;
    QString audioExt;
    QString lastError;
    bool findCDG();
    bool findAudio();
    uint64_t cdgSize();
    uint64_t audioSize();
    uint32_t m_audioFileIndex{0};
    uint32_t m_cdgFileIndex{0};
    uint64_t m_cdgSize{0};
    uint64_t m_audioSize{0};
    uint32_t m_cdgCRC32{0};
    bool m_audioSupportedCompression{false};
    bool m_cdgSupportedCompression{false};
    bool m_cdgFound{false};
    bool m_audioFound{false};
    bool findEntries();
    QStringList audioExtensions;
    OkArchive oka;

signals:

public slots:
};

#endif // KHARCHIVE_H
