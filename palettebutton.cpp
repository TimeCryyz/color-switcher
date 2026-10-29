#include "palettebutton.h"
#include <QTextStream>
#include <QStringConverter>

PaletteButton::PaletteButton(const QColor &color, QWidget *parent)
    : QPushButton(parent), m_color(color)
{
    setText(color.name());
    setToolTip("Добавить этот цвет в палитру холста");

    QTextStream out(stdout);
    out.setEncoding(QStringConverter::Utf8);
    out << "[CREATE] PaletteButton " << color.name() << "\n";
}

PaletteButton::~PaletteButton()
{
    QTextStream out(stdout);
    out.setEncoding(QStringConverter::Utf8);
    out << "[DESTROY] PaletteButton " << m_color.name() << "\n";
}