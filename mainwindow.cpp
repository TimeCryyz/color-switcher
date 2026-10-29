#include "mainwindow.h"
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRandomGenerator>
#include <QVBoxLayout>
#include <QTextStream>
#include <QStringConverter>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent), m_mode(Mode::Cycle), m_clickHandlerAlt(false)
{
    setupUi();
    m_timer.setInterval(600);
    rebuildTimeoutConnection();
    reconnectCanvasHandler();
    setStatus("Готово. Нажмите «Старт/Стоп», чтобы запустить таймер.");
}

MainWindow::~MainWindow()
{
    QTextStream out(stdout);
    out.setEncoding(QStringConverter::Utf8);
    out << "[DESTROY] MainWindow\n";
    m_timer.stop();
}

void MainWindow::setupUi()
{
    setWindowTitle("QTimer + лямбда-слоты + динамические соединения");
    resize(560, 640);

    m_rootLayout = new QVBoxLayout(this);
    m_canvas.setParent(this);
    m_rootLayout->addWidget(&m_canvas);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setWordWrap(true);
    m_rootLayout->addWidget(m_statusLabel);

    m_intervalLabel = new QLabel(this);
    m_modeLabel = new QLabel(this);
    m_rootLayout->addWidget(m_intervalLabel);
    m_rootLayout->addWidget(m_modeLabel);

    auto *controls = new QHBoxLayout();
    auto *btnStart = new QPushButton("Старт/Стоп", this);
    auto *btnSlow = new QPushButton("Медленнее", this);
    auto *btnFast = new QPushButton("Быстрее", this);
    auto *btnMode = new QPushButton("Режим", this);
    controls->addWidget(btnStart);
    controls->addWidget(btnSlow);
    controls->addWidget(btnFast);
    controls->addWidget(btnMode);
    m_rootLayout->addLayout(controls);

    connect(btnStart, &QPushButton::clicked, this, &MainWindow::toggleTimer);
    connect(btnSlow, &QPushButton::clicked, this, &MainWindow::decreaseInterval);
    connect(btnFast, &QPushButton::clicked, this, &MainWindow::increaseInterval);
    connect(btnMode, &QPushButton::clicked, this, &MainWindow::cycleMode);

    auto *palHeader = new QLabel("Палитра (кнопки создаются динамически):", this);
    m_rootLayout->addWidget(palHeader);

    m_paletteGrid = new QGridLayout();
    m_rootLayout->addLayout(m_paletteGrid);

    auto *palControls = new QHBoxLayout();
    auto *btnAdd = new QPushButton("+ Цвет", this);
    auto *btnClear = new QPushButton("Очистить палитру", this);
    auto *btnClick = new QPushButton("Сменить обработчик клика", this);
    palControls->addWidget(btnAdd);
    palControls->addWidget(btnClear);
    palControls->addWidget(btnClick);
    m_rootLayout->addLayout(palControls);

    connect(btnAdd, &QPushButton::clicked, this, &MainWindow::addColorButton);
    connect(btnClear, &QPushButton::clicked, this, &MainWindow::clearPaletteButtons);
    connect(btnClick, &QPushButton::clicked, this, &MainWindow::toggleClickHandler);

    updateIntervalLabel();
    updateModeLabel();
}

void MainWindow::toggleTimer()
{
    QTextStream out(stdout);
    out.setEncoding(QStringConverter::Utf8);

    if (m_timer.isActive()) {
        m_timer.stop();
        setStatus("Таймер остановлен.");
        out << "[TIMER] isActive: false\n";
    } else {
        m_timer.start();
        setStatus(QString("Таймер запущен (интервал %1 мс).").arg(m_timer.interval()));
        out << "[TIMER] isActive: true\n";
    }
}

void MainWindow::decreaseInterval()
{
    m_timer.setInterval(qMin(m_timer.interval() + 200, 5000));
    updateIntervalLabel();

    QTextStream out(stdout);
    out.setEncoding(QStringConverter::Utf8);
    out << "[TIMER] interval -> " << m_timer.interval() << "\n";
}

void MainWindow::increaseInterval()
{
    m_timer.setInterval(qMax(m_timer.interval() - 200, 50));
    updateIntervalLabel();

    QTextStream out(stdout);
    out.setEncoding(QStringConverter::Utf8);
    out << "[TIMER] interval -> " << m_timer.interval() << "\n";
}

void MainWindow::cycleMode()
{
    switch (m_mode) {
    case Mode::Cycle: m_mode = Mode::Random; break;
    case Mode::Random: m_mode = Mode::Blink; break;
    case Mode::Blink: m_mode = Mode::Cycle; break;
    }
    updateModeLabel();
    rebuildTimeoutConnection();
    setStatus("Режим переключён: " + modeName());

    QTextStream out(stdout);
    out.setEncoding(QStringConverter::Utf8);
    out << "[TIMER] режим -> " << modeName() << "\n";
}

void MainWindow::rebuildTimeoutConnection()
{
    if (m_timeoutConn) {
        disconnect(m_timeoutConn);
    }
    m_timeoutConn = connect(&m_timer, &QTimer::timeout, this, [this]() {
        switch (m_mode) {
        case Mode::Cycle: m_canvas.stepForward(); break;
        case Mode::Random: m_canvas.setRandomColor(); break;
        case Mode::Blink: m_canvas.toggleBlink(); break;
        }

        QTextStream out(stdout);
        out.setEncoding(QStringConverter::Utf8);
        out << "[TIMER] timeout, режим = " << modeName() << ", цвет = " << m_canvas.currentColor().name() << "\n";
    });
}

void MainWindow::reconnectCanvasHandler()
{
    if (m_clickConn) {
        disconnect(m_clickConn);
    }
    if (!m_clickHandlerAlt) {
        m_clickConn = connect(&m_canvas, &ColorWidget::colorChanged, this, [this](const QColor &c) {
            setStatus("Цвет холста: " + c.name());
        });
    } else {
        m_clickConn = connect(&m_canvas, &ColorWidget::colorChanged, this, [this](const QColor &c) {
            setWindowTitle("ColorSwitcher - " + c.name());
        });
    }
}

void MainWindow::toggleClickHandler()
{
    m_clickHandlerAlt = !m_clickHandlerAlt;
    reconnectCanvasHandler();
    setStatus(m_clickHandlerAlt ? "Обработчик клика: вариант Б." : "Обработчик клика: вариант А.");

    QTextStream out(stdout);
    out.setEncoding(QStringConverter::Utf8);
    out << "[CANVAS] переподключение colorChanged, альтернатива = " << (m_clickHandlerAlt ? "true" : "false") << "\n";
}

void MainWindow::addColorButton()
{
    QColor color = QColor::fromHsv(
        QRandomGenerator::global()->bounded(360),
        200 + QRandomGenerator::global()->bounded(55),
        200 + QRandomGenerator::global()->bounded(55));

    auto *btn = new PaletteButton(color, this);
    const int row = m_paletteButtons.size() / 4;
    const int col = m_paletteButtons.size() % 4;
    m_paletteGrid->addWidget(btn, row, col);

    QMetaObject::Connection conn = connect(btn, &QPushButton::clicked, this, [this, btn]() {
        m_canvas.addColor(btn->color());
        setStatus("Добавлен цвет " + btn->color().name());
    });

    m_paletteButtons.append(QPointer<PaletteButton>(btn));
    m_dynamicConnections.append(conn);
}

void MainWindow::clearPaletteButtons()
{
    for (QMetaObject::Connection &c : m_dynamicConnections) {
        if (c) disconnect(c);
    }
    m_dynamicConnections.clear();

    for (QPointer<PaletteButton> &btn : m_paletteButtons) {
        if (!btn.isNull()) btn->deleteLater();
    }
    m_paletteButtons.clear();

    m_canvas.resetPalette();
    setStatus("Палитра очищена.");
}

void MainWindow::setStatus(const QString &text)
{
    m_statusLabel->setText(text);
}

void MainWindow::updateIntervalLabel()
{
    m_intervalLabel->setText(QString("Интервал: %1 мс").arg(m_timer.interval()));
}

void MainWindow::updateModeLabel()
{
    m_modeLabel->setText("Режим: " + modeName());
}

QString MainWindow::modeName() const
{
    switch (m_mode) {
    case Mode::Cycle: return "Цикл";
    case Mode::Random: return "Случайный";
    case Mode::Blink: return "Мигание";
    }
    return "";
}