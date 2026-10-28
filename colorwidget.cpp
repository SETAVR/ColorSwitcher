#include "colorwidget.h"

#include <QDebug>
#include <QMouseEvent>
#include <QPainter>
#include <QRandomGenerator>

ColorWidget::ColorWidget(QWidget *parent)
    : QWidget(parent), m_palette(defaultPalette())
{
    setObjectName("canvas");
    setMinimumSize(320, 220);
    qDebug() << "[CREATE] ColorWidget, palette:" << m_palette.size();
}

ColorWidget::~ColorWidget()
{
    qDebug() << "[DESTROY] ColorWidget";
}

QVector<QColor> ColorWidget::defaultPalette()
{
    return {QColor("#e74c3c"), QColor("#e67e22"), QColor("#f1c40f"),
            QColor("#2ecc71"), QColor("#3498db"), QColor("#9b59b6")};
}

QColor ColorWidget::currentColor() const
{
    // The signal and log must describe the displayed colour, including black.
    if (m_blinkOn)
        return QColor(Qt::black);
    return m_palette.isEmpty() ? QColor(Qt::gray) : m_palette.at(m_index);
}

void ColorWidget::addColor(const QColor &color)
{
    if (!color.isValid())
        return;
    m_palette.append(color);
    qDebug() << "[PALETTE] Added" << color.name() << "total:" << m_palette.size();
}

void ColorWidget::resetPalette()
{
    m_palette = defaultPalette();
    m_index = 0;
    m_blinkOn = false;
    emitCurrent();
}

void ColorWidget::stepForward()
{
    if (m_palette.isEmpty()) return;
    m_index = (m_index + 1) % int(m_palette.size());
    m_blinkOn = false;
    emitCurrent();
}

void ColorWidget::stepBackward()
{
    if (m_palette.isEmpty()) return;
    m_index = (m_index - 1 + int(m_palette.size())) % int(m_palette.size());
    m_blinkOn = false;
    emitCurrent();
}

void ColorWidget::setRandomColor()
{
    if (m_palette.isEmpty()) return;
    m_index = QRandomGenerator::global()->bounded(int(m_palette.size()));
    m_blinkOn = false;
    emitCurrent();
}

void ColorWidget::toggleBlink()
{
    m_blinkOn = !m_blinkOn;
    emitCurrent();
}

void ColorWidget::stopBlink()
{
    if (m_blinkOn) {
        m_blinkOn = false;
        emitCurrent();
    }
}

void ColorWidget::emitCurrent()
{
    update();
    emit colorChanged(currentColor());
}

void ColorWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    const QColor color = currentColor();
    painter.fillRect(rect(), color);
    painter.setPen(color.lightness() > 128 ? Qt::black : Qt::white);
    painter.drawText(rect(), Qt::AlignCenter, color.name());
}

void ColorWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        qDebug() << "[MOUSE] Left click";
        stepForward();
    } else if (event->button() == Qt::RightButton) {
        qDebug() << "[MOUSE] Right click";
        stepBackward();
    } else {
        QWidget::mousePressEvent(event);
        return;
    }
    event->accept();
}
