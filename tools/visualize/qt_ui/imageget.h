//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef IMAGEGET_H
#define IMAGEGET_H

#include <QApplication>
#include <QBuffer>
#include <QByteArray>
#include <QCoreApplication>
#include <QEventLoop>
#include <QImageReader>
#include <QLabel>
#include <QMutex>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPixmap>
#include <QQueue>
#include <QString>
#include <QThread>
#include <QWebSocket>

class ImageGet : public QThread {
  Q_OBJECT
 public:
  ImageGet(QUrl url, QObject* parent = nullptr);
  ~ImageGet();
  QUrl url;
  QEventLoop loop;
  enum LinkStatus {
    LinkDisconnected,
    LinkTimeout,
    LinkError = -1,
    LinkSuccess = 0
  };

 signals:
  void imageReceived(const QString& image);
  void statusSend(const qint64 status, const QString& message);

 protected:
  void run() override;
  QWebSocket* webSocket;
};

#endif  // IMAGEGET_H
