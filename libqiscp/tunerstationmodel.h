#ifndef TUNERSTATIONMODEL_H
#define TUNERSTATIONMODEL_H

#include <QObject>
#include <QAbstractListModel>

class TunerStationModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit TunerStationModel(QObject *parent=0);

    enum Roles {StationID = Qt::UserRole, TitleRole, TuneRole, LocationSetRole};
    int rowCount(const QModelIndex &parent=QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;

    QHash<int, QByteArray> roleNames() const {
            QHash<int, QByteArray> roles;
            roles[StationID] = "station";
            roles[TitleRole] = "stationTitle";
            roles[TuneRole] = "description";
            roles[LocationSetRole] = "inUse";
            return roles;
        }
};

#endif // TUNERSTATIONMODEL_H
