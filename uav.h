#ifndef UAV_H
#define UAV_H

#include <QString>

#include <vector>

// Задание 1, вариант 2. Абстрактный базовый класс летательного аппарата.
// Создать объект UAV нельзя — только его наследников.
class UAV {
public:
    virtual void takeoff() = 0;
    virtual void land() = 0;
    virtual QString type() const = 0;
    virtual int motor_count() const = 0;     // добавлено по варианту 2

    // Тип взлёта и посадки нужны print_vehicle_info(const UAV&),
    // а takeoff()/land() не const, поэтому описание вынесено в отдельные методы.
    virtual QString takeoff_type() const = 0;
    virtual QString landing_type() const = 0;

    virtual ~UAV() = default;

    bool is_airborne() const;

protected:
    void set_airborne(bool value);   // меняют состояние только наследники

private:
    bool airborne = false;
};

// Промежуточный абстрактный класс: общее поведение всех мультикоптеров
// (вертикальный взлёт и посадка). type() и motor_count() остаются чисто виртуальными.
class Multicopter : public UAV {
public:
    void takeoff() override;
    void land() override;
    QString takeoff_type() const override;
    QString landing_type() const override;
};

class Quadcopter : public Multicopter {
public:
    QString type() const override;
    int motor_count() const override;
};

class Hexacopter : public Multicopter {
public:
    QString type() const override;
    int motor_count() const override;
};

class Octocopter : public Multicopter {
public:
    QString type() const override;
    int motor_count() const override;
};

// Универсальные функции: работают только через интерфейс UAV,
// фактический тип аппарата им неизвестен.
QString vehicle_info_text(const UAV& vehicle);
void print_vehicle_info(const UAV& vehicle);
void print_motor_summary(const std::vector<UAV*>& fleet);

#endif
