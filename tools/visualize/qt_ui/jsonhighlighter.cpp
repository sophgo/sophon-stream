//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "jsonhighlighter.h"

jsonHighlighter::jsonHighlighter(QTextDocument* parent) : QSyntaxHighlighter(
    parent) {
  m_keyFormat.setForeground(Qt::darkBlue);
  QFont f = parent->defaultFont();
  f.setBold(true);
  f.setPointSize(14);
  m_keyFormat.setFont(f);
  m_stringFormat.setForeground(Qt::darkGreen);
  m_numberFormat.setForeground(Qt::darkMagenta);
  m_booleanFormat.setForeground(Qt::darkYellow);
  m_nullFormat.setForeground(Qt::gray);
}

void jsonHighlighter::highlightBlock(const QString& text) {
  //匹配值
  QRegularExpression regex_1("(\".*?\")|(-?\\d+\\.?\\d*)|(true|false)|(null)");
  int index_1 = 0;
  while (index_1 >= 0) {
    QRegularExpressionMatch match = regex_1.match(text, index_1);
    if (!match.hasMatch())
      break;
    if (match.capturedStart(1) != -1)   // String
      setFormat(match.capturedStart(1), match.capturedLength(1), m_stringFormat);
    else if (match.capturedStart(2) != -1)   // Number
      setFormat(match.capturedStart(2), match.capturedLength(2), m_numberFormat);
    else if (match.capturedStart(3) != -1)   // Boolean
      setFormat(match.capturedStart(3), match.capturedLength(3), m_booleanFormat);
    else if (match.capturedStart(4) != -1)   // Null
      setFormat(match.capturedStart(4), match.capturedLength(4), m_nullFormat);
    index_1 = match.capturedEnd();
  }
  //匹配键
  QRegularExpression regex("(\"\\w+\"):\\s*");
  int index = 0;
  while (index >= 0) {
    QRegularExpressionMatch match = regex.match(text, index);
    if (!match.hasMatch())
      break;
    int start = match.capturedStart(1);
    int length = match.capturedLength(1);
    setFormat(start, length, m_keyFormat);
    index = match.capturedEnd();
  }
}
