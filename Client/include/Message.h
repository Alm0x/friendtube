#ifndef MESSAGE_H
#define MESSAGE_H

#include <QFile>
#include <QGraphicsTextItem>
#include <QJsonObject>
#include <QJsonParseError>
#include <QString>
#include <QTextStream>
#include <QTime>

struct Message {
    QString send_message = "";
    QString type = "text";
    QTime sending_time;
    QGraphicsPixmapItem *emoji;
    bool metka_message = false;// есть ли сообщение
    bool metka_message_painter =
            false;// чтобы заново таймер не запускать
    QJsonObject from_message_to_json();
};

Message from_json_to_message(QJsonObject);

#endif// MESSAGE_H
