//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef JSONHIGHLIGHTER_H
#define JSONHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QTextDocument>
#include <QFont>

class jsonHighlighter : public QSyntaxHighlighter {
 public:
  jsonHighlighter(QTextDocument* parent = nullptr);
 protected:
  void highlightBlock(const QString& text) override;

 private:
  QTextCharFormat m_keyFormat;
  QTextCharFormat m_stringFormat;
  QTextCharFormat m_numberFormat;
  QTextCharFormat m_booleanFormat;
  QTextCharFormat m_nullFormat;

};

#endif // JSONHIGHLIGHTER_H
