#include "mainwindow.h"
#include "sensor.h"
#include "uav.h"

#include <QApplication>
#include <QDebug>

#include <vector>

// Контрольная работа №2, вариант 2.
// Сначала выполняются консольные сценарии заданий (вывод qDebug()),
// затем открывается окно с визуализацией каждого задания.

// Задание 1. Виртуальные классы летательных аппаратов
static void run_task_1() {
    qDebug().noquote() << "===== Task 1: UAV (variant 2) =====";

    Quadcopter quadcopter;
    Hexacopter hexacopter;
    Octocopter octocopter;
    std::vector<UAV*> fleet{&quadcopter, &hexacopter, &octocopter};

    print_motor_summary(fleet);

    for (const UAV* vehicle : fleet) {
        qDebug().noquote() << "---";
        print_vehicle_info(*vehicle);
    }

    qDebug().noquote() << "--- Polymorphic flight: takeoff() and land() via UAV* ---";
    for (UAV* vehicle : fleet) {
        vehicle->takeoff();
        vehicle->land();
    }

    qDebug().noquote() << "--- Error scenarios ---";
    quadcopter.land();          // посадка аппарата, который уже на земле
    quadcopter.takeoff();
    quadcopter.takeoff();       // повторный взлёт
    quadcopter.land();
    std::vector<UAV*> broken_fleet{&hexacopter, nullptr};
    print_motor_summary(broken_fleet);   // нулевой указатель в списке
}

static void report_best(const Sensor* best) {
    if (best != nullptr) {
        qDebug().noquote() << QString("Best navigation source: %1 (accuracy=%2 m)")
                                  .arg(best->name())
                                  .arg(best->accuracy(), 0, 'f', 1);
    } else {
        qDebug().noquote() << "Best navigation source: none (nullptr)";
    }
}

// Задание 2. Абстрактный класс сенсора
static void run_task_2() {
    qDebug().noquote() << "===== Task 2: navigation sensors (variant 2) =====";

    GPS gps(10, 1.5);
    GLONASS glonass(8, 2.0);
    VisualOdometry visual_odometry(60, 3.2);
    std::vector<Sensor*> sensors{&gps, &glonass, &visual_odometry};

    print_sensors(sensors);
    report_best(choose_best_sensor(sensors));

    qDebug().noquote() << "--- Polymorphic read() ---";
    for (Sensor* sensor : sensors) {
        qDebug().noquote() << sensor->name() + ": altitude=" + QString::number(sensor->read()) + " m";
    }

    qDebug().noquote() << "--- GPS loses satellites: gps.set_satellites(3) ---";
    gps.set_satellites(3);
    print_sensors(sensors);
    report_best(choose_best_sensor(sensors));

    qDebug().noquote() << "--- GLONASS disabled: glonass.set_enabled(false) ---";
    glonass.set_enabled(false);
    report_best(choose_best_sensor(sensors));

    qDebug().noquote() << "--- Visual odometry lost features: set_tracked_features(5) ---";
    visual_odometry.set_tracked_features(5);
    report_best(choose_best_sensor(sensors));
    gps.read();                 // чтение неготового сенсора

    qDebug().noquote() << "--- Null pointer in the list ---";
    gps.set_satellites(10);
    std::vector<Sensor*> with_null{nullptr, &gps};
    report_best(choose_best_sensor(with_null));
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    MainWindow window;   // окно перехватывает qDebug() и показывает его в журнале

    run_task_1();
    run_task_2();
    qDebug().noquote() << "===== Interactive mode: use the tabs above =====";

    window.resize(1400, 900);
    window.show();

    return app.exec();
}
