#include "uav_scene.h"

#include "uav.h"

#include <QDebug>

UavScene::UavScene(QObject* parent)
    : QObject(parent) {}

void UavScene::set_vehicle(UAV* vehicle) {
    if (vehicle == nullptr) {
        qDebug().noquote() << "Error: vehicle is not set";
    }
    current = vehicle;
    emit changed();
}

const UAV* UavScene::vehicle() const {
    return current;
}

QString UavScene::type_name() const {
    return current ? current->type() : "—";
}

int UavScene::motor_count() const {
    return current ? current->motor_count() : 0;
}

bool UavScene::airborne() const {
    return current && current->is_airborne();
}

void UavScene::takeoff() {
    if (current == nullptr) {
        qDebug().noquote() << "Error: vehicle is not set";
        return;
    }
    current->takeoff();
    emit changed();
}

void UavScene::land() {
    if (current == nullptr) {
        qDebug().noquote() << "Error: vehicle is not set";
        return;
    }
    current->land();
    emit changed();
}
