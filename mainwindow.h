#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "sensor.h"
#include "uav.h"

#include <QMainWindow>

#include <vector>

class QCheckBox;
class QDoubleSpinBox;
class QLabel;
class QListWidget;
class QPlainTextEdit;
class QSpinBox;
class SensorMapWidget;
class UavScene;

// Главное окно: вкладка на каждое задание и общий журнал вывода qDebug().
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    static void append_log(const QString& line);

private slots:
    void select_vehicle(int row);
    void run_fleet_test();
    void apply_sensor_inputs();
    void reset_sensors();

private:
    QWidget* build_uav_tab();
    QWidget* build_sensor_tab();
    void update_vehicle_info();
    void update_sensor_view();

    // Задание 1: аппараты и их общий список через абстрактный указатель
    Quadcopter quadcopter;
    Hexacopter hexacopter;
    Octocopter octocopter;
    std::vector<UAV*> fleet;

    // Задание 2: сенсоры с контрольными значениями
    GPS gps{10, 1.5};
    GLONASS glonass{8, 2.0};
    VisualOdometry visual_odometry{60, 3.2};
    std::vector<Sensor*> sensors;
    Sensor* last_best = nullptr;

    UavScene* scene = nullptr;
    QListWidget* vehicle_list = nullptr;
    QLabel* vehicle_info = nullptr;

    QCheckBox* gps_enabled = nullptr;
    QSpinBox* gps_satellites = nullptr;
    QDoubleSpinBox* gps_accuracy = nullptr;
    QCheckBox* glonass_enabled = nullptr;
    QSpinBox* glonass_satellites = nullptr;
    QDoubleSpinBox* glonass_accuracy = nullptr;
    QCheckBox* vo_enabled = nullptr;
    QSpinBox* vo_features = nullptr;
    QDoubleSpinBox* vo_accuracy = nullptr;
    SensorMapWidget* sensor_map = nullptr;

    QPlainTextEdit* log_view = nullptr;
    static MainWindow* instance;
};

#endif
