#include "mainwindow.h"

#include <QDebug>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRandomGenerator>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) : QWidget(parent)
{
    m_timer.setParent(this);
    m_timer.setObjectName("timer");
    m_timer.setInterval(600);
    setupUi();
    rebuildTimeoutConnection();
    reconnectCanvasHandler();
    // A one-shot callback with a context: cancelled if the window is destroyed.
    QTimer::singleShot(0, this, [this] {
        setStatus(QStringLiteral("Готово. Нажмите «Старт/Стоп», чтобы запустить таймер."));
        qDebug() << "[ONCE] Initial status";
    });
}

MainWindow::~MainWindow()
{
    m_timer.stop();
    qDebug() << "[DESTROY] MainWindow";
}

void MainWindow::setupUi()
{
    setWindowTitle(QStringLiteral("ColorSwitcher — QTimer и динамические соединения"));
    resize(620, 600);
    auto *root = new QVBoxLayout(this);
    m_canvas.setParent(this);
    root->addWidget(&m_canvas, 1);
    m_statusLabel = new QLabel(this);
    m_statusLabel->setObjectName("status");
    m_statusLabel->setWordWrap(true);
    m_statusLabel->setMinimumHeight(40);
    root->addWidget(m_statusLabel);
    m_intervalLabel = new QLabel(this);
    m_intervalLabel->setObjectName("interval");
    m_modeLabel = new QLabel(this);
    m_modeLabel->setObjectName("mode");
    root->addWidget(m_intervalLabel);
    root->addWidget(m_modeLabel);

    auto *controls = new QHBoxLayout;
    auto *start = new QPushButton(QStringLiteral("Старт/Стоп"), this);
    start->setObjectName("start");
    auto *slow = new QPushButton(QStringLiteral("Медленнее"), this);
    slow->setObjectName("slow");
    auto *fast = new QPushButton(QStringLiteral("Быстрее"), this);
    fast->setObjectName("fast");
    auto *mode = new QPushButton(QStringLiteral("Режим"), this);
    mode->setObjectName("modeButton");
    for (auto *button : {start, slow, fast, mode}) controls->addWidget(button);
    root->addLayout(controls);
    connect(start, &QPushButton::clicked, this, &MainWindow::toggleTimer);
    connect(slow, &QPushButton::clicked, this, &MainWindow::decreaseInterval);
    connect(fast, &QPushButton::clicked, this, &MainWindow::increaseInterval);
    connect(mode, &QPushButton::clicked, this, &MainWindow::cycleMode);

    root->addWidget(new QLabel(QStringLiteral("Палитра (кнопки создаются динамически):"), this));
    m_paletteGrid = new QGridLayout;
    root->addLayout(m_paletteGrid);
    auto *paletteControls = new QHBoxLayout;
    auto *add = new QPushButton(QStringLiteral("+ Цвет"), this);
    add->setObjectName("add");
    auto *clear = new QPushButton(QStringLiteral("Очистить палитру"), this);
    clear->setObjectName("clear");
    auto *handler = new QPushButton(QStringLiteral("Сменить обработчик клика"), this);
    handler->setObjectName("handler");
    for (auto *button : {add, clear, handler}) paletteControls->addWidget(button);
    root->addLayout(paletteControls);
    connect(add, &QPushButton::clicked, this, &MainWindow::addColorButton);
    connect(clear, &QPushButton::clicked, this, &MainWindow::clearPaletteButtons);
    connect(handler, &QPushButton::clicked, this, &MainWindow::toggleClickHandler);
    updateIntervalLabel();
    updateModeLabel();
}

void MainWindow::setStatus(const QString &text)
{
    m_statusLabel->setText(text);
}

void MainWindow::updateIntervalLabel()
{
    m_intervalLabel->setText(QStringLiteral("Интервал: %1 мс").arg(m_timer.interval()));
}

void MainWindow::updateModeLabel()
{
    m_modeLabel->setText(QStringLiteral("Режим: %1").arg(modeName()));
}

QString MainWindow::modeName() const
{
    switch (m_mode) {
    case Mode::Cycle: return QStringLiteral("Цикл");
    case Mode::Random: return QStringLiteral("Случайный");
    case Mode::Blink: return QStringLiteral("Мигание");
    }
    return {};
}

void MainWindow::toggleTimer()
{
    if (m_timer.isActive()) {
        m_timer.stop();
        setStatus(QStringLiteral("Таймер остановлен."));
        qDebug() << "[TIMER] stop()";
    } else {
        m_timer.start();
        setStatus(QStringLiteral("Таймер запущен (интервал %1 мс).").arg(m_timer.interval()));
        qDebug() << "[TIMER] start(), interval =" << m_timer.interval();
    }
}

void MainWindow::decreaseInterval()
{
    m_timer.setInterval(qMin(m_timer.interval() + 200, 5000));
    updateIntervalLabel();
    qDebug() << "[TIMER] interval ->" << m_timer.interval();
}

void MainWindow::increaseInterval()
{
    m_timer.setInterval(qMax(m_timer.interval() - 200, 50));
    updateIntervalLabel();
    qDebug() << "[TIMER] interval ->" << m_timer.interval();
}

void MainWindow::cycleMode()
{
    m_canvas.stopBlink();
    switch (m_mode) {
    case Mode::Cycle: m_mode = Mode::Random; break;
    case Mode::Random: m_mode = Mode::Blink; break;
    case Mode::Blink: m_mode = Mode::Cycle; break;
    }
    updateModeLabel();
    rebuildTimeoutConnection();
    setStatus(QStringLiteral("Режим переключён: %1").arg(modeName()));
    qDebug() << "[TIMER] mode ->" << modeName() << "(reconnected timeout)";
}

void MainWindow::rebuildTimeoutConnection()
{
    // Keep only one timeout handler; UniqueConnection does not deduplicate lambdas.
    if (m_timeoutConn) disconnect(m_timeoutConn);
    m_timeoutConn = connect(&m_timer, &QTimer::timeout, this, [this] {
        switch (m_mode) {
        case Mode::Cycle: m_canvas.stepForward(); break;
        case Mode::Random: m_canvas.setRandomColor(); break;
        case Mode::Blink: m_canvas.toggleBlink(); break;
        }
        qDebug() << "[TIMER] timeout, mode =" << modeName()
                 << "color =" << m_canvas.currentColor().name();
    });
}

void MainWindow::reconnectCanvasHandler()
{
    if (m_clickConn) disconnect(m_clickConn);
    if (!m_clickHandlerAlt) {
        m_clickConn = connect(&m_canvas, &ColorWidget::colorChanged, this, [this](const QColor &color) {
            setStatus(QStringLiteral("Цвет холста: %1").arg(color.name()));
        });
    } else {
        m_clickConn = connect(&m_canvas, &ColorWidget::colorChanged, this, [this](const QColor &color) {
            setWindowTitle(QStringLiteral("ColorSwitcher — %1").arg(color.name()));
        });
    }
}

void MainWindow::toggleClickHandler()
{
    m_clickHandlerAlt = !m_clickHandlerAlt;
    reconnectCanvasHandler();
    setStatus(m_clickHandlerAlt ? QStringLiteral("Обработчик Б: заголовок окна.")
                               : QStringLiteral("Обработчик А: строка статуса."));
    qDebug() << "[CANVAS] Reconnected colorChanged, alternative =" << m_clickHandlerAlt;
}

void MainWindow::addColorButton()
{
    const QColor color = QColor::fromHsv(QRandomGenerator::global()->bounded(360),
                                        200 + QRandomGenerator::global()->bounded(55),
                                        200 + QRandomGenerator::global()->bounded(55));
    auto *button = new PaletteButton(color, this);
    const int index = int(m_paletteButtons.size());
    m_paletteGrid->addWidget(button, index / 4, index % 4);
    // Capture the colour by value; the lambda never dereferences a button pointer.
    const auto connection = connect(button, &QPushButton::clicked, this, [this, color] {
        m_canvas.addColor(color);
        setStatus(QStringLiteral("Добавлен цвет %1 в палитру холста.").arg(color.name()));
        qDebug() << "[DYNAMIC] Clicked colour button" << color.name();
    });
    m_paletteButtons.append(QPointer<PaletteButton>(button));
    m_dynamicConnections.append(connection);
    setStatus(QStringLiteral("Создана кнопка-цвет %1.").arg(color.name()));
    qDebug() << "[DYNAMIC] Connected" << color.name() << "total:" << m_dynamicConnections.size();
}

void MainWindow::clearPaletteButtons()
{
    const int count = int(m_dynamicConnections.size());
    for (const auto &connection : m_dynamicConnections) disconnect(connection);
    m_dynamicConnections.clear();
    for (const auto &button : m_paletteButtons) {
        if (button) {
            m_paletteGrid->removeWidget(button.data());
            button->hide();
            button->deleteLater();
        }
    }
    m_paletteButtons.clear();
    m_canvas.resetPalette();
    setStatus(QStringLiteral("Палитра очищена: соединения разорваны, кнопки удалены."));
    qDebug() << "[DYNAMIC] Cleared palette, disconnected:" << count;
}
