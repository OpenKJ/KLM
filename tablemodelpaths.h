#ifndef TABLEMODELPATHS_H
#define TABLEMODELPATHS_H

#include <QAbstractTableModel>
#include "karaokefile.h"

class TableModelPaths : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Columns{
        PATH,
        PATTERN
    };
    explicit TableModelPaths(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    void addPath(QSharedPointer<KaraokePath> path);
    const KLM::KaraokePathList& paths() const;
    bool exists(const QString &path);
    void remove(const QString &path);
    void modifyNamingPattern(const QString &path, NamingPattern newPattern);

private:
    KLM::KaraokePathList m_paths;
};

#endif // TABLEMODELPATHS_H
