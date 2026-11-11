#include "mainwindow.h"

#include "sensor_map_widget.h"
#include "uav_scene.h"

#include <QCheckBox>
#include <QDebug>
#include <QDoubleSpinBox>
#include <QFontDatabase>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QQmlContext>
#include <QQuickWidget>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QSplitter>
#include <QTabWidget>
#include <QVBoxLayout>

MainWindow* MainWindow::instance = nullptr;

namespace {

QtMessageHandler previous_handler = nullptr;

// Весь вывод qDebug() попадает и в консоль Qt Creator, и в журнал окна
void message_handler(QtMsgType type, const QMessageLogContext& context, const QString& message) {
    if (previous_handler != nullptr) {
        previous_handler(type, context, message);
    }
    MainWindow::append_log(message);
}

}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      fleet{&quadcopter, &hexacopter, &octocopter},
      sensors{&gps, &glonass, &visual_odometry} {
    setWindowTitle("Контрольная работа №2 — вариант 2: полиморфизм в архитектуре БПЛА");

    auto* tabs = new QTabWidget;
    tabs->addTab(build_uav_tab(), "Задание 1. Мультикоптеры (UAV)");
    tabs->addTab(build_sensor_tab(), "Задание 2. Навигационные сенсоры (Sensor)");

    log_view = new QPlainTextEdit;
    log_view->setReadOnly(true);
    log_view->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    log_view->setPlaceholderText("Вывод qDebug()");

    auto* log_box = new QGroupBox("Вывод qDebug()");
    auto* log_layout = new QVBoxLayout(log_box);
    log_layout->addWidget(log_view);

    auto* splitter = new QSplitter(Qt::Vertical);
    splitter->addWidget(tabs);
    splitter->addWidget(log_box);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);
    setCentralWidget(splitter);

    instance = this;
    previous_handler = qInstallMessageHandler(message_handler);

    vehicle_list->setCurrentRow(0);
    update_sensor_view();
}

MainWindow::~MainWindow() {
    qInstallMessageHandler(previous_handler);
    instance = nullptr;
}

void MainWindow::append_log(const QString& line) {
    if (instance == nullptr || instance->log_view == nullptr) {
        return;
    }
    const QString escaped = line.toHtmlEscaped().replace("\n", "<br>");
    const bool error = line.startsWith("Error");
    const bool header = line.startsWith("=====");
    QString html = escaped;
    if (error) {
        html = "<span style='color:#d0392b'>" + escaped + "</span>";
    } else if (header) {
        html = "<b>" + escaped + "</b>";
    }
    instance->log_view->appendHtml(html);
}

// ---------- Задание 1 ----------

QWidget* MainWindow::build_uav_tab() {
    auto* page = new QWidget;
    auto* layout = new QHBoxLayout(page);

    auto* panel = new QWidget;
    panel->setFixedWidth(300);
    auto* panel_layout = new QVBoxLayout(panel);
    panel_layout->setContentsMargins(0, 0, 0, 0);

    auto* list_box = new QGroupBox("Аппарат (std::vector<UAV*>)");
    auto* list_layout = new QVBoxLayout(list_box);
    vehicle_list = new QListWidget;
    for (const UAV* vehicle : fleet) {
        vehicle_list->addItem(QString("%1 — моторов: %2").arg(vehicle->type()).arg(vehicle->motor_count()));
    }
    vehicle_list->setMaximumHeight(90);
    list_layout->addWidget(vehicle_list);

    auto* takeoff_button = new QPushButton("Взлёт — takeoff()");
    auto* land_button = new QPushButton("Посадка — land()");
    auto* fleet_button = new QPushButton("Испытать весь парк");
    list_layout->addWidget(takeoff_button);
    list_layout->addWidget(land_button);
    list_layout->addWidget(fleet_button);

    auto* info_box = new QGroupBox("print_vehicle_info(const UAV&&)");
    auto* info_layout = new QVBoxLayout(info_box);
    vehicle_info = new QLabel;
    vehicle_info->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    vehicle_info->setTextInteractionFlags(Qt::TextSelectableByMouse);
    info_layout->addWidget(vehicle_info);

    auto* hint = new QLabel(
        "3D-модель строится только по методам интерфейса UAV: "
        "число лучей и винтов — motor_count(), подпись — type(), "
        "взлёт и посадка — takeoff()/land(). Проверок типа нет.");
    hint->setWordWrap(true);
    hint->setStyleSheet("color: gray;");

    panel_layout->addWidget(list_box);
    panel_layout->addWidget(info_box);
    panel_layout->addWidget(hint);
    panel_layout->addStretch();

    scene = new UavScene(this);
    auto* view = new QQuickWidget;
    view->setResizeMode(QQuickWidget::SizeRootObjectToView);
    view->rootContext()->setContextProperty("uav", scene);
    view->setSource(QUrl("qrc:/uav_scene.qml"));

    layout->addWidget(panel);
    layout->addWidget(view, 1);

    connect(vehicle_list, &QListWidget::currentRowChanged, this, &MainWindow::select_vehicle);
    connect(takeoff_button, &QPushButton::clicked, scene, &UavScene::takeoff);
    connect(land_button, &QPushButton::clicked, scene, &UavScene::land);
    connect(fleet_button, &QPushButton::clicked, this, &MainWindow::run_fleet_test);
    connect(scene, &UavScene::changed, this, &MainWindow::update_vehicle_info);
    return page;
}

void MainWindow::select_vehicle(int row) {
    if (row < 0 || row >= static_cast<int>(fleet.size())) {
        scene->set_vehicle(nullptr);
        return;
    }
    scene->set_vehicle(fleet[row]);
    qDebug().noquote() << "----- Selected vehicle -----";
    print_vehicle_info(*fleet[row]);
}

void MainWindow::run_fleet_test() {
    qDebug().noquote() << "===== Fleet test: print_motor_summary() =====";
    print_motor_summary(fleet);
}

void MainWindow::update_vehicle_info() {
    const UAV* vehicle = scene->vehicle();
    if (vehicle == nullptr) {
        vehicle_info->setText("Error: vehicle is not set");
        return;
    }
    vehicle_info->setText(vehicle_info_text(*vehicle) + "\nState: "
                          + (vehicle->is_airborne() ? "airborne" : "on the ground"));
}

// ---------- Задание 2 ----------

QWidget* MainWindow::build_sensor_tab() {
    auto* page = new QWidget;
    auto* layout = new QHBoxLayout(page);

    auto* panel = new QWidget;
    panel->setFixedWidth(360);
    auto* panel_layout = new QVBoxLayout(panel);
    panel_layout->setContentsMargins(0, 0, 0, 0);

    auto* box = new QGroupBox("Состояние сенсоров (std::vector<Sensor*>)");
    auto* grid = new QGridLayout(box);
    grid->addWidget(new QLabel("<b>Сенсор</b>"), 0, 0);
    grid->addWidget(new QLabel("<b>Вкл.</b>"), 0, 1);
    grid->addWidget(new QLabel("<b>Спутники / точки</b>"), 0, 2);
    grid->addWidget(new QLabel("<b>Ошибка, м</b>"), 0, 3);

    auto make_count = [](int maximum) {
        auto* spin = new QSpinBox;
        spin->setRange(0, maximum);
        return spin;
    };
    auto make_accuracy = []() {
        auto* spin = new QDoubleSpinBox;
        spin->setRange(0.1, 20.0);
        spin->setDecimals(1);
        spin->setSingleStep(0.1);
        return spin;
    };

    gps_enabled = new QCheckBox;
    gps_satellites = make_count(30);
    gps_accuracy = make_accuracy();
    glonass_enabled = new QCheckBox;
    glonass_satellites = make_count(30);
    glonass_accuracy = make_accuracy();
    vo_enabled = new QCheckBox;
    vo_features = make_count(500);
    vo_accuracy = make_accuracy();

    grid->addWidget(new QLabel("GPS"), 1, 0);
    grid->addWidget(gps_enabled, 1, 1);
    grid->addWidget(gps_satellites, 1, 2);
    grid->addWidget(gps_accuracy, 1, 3);
    grid->addWidget(new QLabel("GLONASS"), 2, 0);
    grid->addWidget(glonass_enabled, 2, 1);
    grid->addWidget(glonass_satellites, 2, 2);
    grid->addWidget(glonass_accuracy, 2, 3);
    grid->addWidget(new QLabel("Visual odometry"), 3, 0);
    grid->addWidget(vo_enabled, 3, 1);
    grid->addWidget(vo_features, 3, 2);
    grid->addWidget(vo_accuracy, 3, 3);

    auto* rules = new QLabel(
        QString("Готовность: GPS/GLONASS — включён и спутников ≥ %1; "
                "визуальная одометрия — включена и точек ≥ %2.\n"
                "choose_best_sensor() выбирает готовый сенсор с минимальной ошибкой.")
            .arg(SatelliteSensor::min_satellites)
            .arg(VisualOdometry::min_features));
    rules->setWordWrap(true);
    rules->setStyleSheet("color: gray;");

    auto* reset_button = new QPushButton("Контрольные значения");
    auto* gps_fail_button = new QPushButton("Отказ GPS (3 спутника)");
    auto* all_fail_button = new QPushButton("Отказ всех сенсоров");
    auto* print_button = new QPushButton("print_sensors() в журнал");

    panel_layout->addWidget(box);
    panel_layout->addWidget(rules);
    panel_layout->addWidget(reset_button);
    panel_layout->addWidget(gps_fail_button);
    panel_layout->addWidget(all_fail_button);
    panel_layout->addWidget(print_button);
    panel_layout->addStretch();

    sensor_map = new SensorMapWidget;
    layout->addWidget(panel);
    layout->addWidget(sensor_map, 1);

    reset_sensors();

    for (QCheckBox* check : {gps_enabled, glonass_enabled, vo_enabled}) {
        connect(check, &QCheckBox::toggled, this, &MainWindow::apply_sensor_inputs);
    }
    for (QSpinBox* spin : {gps_satellites, glonass_satellites, vo_features}) {
        connect(spin, &QSpinBox::valueChanged, this, &MainWindow::apply_sensor_inputs);
    }
    for (QDoubleSpinBox* spin : {gps_accuracy, glonass_accuracy, vo_accuracy}) {
        connect(spin, &QDoubleSpinBox::valueChanged, this, &MainWindow::apply_sensor_inputs);
    }
    connect(reset_button, &QPushButton::clicked, this, [this]() {
        reset_sensors();
        apply_sensor_inputs();
    });
    connect(gps_fail_button, &QPushButton::clicked, this, [this]() {
        gps_satellites->setValue(3);
    });
    connect(all_fail_button, &QPushButton::clicked, this, [this]() {
        const QSignalBlocker b1(gps_enabled), b2(glonass_enabled), b3(vo_enabled);
        gps_enabled->setChecked(false);
        glonass_enabled->setChecked(false);
        vo_enabled->setChecked(false);
        apply_sensor_inputs();
    });
    connect(print_button, &QPushButton::clicked, this, [this]() {
        qDebug().noquote() << "===== print_sensors() =====";
        print_sensors(sensors);
    });
    return page;
}

// Значения в полях ввода — контрольные значения варианта 2
void MainWindow::reset_sensors() {
    const QSignalBlocker blockers[] = {
        QSignalBlocker(gps_enabled), QSignalBlocker(gps_satellites), QSignalBlocker(gps_accuracy),
        QSignalBlocker(glonass_enabled), QSignalBlocker(glonass_satellites), QSignalBlocker(glonass_accuracy),
        QSignalBlocker(vo_enabled), QSignalBlocker(vo_features), QSignalBlocker(vo_accuracy)
    };
    gps_enabled->setChecked(true);
    gps_satellites->setValue(10);
    gps_accuracy->setValue(1.5);
    glonass_enabled->setChecked(true);
    glonass_satellites->setValue(8);
    glonass_accuracy->setValue(2.0);
    vo_enabled->setChecked(true);
    vo_features->setValue(60);
    vo_accuracy->setValue(3.2);
}

// Поля формы передаются в объекты только через их сеттеры
void MainWindow::apply_sensor_inputs() {
    gps.set_enabled(gps_enabled->isChecked());
    gps.set_satellites(gps_satellites->value());
    gps.set_accuracy(gps_accuracy->value());
    glonass.set_enabled(glonass_enabled->isChecked());
    glonass.set_satellites(glonass_satellites->value());
    glonass.set_accuracy(glonass_accuracy->value());
    visual_odometry.set_enabled(vo_enabled->isChecked());
    visual_odometry.set_tracked_features(vo_features->value());
    visual_odometry.set_accuracy(vo_accuracy->value());
    update_sensor_view();
}

void MainWindow::update_sensor_view() {
    Sensor* best = choose_best_sensor(sensors);
    if (best != last_best && best != nullptr) {
        qDebug().noquote() << "Best navigation source:" << best->name();
    }
    last_best = best;
    sensor_map->set_sensors(sensors, best);
}
