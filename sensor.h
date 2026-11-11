#ifndef SENSOR_H
#define SENSOR_H

#include <QString>

#include <vector>

// Задание 2, вариант 2. Абстрактный базовый класс сенсора.
// read() возвращает измеренную высоту аппарата, м.
class Sensor {
public:
    virtual double read() = 0;
    virtual bool is_ready() const = 0;
    virtual QString name() const = 0;
    virtual double accuracy() const = 0;     // добавлено по варианту 2: ошибка, м

    // Строка параметров для вывода. Не чисто виртуальный метод:
    // по умолчанию выводится только точность, наследники дополняют её.
    virtual QString details() const;

    virtual ~Sensor() = default;
};

// Общая часть спутниковых приёмников (GPS, ГЛОНАСС). Класс абстрактный:
// name() остаётся чисто виртуальным.
class SatelliteSensor : public Sensor {
public:
    SatelliteSensor(int satellites, double accuracy_m, double altitude_m);

    double read() override;
    bool is_ready() const override;
    double accuracy() const override;
    QString details() const override;

    int satellites() const;
    void set_satellites(int count);
    void set_accuracy(double accuracy_m);
    void set_enabled(bool enabled);
    bool is_enabled() const;

    static constexpr int min_satellites = 4;   // меньше 4 спутников — нет решения

private:
    int satellite_count;
    double error_m;
    double altitude;
    bool enabled = true;
};

class GPS : public SatelliteSensor {
public:
    GPS(int satellites, double accuracy_m, double altitude_m = 150.0);
    QString name() const override;
};

class GLONASS : public SatelliteSensor {
public:
    GLONASS(int satellites, double accuracy_m, double altitude_m = 150.0);
    QString name() const override;
};

// Визуальная одометрия: готова, если камера отслеживает достаточно ориентиров.
class VisualOdometry : public Sensor {
public:
    VisualOdometry(int tracked_features, double accuracy_m, double altitude_m = 150.0);

    double read() override;
    bool is_ready() const override;
    QString name() const override;
    double accuracy() const override;

    int tracked_features() const;
    void set_tracked_features(int count);
    void set_accuracy(double accuracy_m);
    void set_enabled(bool enabled);
    bool is_enabled() const;

    static constexpr int min_features = 20;

private:
    int feature_count;
    double error_m;
    double altitude;
    bool enabled = true;
};

// Универсальные функции: работают только через интерфейс Sensor.
void print_sensors(const std::vector<Sensor*>& sensors);
Sensor* choose_best_sensor(const std::vector<Sensor*>& sensors);

#endif
