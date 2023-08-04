//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "mainwindow.h"

#include <QApplication>
#include <QFont>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  MainWindow w;
  w.show();
  QFont defaultFont = a.font();
  defaultFont.setPointSize(16);
  a.setFont(defaultFont);
  return a.exec();
}
