#ifndef UAV_SCENE_H
#define UAV_SCENE_H

#include <QObject>
#include <QString>

class UAV;

// Мост между C++ и 3D-сценой uav_scene.qml (задание 1).
// Хранит указатель на абстрактный UAV и знает о нём только интерфейс:
// число моторов для модели берётся из motor_count(), а не из типа класса.
class UavScene : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString type_name READ type_name NOTIFY changed)
    Q_PROPERTY(int motor_count READ motor_count NOTIFY changed)
    Q_PROPERTY(bool airborne READ airborne NOTIFY changed)

public:
    explicit UavScene(QObject* parent = nullptr);

    void set_vehicle(UAV* vehicle);
    const UAV* vehicle() const;

    QString type_name() const;
    int motor_count() const;
    bool airborne() const;

public slots:
    void takeoff();
    void land();

signals:
    void changed();

private:
    UAV* current = nullptr;
};

#endif
