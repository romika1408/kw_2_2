#include "sensor_map_widget.h"

#include "sensor.h"

#include <QPainter>
#include <QPainterPath>

#include <algorithm>
#include <cmath>

namespace {
constexpr double pi = 3.14159265358979323846;
}

SensorMapWidget::SensorMapWidget(QWidget* parent)
    : QWidget(parent) {
    setMinimumSize(560, 380);
}

void SensorMapWidget::set_sensors(const std::vector<Sensor*>& list, Sensor* best_sensor) {
    sensors = list;
    best = best_sensor;
    update();
}

QColor SensorMapWidget::sensor_color(int index) {
    static const QColor palette[] = {
        QColor("#4fa3ff"), QColor("#ff6b6b"), QColor("#ffc145"), QColor("#7fe08a")
    };
    return palette[index % 4];
}

void SensorMapWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor("#1d2430"));

    // Заголовок: результат choose_best_sensor()
    QFont title = font();
    title.setPointSize(13);
    title.setBold(true);
    painter.setFont(title);
    if (best != nullptr) {
        painter.setPen(QColor("#7fe08a"));
        painter.drawText(QRectF(16, 8, width() - 32, 30), Qt::AlignVCenter,
                         QString("Лучший источник: %1 (ошибка %2 м)")
                             .arg(best->name())
                             .arg(best->accuracy(), 0, 'f', 1));
    } else {
        painter.setPen(QColor("#ff6b6b"));
        painter.drawText(QRectF(16, 8, width() - 32, 30), Qt::AlignVCenter,
                         "Ошибка: ни один сенсор не готов (choose_best_sensor → nullptr)");
    }

    const QRectF content(16, 46, width() - 32, height() - 62);
    const QRectF map_area(content.left(), content.top(), content.width() * 0.58, content.height());
    const QRectF bars_area(map_area.right() + 16, content.top(),
                           content.right() - map_area.right() - 16, content.height());
    draw_map(painter, map_area);
    draw_bars(painter, bars_area);
}

// Карта: в центре истинное положение, вокруг — оценки сенсоров.
// Каждая оценка смещена в своём направлении, радиус круга равен ошибке accuracy().
void SensorMapWidget::draw_map(QPainter& painter, const QRectF& area) const {
    painter.save();
    painter.setPen(QColor("#2f3a4a"));
    painter.setBrush(QColor("#222b38"));
    painter.drawRoundedRect(area, 8, 8);

    double max_extent = 1.0;
    for (const Sensor* sensor : sensors) {
        if (sensor != nullptr) {
            max_extent = std::max(max_extent, sensor->accuracy() * 1.6);
        }
    }
    const QPointF center = area.center();
    const double px_per_m = (std::min(area.width(), area.height()) / 2.0 - 18) / max_extent;

    // Сетка с шагом 1 м
    painter.setPen(QPen(QColor("#2c3644"), 1));
    const int lines = static_cast<int>(max_extent) + 1;
    for (int i = -lines; i <= lines; ++i) {
        const double offset = i * px_per_m;
        if (std::abs(offset) < area.width() / 2) {
            painter.drawLine(QPointF(center.x() + offset, area.top() + 4),
                             QPointF(center.x() + offset, area.bottom() - 4));
        }
        if (std::abs(offset) < area.height() / 2) {
            painter.drawLine(QPointF(area.left() + 4, center.y() + offset),
                             QPointF(area.right() - 4, center.y() + offset));
        }
    }

    QFont small = painter.font();
    small.setPointSize(9);
    small.setBold(false);
    painter.setFont(small);

    for (int i = 0; i < static_cast<int>(sensors.size()); ++i) {
        const Sensor* sensor = sensors[i];
        if (sensor == nullptr) {
            continue;
        }
        const double angle = -pi / 2 + i * 2 * pi / sensors.size();
        const double shift = sensor->accuracy() * 0.6 * px_per_m;
        const QPointF estimate(center.x() + shift * std::cos(angle),
                               center.y() + shift * std::sin(angle));
        const double radius = sensor->accuracy() * px_per_m;

        QColor color = sensor->is_ready() ? sensor_color(i) : QColor("#6b7383");
        const bool is_best = sensor == best;

        QColor fill = color;
        fill.setAlpha(is_best ? 70 : 28);
        QPen pen(color, is_best ? 3 : 1.5);
        if (!sensor->is_ready()) {
            pen.setStyle(Qt::DashLine);
        }
        painter.setPen(pen);
        painter.setBrush(fill);
        painter.drawEllipse(estimate, radius, radius);

        painter.setBrush(color);
        painter.drawEllipse(estimate, 4, 4);

        const QString label = sensor->name()
                              + (sensor->is_ready() ? "" : " — не готов")
                              + (is_best ? "  ★" : "");
        const QPointF label_pos(estimate.x() + std::cos(angle) * (radius + 6) - 40,
                                estimate.y() + std::sin(angle) * (radius + 6) + 4);
        painter.setPen(color);
        painter.drawText(label_pos, label);
    }

    // Истинное положение аппарата
    painter.setPen(QPen(Qt::white, 2));
    painter.drawLine(center + QPointF(-9, 0), center + QPointF(9, 0));
    painter.drawLine(center + QPointF(0, -9), center + QPointF(0, 9));
    painter.drawText(center + QPointF(10, 18), "истинное положение");

    // Масштабная линейка 1 м
    painter.setPen(QPen(QColor("#c9ced6"), 2));
    const QPointF ruler(area.left() + 14, area.bottom() - 14);
    painter.drawLine(ruler, ruler + QPointF(px_per_m, 0));
    painter.drawText(ruler + QPointF(0, -6), "1 м");

    painter.restore();
}

// Диаграмма: ошибка каждого сенсора (меньше — лучше) и показание read().
void SensorMapWidget::draw_bars(QPainter& painter, const QRectF& area) const {
    painter.save();
    painter.setPen(QColor("#2f3a4a"));
    painter.setBrush(QColor("#222b38"));
    painter.drawRoundedRect(area, 8, 8);

    QFont header = painter.font();
    header.setPointSize(10);
    header.setBold(true);
    painter.setFont(header);
    painter.setPen(QColor("#c9ced6"));
    painter.drawText(QRectF(area.left() + 12, area.top() + 8, area.width() - 24, 20),
                     "Ошибка accuracy(), м — меньше лучше");

    double max_error = 0.5;
    for (const Sensor* sensor : sensors) {
        if (sensor != nullptr) {
            max_error = std::max(max_error, sensor->accuracy());
        }
    }

    QFont small = painter.font();
    small.setPointSize(9);
    small.setBold(false);
    painter.setFont(small);

    const double row_height = std::min(78.0, (area.height() - 40) / std::max<size_t>(1, sensors.size()));
    const double bar_left = area.left() + 12;
    const double bar_max = area.width() - 70;

    for (int i = 0; i < static_cast<int>(sensors.size()); ++i) {
        Sensor* sensor = sensors[i];
        const double top = area.top() + 36 + i * row_height;
        if (sensor == nullptr) {
            painter.setPen(QColor("#ff6b6b"));
            painter.drawText(QPointF(bar_left, top + 14), "Error: sensor is not set");
            continue;
        }
        const bool ready = sensor->is_ready();
        const QColor color = ready ? sensor_color(i) : QColor("#6b7383");

        painter.setPen(color);
        painter.drawText(QPointF(bar_left, top + 12),
                         sensor->name() + (sensor == best ? "  ★ выбран" : ""));

        const double bar_width = bar_max * sensor->accuracy() / max_error;
        painter.setPen(Qt::NoPen);
        painter.setBrush(ready ? color : QColor("#3a4250"));
        painter.drawRoundedRect(QRectF(bar_left, top + 18, bar_width, 16), 4, 4);

        painter.setPen(QColor("#e6e9ee"));
        painter.drawText(QPointF(bar_left + bar_width + 6, top + 31),
                         QString::number(sensor->accuracy(), 'f', 1) + " м");

        // read() вызывается только у готового сенсора, иначе он сообщит об ошибке
        const QString reading = ready
            ? QString("read() = %1 м, %2").arg(sensor->read(), 0, 'f', 1).arg(sensor->details())
            : QString("не готов: %1").arg(sensor->details());
        painter.setPen(QColor("#8a93a3"));
        painter.drawText(QPointF(bar_left, top + 50), reading);
    }

    painter.restore();
}
