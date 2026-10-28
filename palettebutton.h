#ifndef PALETTEBUTTON_H
#define PALETTEBUTTON_H

#include <QColor>
#include <QPushButton>

class PaletteButton : public QPushButton
{
    Q_OBJECT
public:
    explicit PaletteButton(const QColor &color, QWidget *parent = nullptr);
    ~PaletteButton() override;
    QColor color() const { return m_color; }

private:
    QColor m_color;
};

#endif
