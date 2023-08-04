//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow) {
  ui->setupUi(this);
  pHighLighter = new jsonHighlighter(ui->plainTextEdit->document());
  isRunning = 0;
  timerForImageShow.setInterval(12);
  connect(&timerForImageShow, &QTimer::timeout, this, [&]() {
    if (!isRunning)
      return;
    if (mutexForImageShow.tryLock()) {
      //qDebug() << "Timer triggered!";
      /* 这个函数用于显示image */
      imageSize = size() / 2;
      ui->labelShow->setPixmap(imageShow.scaled(imageSize,
          Qt::KeepAspectRatio,
          Qt::SmoothTransformation));
      mutexForImageShow.unlock() ;
    }
    else
      return;
  });
  timerForImageShow.start();
  inputLock(false);
  connect(ui->pushButtonSave, SIGNAL(clicked()), this,
    SLOT(on_pushButtonSave_clicked()));
}

MainWindow::~MainWindow() {
  closeWebSocket();
  sendStartStop(false);
  delete pHighLighter;
  delete ui;
}

void MainWindow::on_pushButtonStart_2_clicked() {
  getPipelines();
}

void MainWindow::getPipelines() {
  QNetworkRequest request(QUrl(QString("http://%1:%2/pipelines").arg(
        ui->lineEditIP->text(), QString::number(ui->spinBoxPort->value()))));
  pNetworkReply = manager.get(request);
  connect(pNetworkReply, &QNetworkReply::finished, [&]() {
    if (pNetworkReply->error() == QNetworkReply::NoError) {
      QByteArray jsonData = pNetworkReply->readAll();
      qDebug() << "GET response:" << QString(jsonData);
      QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
      QJsonObject jsonObject = jsonDoc.object();
      QJsonArray dataArray = jsonObject.value("data").toArray();
      this->ui->comboBoxSeletPipline->clear();
      //pipeLineFilesMaps.clear();
      pipeLinesMap.clear();
      pipeLinesIdMap.clear();
      for (const QJsonValue& value : dataArray) {
        QMap <QString, QString> pipeLineFilesMap;
        QJsonObject pipelineObject = value.toObject();
        QString pipelineName = pipelineObject.value("pipeline_name").toString();
        QString pipelineId = QString::number(
            pipelineObject.value("pipeline_id").toInt());
        this->ui->comboBoxSeletPipline->addItem(pipelineName);
        // Associate pipeline_name with json_list
        for (const QJsonValue& jsonValue :
          pipelineObject.value("json_list").toArray()) {
          QJsonObject jsonItem = jsonValue.toObject();
          pipeLineFilesMap.insert(jsonItem.value("json_name").toString(),
            jsonItem.value("json_id").toString());
        }
        pipeLinesMap.insert(pipelineName, pipeLineFilesMap);
        pipeLinesIdMap.insert(pipelineName, pipelineId);
        //pipeLineFilesMaps.append(pipeLineFilesMap);
      }
      qDebug() << "pipeLinesMap " << pipeLinesMap;
      for (const QString& key : pipeLinesMap.keys()) {
        const QMap<QString, QString>& innerMap = pipeLinesMap.value(key);
        qDebug() << "pipeLine: " << key << " innerMap " << innerMap;
      }
      qDebug() << "pipeLinesIdMap " << pipeLinesIdMap;
      if (this->ui->comboBoxSeletPipline->count() > 0) {
        this->ui->comboBoxSeletPipline->setCurrentIndex(0);
        this->ui->comboBoxSeletPipline->currentTextChanged(
          this->ui->comboBoxSeletPipline->itemText(0));
      }
    }
    else
      qDebug() << "Error:" << pNetworkReply->errorString();
    pNetworkReply->deleteLater();
  });
}

QString MainWindow::getConfFile(QString fileName) {
  QString urlStr;
  QString ret = "";
  QEventLoop loop;
  urlStr = QString("http://%1:%2/jsons/%3").arg(
      ui->lineEditIP->text(), QString::number(ui->spinBoxPort->value()),
      pipeLinesMap.value(ui->comboBoxSeletPipline->currentText()).value(
        (fileName == QString("NONE")) ? ui->comboBoxSeletFile->currentText() :
        fileName));
  QUrl url = QUrl(urlStr);
  QNetworkRequest request(url);
  pNetworkReply = manager.get(request);
  connect(pNetworkReply, &QNetworkReply::finished, [&]() {
    if (pNetworkReply->error() == QNetworkReply::NoError) {
      QByteArray jsonData = pNetworkReply->readAll();
      qDebug() << "GET response:" << QString(jsonData);
      QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
      if (!jsonDoc.isNull() && jsonDoc.isObject()) {
        QJsonObject jsonObj = jsonDoc.object();
        qDebug() << "jsonObj " << jsonObj;
        if (jsonObj.contains("data")) {
          QJsonObject jsonObj2 = jsonObj["data"].toObject();
          if (jsonObj2.contains("json_content")) {
            QJsonArray jsonObj3 = jsonObj2["json_content"].toArray();
            QJsonObject jsonObj4 = jsonObj2["json_content"].toObject();
            if (jsonObj3.isEmpty())
              ret = QJsonDocument(jsonObj4).toJson(QJsonDocument::Indented);
            else
              ret = QJsonDocument(jsonObj3).toJson(QJsonDocument::Indented);
            qDebug() << "Extracted data:" << ret;
            if (fileName == QString("NONE"))
              this->ui->plainTextEdit->setPlainText(ret);
          }
        }
      }
    }
    else {
      QMessageBox::warning(this, "警告",
        QString("getConfFile " + pNetworkReply->errorString()));
      qDebug() << "Error:" << pNetworkReply->errorString();
    }
    pNetworkReply->deleteLater();
    loop.quit();
  });
  loop.exec();
  return ret;
}

QString MainWindow::putConfFile(QString fileName) {
  QString urlStr;
  QString ret = "";
  QEventLoop loop;
  urlStr = QString("http://%1:%2/jsons/%3").arg(
      ui->lineEditIP->text(), QString::number(ui->spinBoxPort->value()),
      pipeLinesMap.value(ui->comboBoxSeletPipline->currentText()).value(
        (fileName == QString("NONE")) ? ui->comboBoxSeletFile->currentText() :
        fileName));
  QUrl url = QUrl(urlStr);
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  QByteArray jsonData = ui->plainTextEdit->toPlainText().toUtf8();
  pNetworkReply = manager.put(request, jsonData);
  connect(pNetworkReply, &QNetworkReply::finished, [&]() {
    if (pNetworkReply->error() == QNetworkReply::NoError)
      qDebug() << "Upload successful.";
    else
      qWarning() << "Upload failed:" << pNetworkReply->errorString();
    pNetworkReply->deleteLater();
    loop.quit();
  });
  loop.exec();
  return ret;
}

void MainWindow::sendStartStop(bool flag) {
  QJsonObject jsonObject;
  QEventLoop loop;
  jsonObject["is_running"] = flag;
  QByteArray jsonData = QJsonDocument(jsonObject).toJson();
  QUrl url(QString("http://%1:%2/pipelines/%3").arg(
      ui->lineEditIP->text(), QString::number(ui->spinBoxPort->value()),
      pipeLinesIdMap.value(ui->comboBoxSeletPipline->currentText())));
  qDebug() << "sendStartStop " << flag << " " << url << " " << jsonData;
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  pNetworkReply = manager.sendCustomRequest(request, "PATCH", jsonData);
  connect(pNetworkReply, &QNetworkReply::finished, [&]() {
    if (pNetworkReply->error() == QNetworkReply::NoError) {
      QByteArray responseData = pNetworkReply->readAll();
      qDebug() << "Response:" << responseData;
    }
    else {
      //            QMessageBox::warning(this, "警告",
      //                QString("sendStartStop " + pNetworkReply->errorString()));
      qDebug() << "Error:" << pNetworkReply->errorString();
    }
    pNetworkReply->deleteLater();
    loop.quit();
  });
  loop.exec();
}

void MainWindow::openWebSocket(quint64 port) {
  pThread = new ImageGet(QUrl(QString("ws://%1:%2").arg(
        ui->lineEditIP->text(), QString::number(port))), this);
  connect(pThread,
  &ImageGet::imageReceived, this, [&](const QString & message) {
    if (mutexForImageShow.tryLock()) {
      qDebug() << "&ImageGet::imageReceived" ;
      imagePro(message, &imageShow);
      mutexForImageShow.unlock();
    }
    else
      return;
  });
  connect(pThread,
    &ImageGet::statusSend, this, [&](const qint64 status,
  const QString & message) {
    if (status == ImageGet::LinkSuccess) {
      isRunning = 1;
      inputLock(true);
    }
    else if (status == ImageGet::LinkError) {
      qDebug() << "ImageGet::LinkError " << message;
      QMessageBox::warning(this, "警告", QString("ImageGet::LinkError " + message));
      inputLock(false);
      isRunning = 0;
    }
    else if (status == ImageGet::LinkDisconnected) {
      qDebug() << "ImageGet::LinkDisconnected " << message;
      //            QMessageBox::warning(this, "警告",
      //                QString("ImageGet::LinkDisconnected " + message));
      inputLock(false);
      isRunning = 0;
    }
    else if (status == ImageGet::LinkTimeout) {
      qDebug() << "ImageGet::LinkTimeout " << message;
      QMessageBox::warning(this, "警告",
        QString("ImageGet::LinkTimeout " + message));
      inputLock(false);
      isRunning = 0;
    }
  });
  pThread->start();
}
void MainWindow::closeWebSocket() {
  if (nullptr == pThread)
    return;
  if (!pThread->isRunning())
    return;
  pThread->exit();
  pThread->wait();
  pThread = nullptr;
}
void MainWindow::flashComboBoxSeletFile() {
  if (ui->comboBoxSeletPipline->count() < 1)
    return;
  QString SeletPipline = ui->comboBoxSeletPipline->currentText();
  const QMap<QString, QString>& innerMap = pipeLinesMap.value(SeletPipline);
  if (innerMap.isEmpty())
    return;
  ui->comboBoxSeletFile->clear();
  for (const QString& key : innerMap.keys())
    ui->comboBoxSeletFile->addItem(key);
}
void MainWindow::imagePro(const QString& message, QPixmap* imageOut) {
  /* 这个函数用于将base64转为image，但是其是由websocket通过信号槽触发执行的，并不是很推荐在此进行高负载 */
  QByteArray imageData = QByteArray::fromBase64(message.toLocal8Bit());
  QPixmap temp;
  temp.loadFromData(imageData, "JPEG");
  /* 通过copy操作模拟一些轻度负载 */
  *imageOut = temp.copy();
}

void MainWindow::inputLock(bool flag) {
  ui->pushButtonStart->setEnabled(!flag);
  //ui->pushButtonStop->setEnabled(flag);
  ui->lineEditIP->setEnabled(!flag);
  ui->spinBoxPort->setEnabled(!flag);
}
void MainWindow::on_comboBoxSeletFile_currentTextChanged(const QString& arg1) {
  getConfFile();
}
void MainWindow::on_comboBoxSeletPipline_currentTextChanged(
  const QString& arg1) {
  flashComboBoxSeletFile();
}
void MainWindow::on_pushButtonStart_clicked() {
  if (isRunning)
    return;
  if (ui->comboBoxSeletChannel->currentText().isEmpty())
    return;
  closeWebSocket();
  sendStartStop(true);
  openWebSocket(ui->comboBoxSeletChannel->currentText().toInt());
}
void MainWindow::on_pushButtonStop_clicked() {
  closeWebSocket();
  sendStartStop(false);
}
void MainWindow::on_pushButtonFlashChannel_clicked() {
  /* 找到通道 */
  for (const QString& item : pipeLinesMap.value(
      ui->comboBoxSeletPipline->currentText()).keys()) {
    if (item.contains("_demo")) {
      QString jsonStr = getConfFile(item);
      if (!jsonStr.isEmpty()) {
        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonStr.toUtf8());
        if ((!jsonDoc.isNull()) && (jsonDoc.isObject())) {
          QJsonObject jsonObject = jsonDoc.object();
          qDebug() << "jsonObject" << jsonObject;
          if (jsonObject.contains("channels") && jsonObject["channels"].isArray()) {
            QJsonArray channelsArray = jsonObject["channels"].toArray();
            channelIds.clear();
            for (const QJsonValue& channelValue : channelsArray) {
              if (channelValue.isObject()) {
                QJsonObject channelObject = channelValue.toObject();
                if (channelObject.contains("channel_id")
                  && channelObject["channel_id"].isDouble()) {
                  int channelId = channelObject["channel_id"].toInt();
                  channelIds.append(QString::number(channelId));
                }
              }
            }
            qDebug() << "channel_ids:" << channelIds;
          }
        }
        else
          qDebug() << "Invalid JSON format.";
      }
    }
  }
  /* 找到起始端口号 */
  for (const QString& item : pipeLinesMap.value(
      ui->comboBoxSeletPipline->currentText()).keys()) {
    if (item.contains("encode")) {
      QString jsonStr = getConfFile(item);
      QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonStr.toUtf8());
      if (!jsonDoc.isNull() && jsonDoc.isObject()) {
        QJsonObject jsonObject = jsonDoc.object();
        if (jsonObject.contains("configure") && jsonObject["configure"].isObject()) {
          QJsonObject configureObject = jsonObject["configure"].toObject();
          if (configureObject.contains("wss_port")) {
            QJsonValue wssPortValue = configureObject["wss_port"];
            if (wssPortValue.isString()) {
              wssPort = wssPortValue.toString();
              qDebug() << "wss_port:" << wssPort;
            }
          }
        }
      }
      else
        qDebug() << "Invalid JSON format.";
    }
  }
  if (channelIds.count() < 1)
    return;
  ui->comboBoxSeletChannel->clear();
  for (const QString& item : channelIds)
    ui->comboBoxSeletChannel->addItem(QString::number(item.toInt() +
        wssPort.toInt()));
}

void MainWindow::on_pushButtonSave_clicked() {
  putConfFile();
}

