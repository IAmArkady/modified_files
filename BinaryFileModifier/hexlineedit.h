#ifndef HEXLINEEDIT_H
#define HEXLINEEDIT_H

#include <QLineEdit>
#include <QObject>
#include <QKeyEvent>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

class HexLineEdit : public QLineEdit
{
    Q_OBJECT
    int cur_pos;
public:
    explicit HexLineEdit(QWidget *parent = Q_NULLPTR);
    explicit HexLineEdit(QString text, QWidget *parent = Q_NULLPTR);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
};

#endif
