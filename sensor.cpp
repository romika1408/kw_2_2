#include "sensor.h"

#include <QDebug>

#include <limits>

QString Sensor::details() const {
    return QString("accuracy=%1 m").arg(accuracy(), 0, 'f', 1);
}

// ---------- SatelliteSensor ----------

SatelliteSensor::SatelliteSensor(int satellites, double accuracy_m, double altitude_m)
    : satellite_count(0), error_m(0.0), altitude(altitude_m) {
    set_satellites(satellites);
    set_accuracy(accuracy_m);
}

double SatelliteSensor::read() {
    if (!is_ready()) {
        qDebug().noquote() << "Error:" << name() << "is not ready";
        return std::numeric_limits<double>::quiet_NaN();
    }
    return altitude;
}

bool SatelliteSensor::is_ready() const {
    return enabled && satellite_count >= min_satellites;
}

double SatelliteSensor::accuracy() const {
    return error_m;
}

QString SatelliteSensor::details() const {
    return QString("satellites=%1, ").arg(satellite_count) + Sensor::details();
}

int SatelliteSensor::satellites() const {
    return satellite_count;
}

void SatelliteSensor::set_satellites(int count) {
    if (count < 0) {
        qDebug().noquote() << "Error: satellite count cannot be negative, set to 0";
        count = 0;
    }
    satellite_count = count;
}

void SatelliteSensor::set_accuracy(double accuracy_m) {
    if (accuracy_m <= 0.0) {
        qDebug().noquote() << "Error: accuracy must be positive, value ignored";
        return;
    }
    error_m = accuracy_m;
}

void SatelliteSensor::set_enabled(bool value) {
    enabled = value;
}

bool SatelliteSensor::is_enabled() const {
    return enabled;
}

GPS::GPS(int satellites, double accuracy_m, double altitude_m)
    : SatelliteSensor(satellites, accuracy_m, altitude_m) {}

QString GPS::name() const {
    return "GPS";
}

GLONASS::GLONASS(int satellites, double accuracy_m, double altitude_m)
    : SatelliteSensor(satellites, accuracy_m, altitude_m) {}

QString GLONASS::name() const {
    return "GLONASS";
}

// ---------- VisualOdometry ----------

VisualOdometry::VisualOdometry(int tracked_features, double accuracy_m, double altitude_m)
    : feature_count(0), error_m(0.0), altitude(altitude_m) {
    set_tracked_features(tracked_features);
    set_accuracy(accuracy_m);
}

double VisualOdometry::read() {
    if (!is_ready()) {
        qDebug().noquote() << "Error:" << name() << "is not ready";
        return std::numeric_limits<double>::quiet_NaN();
    }
    return altitude;
}

bool VisualOdometry::is_ready() const {
    return enabled && feature_count >= min_features;
}

QString VisualOdometry::name() const {
    return "Visual odometry";
}

double VisualOdometry::accuracy() const {
    return error_m;
}

int VisualOdometry::tracked_features() const {
    return feature_count;
}

void VisualOdometry::set_tracked_features(int count) {
    if (count < 0) {
        qDebug().noquote() << "Error: feature count cannot be negative, set to 0";
        count = 0;
    }
    feature_count = count;
}

void VisualOdometry::set_accuracy(double accuracy_m) {
    if (accuracy_m <= 0.0) {
        qDebug().noquote() << "Error: accuracy must be positive, value ignored";
        return;
    }
    error_m = accuracy_m;
}

void VisualOdometry::set_enabled(bool value) {
    enabled = value;
}

bool VisualOdometry::is_enabled() const {
    return enabled;
}

// ---------- Универсальные функции ----------

void print_sensors(const std::vector<Sensor*>& sensors) {
    for (const Sensor* sensor : sensors) {
        if (sensor == nullptr) {
            qDebug().noquote() << "Error: sensor is not set";
            continue;
        }
        const QString state = sensor->is_ready() ? "" : " [not ready]";
        qDebug().noquote() << sensor->name() + ": " + sensor->details() + state;
    }
}

// Критерии: сенсор готов; среди готовых — минимальная ошибка.
// Если готовых нет — сообщение об ошибке и nullptr.
Sensor* choose_best_sensor(const std::vector<Sensor*>& sensors) {
    Sensor* best = nullptr;
    for (Sensor* sensor : sensors) {
        if (sensor == nullptr) {
            qDebug().noquote() << "Error: sensor is not set, skipped";
            continue;
        }
        if (!sensor->is_ready()) {
            continue;
        }
        if (best == nullptr || sensor->accuracy() < best->accuracy()) {
            best = sensor;
        }
    }
    if (best == nullptr) {
        qDebug().noquote() << "Error: no ready navigation sensor";
    }
    return best;
}
