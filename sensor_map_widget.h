#ifndef SENSOR_MAP_WIDGET_H
#define SENSOR_MAP_WIDGET_H

#include <QWidget>

#include <vector>

class Sensor;

// Визуализация задания 2: круги ошибки каждого навигационного сенсора
// вокруг истинного положения аппарата и столбчатая диаграмма точности.
// Виджет знает о сенсорах только интерфейс Sensor.
class SensorMapWidget : public QWidget {
    Q_OBJECT

public:
    explicit SensorMapWidget(QWidget* parent = nullptr);

    void set_sensors(const std::vector<Sensor*>& list, Sensor* best_sensor);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void draw_map(QPainter& painter, const QRectF& area) const;
    void draw_bars(QPainter& painter, const QRectF& area) const;
    static QColor sensor_color(int index);

    std::vector<Sensor*> sensors;
    Sensor* best = nullptr;
};

#endif
