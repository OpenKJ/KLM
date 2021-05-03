#ifndef KARAOKEFILE_H
#define KARAOKEFILE_H

#include <QString>
#include <vector>
#include <QObject>

struct NamingPattern {
    QString name{"SongID - Artist - Title"};
    QString sep{" - "};
    int songIdPos{0};
    int artistPos{1};
    int titlePos{2};

    [[maybe_unused]] static NamingPattern PatternSAT() {
        return NamingPattern{"SongID - Artist - Title", " - ", 0, 1, 2};
    }

    [[maybe_unused]] static NamingPattern PatternSTA() {
        return NamingPattern{"SongID - Title - Artist", " - ", 0, 2, 1};
    }

    [[maybe_unused]] static NamingPattern PatternATS() {
        return NamingPattern{"Artist - Title - SongID", " - ", 2, 0, 1};
    }

    [[maybe_unused]] static NamingPattern PatternTAS() {
        return NamingPattern{"Title - Artist - SongID", " - ", 2, 1, 0};
    }

    [[maybe_unused]] static NamingPattern PatternAT() {
        return NamingPattern{"Artist - Title", " - ", -1, 0, 1};
    }

    [[maybe_unused]] static NamingPattern PatternTA() {
        return NamingPattern{"Title - Artist", " - ", -1, 1, 0};
    }
};

class KaraokeFile : public QObject {
    Q_OBJECT
private:
    QString m_artist;
    QString m_title;
    QString m_songid;
    QString m_path;
    uint64_t m_duration{0};
    uint32_t m_crc32{0};
    uint m_audioBitrate{0};
    bool m_fileScanned{false};
    NamingPattern m_namingPattern;
    QStringList supportedKFileExtensions{"cdg", "zip", "mp4", "mkv", "avi", "ogv"};
    QStringList supportedAFileExtensions{".mp3", ".ogg", ".wav"};
    QString findAudioFileForCdg();

    void applyNamingPattern();

public:
    enum class ErrorCode{
        OK,
        BadZip,
        MissingCdg,
        MissingAudio,
        ZeroByteCdg,
        ZeroByteAudio,
        ZeroByte
    };
    std::string getErrCodeStr() const;
    [[nodiscard]] ErrorCode getErrorCode() const {return m_errorCode;}
    explicit KaraokeFile(QString path, QObject *parent = nullptr);
    [[nodiscard]] QString artist() const;
    [[nodiscard]] QString title() const { return m_title; }
    [[nodiscard]] QString songid() const { return m_songid; }
    QString path() const { return m_path; }
    void setPath(const QString &path);
    uint32_t crc();
    uint64_t duration();
    void scanZipFile();
    uint getBitrate();
    ErrorCode m_errorCode{ErrorCode::OK};

    void setNamingPattern(const NamingPattern &pattern);
};

#endif // KARAOKEFILE_H
