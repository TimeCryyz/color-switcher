\# Лабораторная работа №3: QTimer, лямбда-слоты и динамические соединения



\*\*Студент:\*\* Филатов Вячеслав Викторович  

\*\*Дисциплина:\*\* Основы кроссплатформенного программирования  

\*\*Тема:\*\* Изучение QTimer, лямбда-функций и динамических соединений в Qt  

\*\*Технология:\*\* Qt 6 (C++), qmake



\---



\## 1. Цель работы

Освоить класс `QTimer` для периодического и однократного выполнения кода, научиться использовать лямбда-функции в качестве слотов с корректным захватом и контекстом подключения, а также создавать, разрывать и переподключать соединения между сигналами и слотами динамически, во время выполнения программы.



\## 2. Постановка задачи

Разработать приложение `ColorSwitcher`, которое:

\- Имеет цветной холст, меняющий цвет по таймеру в трех режимах (цикл, случайный цвет, мигание).

\- Позволяет пользователю добавлять новые цвета в палитру с помощью динамически создаваемых кнопок.

\- Реагирует на клики мыши по холсту, изменяя текущий цвет.

\- Позволяет динамически менять обработчик сигнала (переподключение).



\## 3. Теоретическое обоснование



\### 3.1. Метаобъектная система Qt и сигналы/слоты

В основе Qt лежит метаобъектная система. Классы, наследующие `QObject` и использующие макрос `Q\_OBJECT`, обрабатываются утилитой `moc` (Meta-Object Compiler). Это позволяет использовать механизм сигналов и слотов для связи объектов. Сигнал — это уведомление о событии, слот — функция-обработчик. Связь устанавливается функцией `connect()`.



\### 3.2. QTimer и цикл событий

`QTimer` — это класс для создания таймеров. Он излучает сигнал `timeout()` через заданные промежутки времени. Работа таймера невозможна без цикла событий (`app.exec()`), так как именно в нем происходит обработка всех системных сообщений и таймеров. В отличие от консольных программ с блокирующим вводом, GUI-приложение должно постоянно реагировать на действия пользователя.



\### 3.3. Лямбда-слоты и контекст подключения

Начиная с Qt5, в `connect()` можно передавать лямбда-функции. Важно правильно захватывать переменные: `\[=]` (по значению), `\[\&]` (по ссылке, опасно для локальных переменных), `\[this]` (захват указателя на класс). Третий аргумент `connect()` — объект-контекст — гарантирует, что при его удалении соединение будет автоматически разорвано, что защищает от вызова слотов у удаленных объектов.



\### 3.4. Динамические соединения

Функция `connect()` возвращает объект `QMetaObject::Connection`. Его можно сохранить и позже передать в `disconnect()`, чтобы разорвать конкретное соединение. Это позволяет динамически менять обработчики сигналов «на лету», не пересоздавая объекты-отправители.



\---



\## 4. Листинг кода с комментариями



\### Файл `main.cpp`

```cpp

\#include "mainwindow.h"

\#include <QApplication>



\#ifdef Q\_OS\_WIN

\#include <windows.h>

\#endif



int main(int argc, char \*argv\[])

{

\#ifdef Q\_OS\_WIN

&#x20;   SetConsoleOutputCP(CP\_UTF8);

&#x20;   SetConsoleCP(CP\_UTF8);

\#endif



&#x20;   QApplication a(argc, argv);

&#x20;   MainWindow w;

&#x20;   w.show();

&#x20;   return a.exec();

}

```



\### Файл `colorwidget.h`

```cpp

\#ifndef COLORWIDGET\_H

\#define COLORWIDGET\_H



\#include <QColor>

\#include <QVector>

\#include <QWidget>



class ColorWidget : public QWidget

{

&#x20;   Q\_OBJECT

public:

&#x20;   explicit ColorWidget(QWidget \*parent = nullptr);

&#x20;   \~ColorWidget() override;



&#x20;   void addColor(const QColor \&c);

&#x20;   void resetPalette();

&#x20;   QColor currentColor() const;



&#x20;   void stepForward();

&#x20;   void stepBackward();

&#x20;   void setRandomColor();

&#x20;   void toggleBlink();



signals:

&#x20;   void colorChanged(const QColor \&color);



protected:

&#x20;   void paintEvent(QPaintEvent \*event) override;

&#x20;   void mousePressEvent(QMouseEvent \*event) override;



private:

&#x20;   QVector<QColor> m\_palette;

&#x20;   int m\_index;

&#x20;   bool m\_blinkOn;

&#x20;   QColor m\_blinkBackup;

&#x20;   static QVector<QColor> defaultPalette();

&#x20;   void emitCurrent();

};



\#endif // COLORWIDGET\_H

```



\### Файл `colorwidget.cpp`

```cpp

\#include "colorwidget.h"

\#include <QMouseEvent>

\#include <QPainter>

\#include <QRandomGenerator>

\#include <QTextStream>

\#include <QStringConverter>



ColorWidget::ColorWidget(QWidget \*parent)

&#x20;   : QWidget(parent), m\_palette(defaultPalette()), m\_index(0), m\_blinkOn(false), m\_blinkBackup(Qt::black)

{

&#x20;   setMinimumSize(320, 220);

&#x20;   setAutoFillBackground(true);

}



ColorWidget::\~ColorWidget()

{

&#x20;   QTextStream out(stdout);

&#x20;   out.setEncoding(QStringConverter::Utf8);

&#x20;   out << "\[DESTROY] ColorWidget\\n";

}



QVector<QColor> ColorWidget::defaultPalette()

{

&#x20;   return { QColor("#e74c3c"), QColor("#e67e22"), QColor("#f1c40f"), QColor("#2ecc71"), QColor("#3498db"), QColor("#9b59b6") };

}



QColor ColorWidget::currentColor() const

{

&#x20;   if (m\_palette.isEmpty()) return Qt::gray;

&#x20;   return m\_palette.at(m\_index % m\_palette.size());

}



void ColorWidget::addColor(const QColor \&c)

{

&#x20;   m\_palette.push\_back(c);

&#x20;   QTextStream out(stdout);

&#x20;   out.setEncoding(QStringConverter::Utf8);

&#x20;   out << "\[PALETTE] Добавлен цвет " << c.name() << ", всего: " << m\_palette.size() << "\\n";

}



void ColorWidget::resetPalette()

{

&#x20;   m\_palette = defaultPalette();

&#x20;   m\_index = 0;

&#x20;   m\_blinkOn = false;

&#x20;   update();

&#x20;   emitCurrent();

}



void ColorWidget::stepForward()

{

&#x20;   if (m\_palette.isEmpty()) return;

&#x20;   m\_index = (m\_index + 1) % m\_palette.size();

&#x20;   m\_blinkOn = false;

&#x20;   update();

&#x20;   emitCurrent();

}



void ColorWidget::stepBackward()

{

&#x20;   if (m\_palette.isEmpty()) return;

&#x20;   m\_index = (m\_index - 1 + m\_palette.size()) % m\_palette.size();

&#x20;   m\_blinkOn = false;

&#x20;   update();

&#x20;   emitCurrent();

}



void ColorWidget::setRandomColor()

{

&#x20;   if (m\_palette.isEmpty()) return;

&#x20;   m\_index = QRandomGenerator::global()->bounded(m\_palette.size());

&#x20;   m\_blinkOn = false;

&#x20;   update();

&#x20;   emitCurrent();

}



void ColorWidget::toggleBlink()

{

&#x20;   if (m\_blinkOn) {

&#x20;       m\_blinkOn = false;

&#x20;   } else {

&#x20;       m\_blinkBackup = currentColor();

&#x20;       m\_blinkOn = true;

&#x20;   }

&#x20;   update();

&#x20;   emitCurrent();

}



void ColorWidget::emitCurrent()

{

&#x20;   emit colorChanged(currentColor());

}



void ColorWidget::paintEvent(QPaintEvent \*)

{

&#x20;   QPainter p(this);

&#x20;   const QColor fill = m\_blinkOn ? QColor(0, 0, 0) : currentColor();

&#x20;   p.fillRect(rect(), fill);

&#x20;   p.setPen(fill.lightness() > 128 ? Qt::black : Qt::white);

&#x20;   p.drawText(rect(), Qt::AlignCenter, fill.name());

}



void ColorWidget::mousePressEvent(QMouseEvent \*event)

{

&#x20;   QTextStream out(stdout);

&#x20;   out.setEncoding(QStringConverter::Utf8);

&#x20;   

&#x20;   if (event->button() == Qt::LeftButton) {

&#x20;       out << "\[MOUSE] Левый клик по холсту\\n";

&#x20;       stepForward();

&#x20;   } else if (event->button() == Qt::RightButton) {

&#x20;       out << "\[MOUSE] Правый клик по холсту\\n";

&#x20;       stepBackward();

&#x20;   }

&#x20;   QWidget::mousePressEvent(event);

}

```



\### Файл `palettebutton.h`

```cpp

\#ifndef PALETTEBUTTON\_H

\#define PALETTEBUTTON\_H



\#include <QColor>

\#include <QPushButton>



class PaletteButton : public QPushButton

{

&#x20;   Q\_OBJECT

public:

&#x20;   explicit PaletteButton(const QColor \&color, QWidget \*parent = nullptr);

&#x20;   \~PaletteButton() override;



&#x20;   QColor color() const { return m\_color; }



private:

&#x20;   QColor m\_color;

};



\#endif // PALETTEBUTTON\_H

```



\### Файл `palettebutton.cpp`

```cpp

\#include "palettebutton.h"

\#include <QTextStream>

\#include <QStringConverter>



PaletteButton::PaletteButton(const QColor \&color, QWidget \*parent)

&#x20;   : QPushButton(parent), m\_color(color)

{

&#x20;   setText(color.name());

&#x20;   setToolTip("Добавить этот цвет в палитру холста");

&#x20;   

&#x20;   QTextStream out(stdout);

&#x20;   out.setEncoding(QStringConverter::Utf8);

&#x20;   out << "\[CREATE] PaletteButton " << color.name() << "\\n";

}



PaletteButton::\~PaletteButton()

{

&#x20;   QTextStream out(stdout);

&#x20;   out.setEncoding(QStringConverter::Utf8);

&#x20;   out << "\[DESTROY] PaletteButton " << m\_color.name() << "\\n";

}

```



\### Файл `mainwindow.h`

```cpp

\#ifndef MAINWINDOW\_H

\#define MAINWINDOW\_H



\#include <QMetaObject>

\#include <QPointer>

\#include <QTimer>

\#include <QWidget>

\#include "colorwidget.h"

\#include "palettebutton.h"



class QLabel;

class QVBoxLayout;

class QGridLayout;



class MainWindow : public QWidget

{

&#x20;   Q\_OBJECT

public:

&#x20;   explicit MainWindow(QWidget \*parent = nullptr);

&#x20;   \~MainWindow() override;



private slots:

&#x20;   void toggleTimer();

&#x20;   void decreaseInterval();

&#x20;   void increaseInterval();

&#x20;   void cycleMode();

&#x20;   void addColorButton();

&#x20;   void clearPaletteButtons();

&#x20;   void toggleClickHandler();



private:

&#x20;   enum class Mode { Cycle, Random, Blink };

&#x20;   void setupUi();

&#x20;   void rebuildTimeoutConnection();

&#x20;   void reconnectCanvasHandler();

&#x20;   void setStatus(const QString \&text);

&#x20;   void updateIntervalLabel();

&#x20;   void updateModeLabel();

&#x20;   QString modeName() const;



&#x20;   QTimer m\_timer;

&#x20;   ColorWidget m\_canvas;

&#x20;   Mode m\_mode;

&#x20;   bool m\_clickHandlerAlt;

&#x20;   QMetaObject::Connection m\_timeoutConn;

&#x20;   QMetaObject::Connection m\_clickConn;



&#x20;   QVBoxLayout \*m\_rootLayout;

&#x20;   QGridLayout \*m\_paletteGrid;

&#x20;   QLabel \*m\_statusLabel;

&#x20;   QLabel \*m\_intervalLabel;

&#x20;   QLabel \*m\_modeLabel;

&#x20;   QList<QPointer<PaletteButton>> m\_paletteButtons;

&#x20;   QList<QMetaObject::Connection> m\_dynamicConnections;

};



\#endif // MAINWINDOW\_H

```



\### Файл `mainwindow.cpp`

```cpp

\#include "mainwindow.h"

\#include <QGridLayout>

\#include <QHBoxLayout>

\#include <QLabel>

\#include <QPushButton>

\#include <QRandomGenerator>

\#include <QVBoxLayout>

\#include <QTextStream>

\#include <QStringConverter>



MainWindow::MainWindow(QWidget \*parent)

&#x20;   : QWidget(parent), m\_mode(Mode::Cycle), m\_clickHandlerAlt(false)

{

&#x20;   setupUi();

&#x20;   m\_timer.setInterval(600);

&#x20;   rebuildTimeoutConnection();

&#x20;   reconnectCanvasHandler();

&#x20;   setStatus("Готово. Нажмите «Старт/Стоп», чтобы запустить таймер.");

}



MainWindow::\~MainWindow()

{

&#x20;   QTextStream out(stdout);

&#x20;   out.setEncoding(QStringConverter::Utf8);

&#x20;   out << "\[DESTROY] MainWindow\\n";

&#x20;   m\_timer.stop();

}



void MainWindow::setupUi()

{

&#x20;   setWindowTitle("QTimer + лямбда-слоты + динамические соединения");

&#x20;   resize(560, 640);



&#x20;   m\_rootLayout = new QVBoxLayout(this);

&#x20;   m\_canvas.setParent(this);

&#x20;   m\_rootLayout->addWidget(\&m\_canvas);



&#x20;   m\_statusLabel = new QLabel(this);

&#x20;   m\_statusLabel->setWordWrap(true);

&#x20;   m\_rootLayout->addWidget(m\_statusLabel);



&#x20;   m\_intervalLabel = new QLabel(this);

&#x20;   m\_modeLabel = new QLabel(this);

&#x20;   m\_rootLayout->addWidget(m\_intervalLabel);

&#x20;   m\_rootLayout->addWidget(m\_modeLabel);



&#x20;   auto \*controls = new QHBoxLayout();

&#x20;   auto \*btnStart = new QPushButton("Старт/Стоп", this);

&#x20;   auto \*btnSlow = new QPushButton("Медленнее", this);

&#x20;   auto \*btnFast = new QPushButton("Быстрее", this);

&#x20;   auto \*btnMode = new QPushButton("Режим", this);

&#x20;   controls->addWidget(btnStart);

&#x20;   controls->addWidget(btnSlow);

&#x20;   controls->addWidget(btnFast);

&#x20;   controls->addWidget(btnMode);

&#x20;   m\_rootLayout->addLayout(controls);



&#x20;   connect(btnStart, \&QPushButton::clicked, this, \&MainWindow::toggleTimer);

&#x20;   connect(btnSlow, \&QPushButton::clicked, this, \&MainWindow::decreaseInterval);

&#x20;   connect(btnFast, \&QPushButton::clicked, this, \&MainWindow::increaseInterval);

&#x20;   connect(btnMode, \&QPushButton::clicked, this, \&MainWindow::cycleMode);



&#x20;   auto \*palHeader = new QLabel("Палитра (кнопки создаются динамически):", this);

&#x20;   m\_rootLayout->addWidget(palHeader);



&#x20;   m\_paletteGrid = new QGridLayout();

&#x20;   m\_rootLayout->addLayout(m\_paletteGrid);



&#x20;   auto \*palControls = new QHBoxLayout();

&#x20;   auto \*btnAdd = new QPushButton("+ Цвет", this);

&#x20;   auto \*btnClear = new QPushButton("Очистить палитру", this);

&#x20;   auto \*btnClick = new QPushButton("Сменить обработчик клика", this);

&#x20;   palControls->addWidget(btnAdd);

&#x20;   palControls->addWidget(btnClear);

&#x20;   palControls->addWidget(btnClick);

&#x20;   m\_rootLayout->addLayout(palControls);



&#x20;   connect(btnAdd, \&QPushButton::clicked, this, \&MainWindow::addColorButton);

&#x20;   connect(btnClear, \&QPushButton::clicked, this, \&MainWindow::clearPaletteButtons);

&#x20;   connect(btnClick, \&QPushButton::clicked, this, \&MainWindow::toggleClickHandler);



&#x20;   updateIntervalLabel();

&#x20;   updateModeLabel();

}



void MainWindow::toggleTimer()

{

&#x20;   QTextStream out(stdout);

&#x20;   out.setEncoding(QStringConverter::Utf8);

&#x20;   

&#x20;   if (m\_timer.isActive()) {

&#x20;       m\_timer.stop();

&#x20;       setStatus("Таймер остановлен.");

&#x20;       out << "\[TIMER] isActive: false\\n";

&#x20;   } else {

&#x20;       m\_timer.start();

&#x20;       setStatus(QString("Таймер запущен (интервал %1 мс).").arg(m\_timer.interval()));

&#x20;       out << "\[TIMER] isActive: true\\n";

&#x20;   }

}



void MainWindow::decreaseInterval()

{

&#x20;   m\_timer.setInterval(qMin(m\_timer.interval() + 200, 5000));

&#x20;   updateIntervalLabel();

&#x20;   

&#x20;   QTextStream out(stdout);

&#x20;   out.setEncoding(QStringConverter::Utf8);

&#x20;   out << "\[TIMER] interval -> " << m\_timer.interval() << "\\n";

}



void MainWindow::increaseInterval()

{

&#x20;   m\_timer.setInterval(qMax(m\_timer.interval() - 200, 50));

&#x20;   updateIntervalLabel();

&#x20;   

&#x20;   QTextStream out(stdout);

&#x20;   out.setEncoding(QStringConverter::Utf8);

&#x20;   out << "\[TIMER] interval -> " << m\_timer.interval() << "\\n";

}



void MainWindow::cycleMode()

{

&#x20;   switch (m\_mode) {

&#x20;   case Mode::Cycle: m\_mode = Mode::Random; break;

&#x20;   case Mode::Random: m\_mode = Mode::Blink; break;

&#x20;   case Mode::Blink: m\_mode = Mode::Cycle; break;

&#x20;   }

&#x20;   updateModeLabel();

&#x20;   rebuildTimeoutConnection();

&#x20;   setStatus("Режим переключён: " + modeName());

&#x20;   

&#x20;   QTextStream out(stdout);

&#x20;   out.setEncoding(QStringConverter::Utf8);

&#x20;   out << "\[TIMER] режим -> " << modeName() << "\\n";

}



void MainWindow::rebuildTimeoutConnection()

{

&#x20;   if (m\_timeoutConn) {

&#x20;       disconnect(m\_timeoutConn);

&#x20;   }

&#x20;   m\_timeoutConn = connect(\&m\_timer, \&QTimer::timeout, this, \[this]() {

&#x20;       switch (m\_mode) {

&#x20;       case Mode::Cycle: m\_canvas.stepForward(); break;

&#x20;       case Mode::Random: m\_canvas.setRandomColor(); break;

&#x20;       case Mode::Blink: m\_canvas.toggleBlink(); break;

&#x20;       }

&#x20;       

&#x20;       QTextStream out(stdout);

&#x20;       out.setEncoding(QStringConverter::Utf8);

&#x20;       out << "\[TIMER] timeout, режим = " << modeName() << ", цвет = " << m\_canvas.currentColor().name() << "\\n";

&#x20;   });

}



void MainWindow::reconnectCanvasHandler()

{

&#x20;   if (m\_clickConn) {

&#x20;       disconnect(m\_clickConn);

&#x20;   }

&#x20;   if (!m\_clickHandlerAlt) {

&#x20;       m\_clickConn = connect(\&m\_canvas, \&ColorWidget::colorChanged, this, \[this](const QColor \&c) {

&#x20;           setStatus("Цвет холста: " + c.name());

&#x20;       });

&#x20;   } else {

&#x20;       m\_clickConn = connect(\&m\_canvas, \&ColorWidget::colorChanged, this, \[this](const QColor \&c) {

&#x20;           setWindowTitle("ColorSwitcher - " + c.name());

&#x20;       });

&#x20;   }

}



void MainWindow::toggleClickHandler()

{

&#x20;   m\_clickHandlerAlt = !m\_clickHandlerAlt;

&#x20;   reconnectCanvasHandler();

&#x20;   setStatus(m\_clickHandlerAlt ? "Обработчик клика: вариант Б." : "Обработчик клика: вариант А.");

&#x20;   

&#x20;   QTextStream out(stdout);

&#x20;   out.setEncoding(QStringConverter::Utf8);

&#x20;   out << "\[CANVAS] переподключение colorChanged, альтернатива = " << (m\_clickHandlerAlt ? "true" : "false") << "\\n";

}



void MainWindow::addColorButton()

{

&#x20;   QColor color = QColor::fromHsv(

&#x20;       QRandomGenerator::global()->bounded(360),

&#x20;       200 + QRandomGenerator::global()->bounded(55),

&#x20;       200 + QRandomGenerator::global()->bounded(55));



&#x20;   auto \*btn = new PaletteButton(color, this);

&#x20;   const int row = m\_paletteButtons.size() / 4;

&#x20;   const int col = m\_paletteButtons.size() % 4;

&#x20;   m\_paletteGrid->addWidget(btn, row, col);



&#x20;   QMetaObject::Connection conn = connect(btn, \&QPushButton::clicked, this, \[this, btn]() {

&#x20;       m\_canvas.addColor(btn->color());

&#x20;       setStatus("Добавлен цвет " + btn->color().name());

&#x20;   });



&#x20;   m\_paletteButtons.append(QPointer<PaletteButton>(btn));

&#x20;   m\_dynamicConnections.append(conn);

}



void MainWindow::clearPaletteButtons()

{

&#x20;   for (QMetaObject::Connection \&c : m\_dynamicConnections) {

&#x20;       if (c) disconnect(c);

&#x20;   }

&#x20;   m\_dynamicConnections.clear();



&#x20;   for (QPointer<PaletteButton> \&btn : m\_paletteButtons) {

&#x20;       if (!btn.isNull()) btn->deleteLater();

&#x20;   }

&#x20;   m\_paletteButtons.clear();



&#x20;   m\_canvas.resetPalette();

&#x20;   setStatus("Палитра очищена.");

}



void MainWindow::setStatus(const QString \&text)

{

&#x20;   m\_statusLabel->setText(text);

}



void MainWindow::updateIntervalLabel()

{

&#x20;   m\_intervalLabel->setText(QString("Интервал: %1 мс").arg(m\_timer.interval()));

}



void MainWindow::updateModeLabel()

{

&#x20;   m\_modeLabel->setText("Режим: " + modeName());

}



QString MainWindow::modeName() const

{

&#x20;   switch (m\_mode) {

&#x20;   case Mode::Cycle: return "Цикл";

&#x20;   case Mode::Random: return "Случайный";

&#x20;   case Mode::Blink: return "Мигание";

&#x20;   }

&#x20;   return "";

}

```



\---



\## 5. Пример работы программы



\### Консольный вывод при запуске и работе таймера

```

\[TIMER] isActive: true

\[TIMER] timeout, режим = Цикл, цвет = #e67e22

\[TIMER] timeout, режим = Цикл, цвет = #f1c40f

\[TIMER] timeout, режим = Цикл, цвет = #2ecc71

```



\### Консольный вывод при клике по холсту

```

\[MOUSE] Левый клик по холсту

\[CANVAS] переподключение colorChanged, альтернатива = false

```



\### Консольный вывод при добавлении цвета

```

\[CREATE] PaletteButton #ff5733

\[PALETTE] Добавлен цвет #ff5733, всего: 7

```



\---



\## 6. Разбор ошибок и их решений



1\. \*\*Кракозябры в консоли Windows.\*\* При выводе русских букв через `qDebug()` в консоли Windows отображались нечитаемые символы. \*Решение:\* В файле `main.cpp` добавлены вызовы `SetConsoleOutputCP(CP\_UTF8)` и `SetConsoleCP(CP\_UTF8)` для переключения кодировки консоли. Также в файл `mainwindow.cpp` и другие файлы добавлен `QTextStream` с явным указанием кодировки UTF-8.

2\. \*\*Ошибка линковки `multiple definition`.\*\* Возникла при дублировании тела метода в `.h` и `.cpp` файлах. \*Решение:\* Тела методов оставлены только в `.cpp` файлах, в `.h` оставлены только объявления.

3\. \*\*Отсутствие реакции на клики мыши.\*\* Клики по холсту не обрабатывались. \*Решение:\* В классе `ColorWidget` переопределен метод `mousePressEvent`, который обрабатывает нажатия левой и правой кнопки мыши.



\---



\## 7. Контрольные вопросы и ответы



\*\*1. Что такое QTimer и какой сигнал он излучает по истечении интервала?\*\*

`QTimer` — это класс Qt для работы с таймерами. По истечении заданного интервала (в миллисекундах) он излучает сигнал `timeout()`. В коде таймер создан как поле `QTimer m\_timer;`, а его сигнал подключен в методе `rebuildTimeoutConnection()`.



\*\*2. Чем `start()` отличается от `start(ms)` и от `QTimer::singleShot()`?\*\*

`start()` запускает повторяющийся таймер с уже заданным интервалом. `start(ms)` запускает таймер один раз с новым интервалом. `QTimer::singleShot()` — это статический метод, который вызывает функцию один раз через заданное время без создания объекта таймера. В коде используется `m\_timer.start()` в методе `toggleTimer()`.



\*\*3. Почему QTimer не работает без `app.exec()`? Что такое цикл событий?\*\*

`QTimer` регистрируется в цикле событий Qt (event loop). Цикл событий — это бесконечный цикл, который перехватывает действия пользователя и системные сообщения и распределяет их по объектам. Без вызова `app.exec()` в `main.cpp` программа завершится сразу, и таймер не успеет сработать.



\*\*4. Чем GUI-приложение с таймером принципиально отличается от консольного калькулятора ЛР1, где `exec()` не вызывался?\*\*

В консольном калькуляторе использовался блокирующий ввод (`std::cin`), который останавливал программу и ждал ввода. В GUI-приложении программа должна постоянно реагировать на клики мыши, нажатия клавиш и таймеры. Для этого нужен цикл событий `app.exec()`.



\*\*5. Что произойдёт, если лямбда-слот захватывает `this`, но контекст подключения не передан?\*\*

Если не передать контекст (третий аргумент в `connect`), то при удалении окна (объекта `this`) таймер продолжит работать и вызовет лямбду, которая попытается обратиться к удаленным полям. Это приведет к падению программы. В коде контекст `this` передан везде, например: `connect(\&m\_timer, \&QTimer::timeout, this, \[this]() { ... });`.



\*\*6. Зачем третий аргумент `connect()` (context object) и как он связан с деревом объектов ЛР2?\*\*

Третий аргумент — это объект-контекст. Когда этот объект уничтожается, Qt автоматически разрывает все его соединения. Это связано с деревом объектов из ЛР2: если контекст является ребенком какого-то виджета, при удалении родителя соединение разорвется автоматически, и программа не упадет.



\*\*7. Чем захват `\[=]` отличается от `\[\&]`? Когда `\[\&]` опасен?\*\*

`\[=]` захватывает переменные по значению (копирует их). `\[\&]` захватывает по ссылке. `\[\&]` опасен, если лямбда будет вызвана позже, чем переменная выйдет из области видимости (например, локальная переменная в функции). В таком случае лямбда будет обращаться к уже уничтоженной памяти.



\*\*8. Что такое `QMetaObject::Connection` и как разорвать соединение по нему?\*\*

Это объект, который хранит информацию о конкретном соединении сигнала и слота. Чтобы разорвать соединение, нужно сохранить его при вызове `connect()`, а затем передать в `disconnect()`. В коде это сделано в методе `rebuildTimeoutConnection()`: `m\_timeoutConn = connect(...);` и затем `disconnect(m\_timeoutConn);`.



\*\*9. Что такое `Qt::UniqueConnection` и какую проблему решает?\*\*

Это флаг, который запрещает создавать дублирующиеся соединения. Если вы вызовете `connect()` с этим флагом дважды для одних и тех же объектов, слот сработает только один раз. Это решает проблему, когда слот вызывается N раз из-за повторных подключений.



\*\*10. Что будет, если вызвать одинаковый `connect()` дважды на одних объектах?\*\*

Слот будет вызываться дважды при каждом сигнале. Симптом: цвет «прыгает» на 2-3 позиции за тик таймера.



\*\*11. Рвёт ли Qt соединения автоматически при удалении отправителя / получателя / контекста?\*\*

Да. Если удалить отправитель (например, кнопку), получатель (окно) или объект-контекст (третий аргумент), Qt автоматически разорвет все связанные соединения.



\*\*12. Почему для удаления динамической кнопки лучше `deleteLater()`, чем `delete`?\*\*

`delete` удаляет объект мгновенно. Если кнопка в этот момент обрабатывает клик (сигнал), программа упадет. `deleteLater()` ставит объект в очередь на удаление, которое произойдет безопасно в цикле событий, когда кнопка уже не участвует в обработке сигналов. В коде в методе `clearPaletteButtons()` используется `btn->deleteLater();`.



\*\*13. Чем `mousePressEvent()` отличается от сигнала `clicked()`? Можно ли подключиться к событию через `connect()`?\*\*

`mousePressEvent()` — это виртуальный метод (событие), который переопределяется в классе-наследнике (например, в `ColorWidget`). `clicked()` — это высокоуровневый сигнал (например, у `QPushButton`). К событиям нельзя подключиться через `connect()`, их можно только переопределять. В коде в `ColorWidget` переопределен `mousePressEvent`, который излучает собственный сигнал `colorChanged`.



\*\*14. Что такое `eventFilter()` и когда он удобнее переопределения виртуального метода?\*\*

`eventFilter()` — это механизм, позволяющий одному объекту перехватывать и обрабатывать события другого объекта. Он удобнее, когда нужно перехватить события у объекта, который вы не можете наследовать (например, у стандартной кнопки `QPushButton`), или когда нужно обработать события сразу для нескольких объектов в одном месте.



\*\*15. Как реализовать мигание одним таймером без второго таймера?\*\*

Нужно хранить состояние (флаг) и переключать его при каждом срабатывании таймера. В коде это сделано в методе `toggleBlink()`: если `m\_blinkOn` истинно, оно становится ложным, и наоборот. При отрисовке в `paintEvent` цвет выбирается в зависимости от этого флага.



\*\*16. Что произойдёт, если менять `interval()` у активного таймера?\*\*

Таймер продолжит работать, но следующий тик произойдет уже с новым интервалом. В коде это используется в методах `increaseInterval()` и `decreaseInterval()`.



\*\*17. Почему состояние режима хранится в поле `m\_mode`, а не захватывается в лямбду?\*\*

Если захватить `m\_mode` по значению в лямбду, то при смене режима лямбда продолжит использовать старое значение. Если захватить по ссылке — это опасно. Хранение в поле класса (`m\_mode`) и захват `\[this]` позволяет лямбде всегда читать актуальное состояние. В коде это реализовано в методе `rebuildTimeoutConnection()`.



\*\*18. Как проверить срабатывание `timeout` в юнит-тесте без реального ожидания?\*\*

Использовать `QTest::qWait()` или функцию `QSignalSpy` из модуля QtTest. `QSignalSpy` позволяет «поймать» сигнал и проверить, был ли он издан. Реального ожидания можно избежать, установив интервал таймера в 0.



\*\*19. Можно ли использовать `QTimer` в потоке без цикла событий? Что для этого нужно?\*\*

Нет, `QTimer` требует наличия цикла событий. Если вы работаете в отдельном потоке (через `QThread`), вам нужно вызвать `QThread::exec()` в методе `run()`, чтобы запустить цикл событий в этом потоке.



\*\*20. Как динамические соединения связаны с автоудалением детей из ЛР2 при очистке палитры?\*\*

В методе `clearPaletteButtons()` вы явно разрываете соединения через `disconnect(c)`, а затем удаляете кнопки через `btn->deleteLater()`. Если бы вы этого не сделали, кнопки остались бы в памяти, а их сигналы могли бы вызывать слоты у уже удаленных объектов.



\*\*21. Чем сообщение компилятора `not declared in this scope` отличается от сообщения линкера `undefined reference` и как по-разному чинить каждый?\*\*

`not declared in this scope` — ошибка компилятора: переменная или метод не объявлены в текущей области видимости. Лечится добавлением объявления (например, в `.h`). `undefined reference` — ошибка линкера: объявление есть, но нет определения (тела) функции. Лечится добавлением тела функции в `.cpp` файл и проверкой, что этот `.cpp` добавлен в `SOURCES`.



\*\*22. Почему `multiple definition` возникает, когда тело метода есть и в `.h`, и в `.cpp`, и как это исправить?\*\*

Линкер находит два одинаковых определения функции (в двух объектных файлах) и не знает, какое использовать. Исправление: оставить тело только в `.cpp` файле (или пометить метод как `inline` в `.h`, если тело короткое).



\*\*23. Если компиляция прошла без ошибок, но линковка падает на `undefined reference` по всем слотам класса – где искать причину?\*\*

Причина в том, что файл `.cpp` с реализацией класса не участвует в сборке (не добавлен в `SOURCES` в `.pro` файле) или содержит только объявления, а не определения. Нужно проверить `.pro` файл и убедиться, что там есть строка с этим `.cpp`.



\---



\## 8. Вывод

В ходе выполнения лабораторной работы были изучены класс `QTimer`, механизм лямбда-слотов и динамические соединения в Qt. Были освоены методы захвата переменных в лямбда-функциях, работа с `QMetaObject::Connection` для разрыва соединений, а также переопределение событий мыши. Сложности возникли с настройкой кодировки консоли, но они были решены с помощью `QTextStream` и системных вызовов Windows. В результате было разработано приложение `ColorSwitcher`, полностью соответствующее заданию.



\---



\## 9. Ссылка на репозиторий

https://github.com/TimeCryyz/color-switcher.git

