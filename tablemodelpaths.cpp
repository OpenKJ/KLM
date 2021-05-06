#include "tablemodelpaths.h"

TableModelPaths::TableModelPaths(QObject *parent)
    : QAbstractTableModel(parent)
{
}

QVariant TableModelPaths::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        switch (section) {
        case PATH:
            return "Path";
        case PATTERN:
            return "Naming Pattern";
        }
    }
    return QVariant();
}

int TableModelPaths::rowCount([[maybe_unused]]const QModelIndex &parent) const
{
    return m_paths.size();
}

int TableModelPaths::columnCount([[maybe_unused]]const QModelIndex &parent) const
{
    return 2;
}

QVariant TableModelPaths::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();  

    if (role == Qt::DisplayRole)
    {
        switch (index.column()) {
        case PATH:
            return m_paths.at(index.row())->path();
        case PATTERN:
            return m_paths.at(index.row())->pattern().name;
        }
    }
    return QVariant();
}

void TableModelPaths::addPath(QSharedPointer<KaraokePath> path)
{
    emit layoutAboutToBeChanged();
    m_paths.push_back(path);
    emit layoutChanged();
}

const KLM::KaraokePathList &TableModelPaths::paths() const
{
    return m_paths;
}

bool TableModelPaths::exists(const QString &path)
{
    auto it = std::find_if(m_paths.begin(), m_paths.end(), [path] (auto kPath) {
        return (kPath->path() == path);
    });
    if (it == m_paths.end())
        return false;
    return true;
}

void TableModelPaths::remove(const QString &path)
{
    emit layoutAboutToBeChanged();
    m_paths.erase(std::remove_if(m_paths.begin(), m_paths.end(), [path] (auto kPath) {
        return (kPath->path() == path);
    }), m_paths.end());
    emit layoutChanged();
}

void TableModelPaths::modifyNamingPattern(const QString &path, NamingPattern newPattern)
{
    auto it = std::find_if(m_paths.begin(), m_paths.end(), [path] (auto kPath) {
        return (kPath->path() == path);
    });
    if (it == m_paths.end())
        return;
    emit layoutAboutToBeChanged();
    it->get()->setPattern(newPattern);
    emit layoutChanged();
}
