#ifndef COLORWIDGET_H
#define COLORWIDGET_H

#include <QColor>
#include <QVector>
#include <QWidget>

class ColorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ColorWidget(QWidget *parent = nullptr);
    ~ColorWidget() override;
    void addColor(const QColor &color);
    void resetPalette();
    QColor currentColor() const;
    void stepForward();
    void stepBackward();
    void setRandomColor();
    void toggleBlink();
    void stopBlink();

signals:
    void colorChanged(const QColor &color);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    static QVector<QColor> defaultPalette();
    void emitCurrent();
    QVector<QColor> m_palette;
    int m_index = 0;
    bool m_blinkOn = false;
};

#endif
