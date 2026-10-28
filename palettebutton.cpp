#include "palettebutton.h"

#include <QDebug>

PaletteButton::PaletteButton(const QColor &color, QWidget *parent)
    : QPushButton(parent), m_color(color)
{
    setText(color.name());
    setToolTip(QStringLiteral("Добавить этот цвет в палитру холста"));
    qDebug() << "[CREATE] PaletteButton" << color.name();
}

PaletteButton::~PaletteButton()
{
    qDebug() << "[DESTROY] PaletteButton" << m_color.name();
}
