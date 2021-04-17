#include "NetworkManager.h"

NetworkManager::NetworkManager(Client *client_, const QString &ip_, int port_, QObject *parent) : QObject() {
    client = client_;
    socket_mutex = new QMutex();
    ip = ip_;
    port = port_;
}

NetworkManager::~NetworkManager(){
    socket_mutex->~QMutex();
}


void NetworkManager::run() {
    socket = new QWebSocket();

   connect(socket, &QWebSocket::connected, this, &NetworkManager::onConnected);
   connect(socket, &QWebSocket::disconnected, this, &NetworkManager::socketDisconnect);
   connect(socket, &QWebSocket::binaryMessageReceived, this, &NetworkManager::socketReady);
   connect(this, SIGNAL(createRoom(Player*, QVector<PlayerView>)), client, SLOT(createRoom(Player*, QVector<PlayerView>)));

   //socket->open(QUrl(ip + ":" + QString::number(port)));
   socket->open(QUrl("ws://localhost:" + QString::number(port)));

   //qDebug() << "Try connect" << ip.toUtf8() << " ";
}


void NetworkManager::onConnected() {
    qDebug() << "Connection is successful";


}


void NetworkManager::socketDisconnect() {
   socket->close();
}


void NetworkManager::socketReady(const QByteArray &data) {
    QMutexLocker locker(socket_mutex);
    // qDebug() << "Client Thread" << QThread::currentThreadId();



    QJsonParseError json_data_error;
    QJsonObject json_data = QJsonDocument::fromJson(data, &json_data_error).object();
    qDebug() << "_______________\n" << json_data << "\n______________\n";

    if(json_data_error.errorString().toInt() == QJsonParseError::NoError) {
        QString event_type = json_data.value("type").toString();
        qDebug() << event_type;

        if(event_type == "first_connection") {
           client_id = json_data.value("client_id").toString();
           client->menu->player->client_id = client_id;
           qDebug() << "OK";
           QJsonObject req;
           req.insert("type", "connect");
           req.insert("person_data", client->menu->player->to_json().object());
           QJsonDocument doc(req);

           sendData(doc.toJson());

           qDebug() << "MUTEX UNLOCK" << QThread::currentThreadId();
           return;
        } else if(event_type == "connected") {
            QJsonObject scene = json_data.value("scene_data").toObject();
            QJsonArray json_players = scene.value("clients").toArray();

            QVector<PlayerView> players_;
            for(auto json_player: json_players) {
                players_.push_back(PlayerView(std::move(Player(json_player.toObject()))));
            }
            qRegisterMetaType<QVector<PlayerView> >("QVector<PlayerView>");
            emit createRoom(client->menu->player, std::move(players_));

             return;
        } else if(event_type == "updated_successfully"){
             client->room->is_updated_data = false;
             qDebug() << "MUTEX UNLOCK" << QThread::currentThreadId();
            return;
            // server update our state successfully
        } else if(event_type == "scene_data") {

              client->room->is_got_scene = false;
              QJsonObject scene = json_data.value("data").toObject();
              QJsonArray json_players = scene.value("clients").toArray();
              QVector<PlayerView> players_;
              for(auto json_player: json_players) {
                  players_.push_back(PlayerView(std::move(Player(json_player.toObject()))));
              }
              client->room->players = std::move(players_);
             qDebug() << "MUTEX UNLOCK" << QThread::currentThreadId();
            return;

        }
    }
}



void NetworkManager::update_state_on_the_server(const QJsonDocument &state){
    QMutexLocker locker(socket_mutex);

    QJsonObject req;
    req.insert("type", "update_my_state");
    req.insert("client_id", client_id);
    req.insert("person_data", state.object());
    QJsonDocument doc(req);

    sendData(doc.toJson());
}


void NetworkManager::sendData(const QByteArray &data) {
    socket->sendBinaryMessage(data);
    socket->flush();
}


void NetworkManager::request_get_scene_on_the_server(){
    QMutexLocker locker(socket_mutex);

    QJsonObject req;
    req.insert("type", "get_scene");
    req.insert("client_id", client_id);
    QJsonDocument doc(req);

    sendData(doc.toJson());
}
