#include "hexlineedit.h"

HexLineEdit::HexLineEdit(QWidget *parent):QLineEdit(parent){
    QRegularExpression reg_exp ("[0-9A-F]*");
    setText("0x");
    setValidator(new QRegularExpressionValidator(reg_exp, this));
}

HexLineEdit::HexLineEdit(QString text, QWidget *parent):QLineEdit(parent){
    setText("0x" + text);
}

void HexLineEdit::keyPressEvent(QKeyEvent *event){
    if (cursorPosition() <= 2 && (event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete))
        return;

    QString input_text = event->text();
    if (input_text.length() == 1 && (input_text[0].isDigit() || input_text[0].isLetter())) {
        if (input_text[0].isDigit())
            setText(this->text() + input_text);
        else {
            input_text = input_text.toUpper();
            if (input_text[0].toUpper() >= 'A' && input_text[0].toUpper() <= 'F') {
                setText(this->text() + input_text);
                event->accept();
            }
        }
        return;
    }
    QLineEdit::keyPressEvent(event);
}

void HexLineEdit::mouseDoubleClickEvent(QMouseEvent *event){
    setSelection(2, text().length() - 2);
}

void HexLineEdit::mousePressEvent(QMouseEvent *event){
    cur_pos = cursorPosition();
    QLineEdit::mousePressEvent(event);
}

void HexLineEdit::mouseMoveEvent(QMouseEvent *event){
    QLineEdit::mouseMoveEvent(event);
    if (selectionStart() < 2)
        if (selectionStart() < selectionEnd())
            setSelection(2, cur_pos);
}
