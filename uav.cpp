#include "uav.h"

#include <QDebug>

bool UAV::is_airborne() const {
    return airborne;
}

void UAV::set_airborne(bool value) {
    airborne = value;
}

// ---------- Multicopter ----------

void Multicopter::takeoff() {
    if (is_airborne()) {
        qDebug().noquote() << "Error:" << type() << "is already airborne";
        return;
    }
    qDebug().noquote() << type() + ": " + takeoff_type();
    set_airborne(true);
}

void Multicopter::land() {
    if (!is_airborne()) {
        qDebug().noquote() << "Error:" << type() << "is already on the ground";
        return;
    }
    qDebug().noquote() << type() + ": " + landing_type();
    set_airborne(false);
}

QString Multicopter::takeoff_type() const {
    return "vertical takeoff";
}

QString Multicopter::landing_type() const {
    return "vertical landing";
}

// ---------- Конкретные аппараты ----------

QString Quadcopter::type() const {
    return "Quadcopter";
}

int Quadcopter::motor_count() const {
    return 4;
}

QString Hexacopter::type() const {
    return "Hexacopter";
}

int Hexacopter::motor_count() const {
    return 6;
}

QString Octocopter::type() const {
    return "Octocopter";
}

int Octocopter::motor_count() const {
    return 8;
}

// ---------- Универсальные функции ----------

QString vehicle_info_text(const UAV& vehicle) {
    return QString("Vehicle type: %1\n"
                   "Motors: %2\n"
                   "Takeoff: %3\n"
                   "Landing: %4")
        .arg(vehicle.type())
        .arg(vehicle.motor_count())
        .arg(vehicle.takeoff_type())
        .arg(vehicle.landing_type());
}

void print_vehicle_info(const UAV& vehicle) {
    qDebug().noquote() << vehicle_info_text(vehicle);
}

void print_motor_summary(const std::vector<UAV*>& fleet) {
    for (const UAV* vehicle : fleet) {
        if (vehicle == nullptr) {
            qDebug().noquote() << "Error: vehicle is not set";
            continue;
        }
        qDebug().noquote() << QString("%1: %2 motors")
                                  .arg(vehicle->type())
                                  .arg(vehicle->motor_count());
    }
}
