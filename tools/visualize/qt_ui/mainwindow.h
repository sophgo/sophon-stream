//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMessageBox>
#include <QDebug>
#include <QComboBox>
#include <QVariantMap>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include <QList>
#include <QPixmap>
#include <QMutex>
#include <QTimer>
#include "jsonhighlighter.h"
#include "imageget.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow(QWidget* parent = nullptr);
  ~MainWindow();

 private slots:
  void on_pushButtonFlashAll_clicked();

  void on_comboBoxSelectFile_currentTextChanged(const QString& arg1);

  void on_comboBoxSelectPipline_currentTextChanged(const QString& arg1);

  void on_pushButtonStart_clicked();

  void on_pushButtonStop_clicked();

  void on_pushButtonFlashChannel_clicked();

  void on_pushButtonSave_clicked();

 private:
  Ui::MainWindow* ui;
  jsonHighlighter* pHighLighter;
  QNetworkAccessManager manager;
  QNetworkReply* pNetworkReply;
  /* pipeLine名称-><file名称->fileID> */
  QMap<QString, QMap<QString, QString>> pipeLinesMap;
  /* pipeLine名称->pipeLineID */
  QMap<QString, QString> pipeLinesIdMap;
  /* 当前全部通道值 */
  QStringList channelIds;
  /* ws起始端口 */
  QString wssPort;
  //QList<QMap<QString, QString>> pipeLineFilesMaps;
  /* 子线程指针 */
  ImageGet* pThread = nullptr;
  /* 正在运行标记位 */
  quint64 isRunning;
  /* 图片数据锁 */
  QMutex mutexForImageShow;
  /* 图片数据 */
  QPixmap imageShow;
  /* 循环显示图片数据 */
  QTimer timerForImageShow;
  QSize imageSize;

  //QMap<QString, QString> *pipeLineFilesMap;

  /* 获取Pipeline所有配置 */
  void getPipelines(void);
  /* 获取指定配置文件，未传递文件名自动从ui中获取 */
  QString getConfFile(QString fileName = QString("NONE"));
  /* 上传指定配置文件，未传递文件名自动从ui中获取 */
  QString putConfFile(QString fileName = QString("NONE"));
  /* 发送开始停止信号 */
  void sendStartStop(bool flag);
  /* 打开websocket线程，接收图片数据 */
  void openWebSocket(quint64 port);
  /* 关闭websocket线程 */
  void closeWebSocket(void);
  /* 刷新文件选择下拉框 */
  void flashComboBoxSelectFile(void);
  /* 预图片信息处理（轻负载） */
  void imagePro(const QString& message, QPixmap* imageOut);
  /* 用户输入锁 */
  void inputLock(bool flag);
};
#endif // MAINWINDOW_H
