//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "imageget.h"

ImageGet::ImageGet(QUrl url, QObject* parent) : QThread(parent), url(url) {
}

ImageGet::~ImageGet() {
  webSocket->close();
  webSocket->deleteLater();
  this->exit();
}

void ImageGet::run() {
  //QWebSocket webSocket;
  webSocket = new QWebSocket();
  qDebug() << "ImageGet run " << url;
  connect(webSocket, &QWebSocket::connected, [&]() {
    qDebug() << "webSocket  connected";
    connect(webSocket, &QWebSocket::textMessageReceived, [&](
    const QString & message) {
      // qDebug() << "ImageGet textMessageReceived: " << message;
      emit imageReceived(message);
    });
    emit statusSend(LinkSuccess, "websocket链接成功");
  });
  connect(webSocket, QOverload<QAbstractSocket::SocketError>::of(
  &QWebSocket::error), [&](QAbstractSocket::SocketError error) {
    qDebug() << "WebSocket error:" << error << "-" << webSocket->errorString();
    emit statusSend(LinkError, "websocket链接失败" + webSocket->errorString());
    webSocket->close();
    this->exit();
  });
  connect(webSocket, &QWebSocket::disconnected, [&]() {
    qDebug() << "WebSocket disconnected";
    emit statusSend(LinkError, "websocket链接失败" + webSocket->errorString());
    webSocket->close();
    this->exit();
  });
  webSocket->open(url);
  this->exec();
  webSocket->close();
  qDebug() << "ImageGet stop";
}
