/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtPDF module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QQUICKPDFDOCUMENT_P_H
#define QQUICKPDFDOCUMENT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtPdfQuick/private/qtpdfquickglobal_p.h>
#include <QtPdf/QPdfDocument>

#include <QtQml/QQmlEngine>
#include <QtQml/QQmlParserStatus>
#include <QtCore/QDateTime>
#include <QtCore/QUrl>

QT_BEGIN_NAMESPACE

class Q_PDFQUICK_EXPORT QQuickPdfDocument : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(int pageCount READ pageCount NOTIFY pageCountChanged FINAL)
    Q_PROPERTY(qreal maxPageWidth READ maxPageWidth NOTIFY metaDataChanged)
    Q_PROPERTY(qreal maxPageHeight READ maxPageHeight NOTIFY metaDataChanged)
    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged FINAL)
    Q_PROPERTY(QPdfDocument::Status status READ status NOTIFY statusChanged FINAL)
    Q_PROPERTY(QString error READ error NOTIFY statusChanged FINAL)

    Q_PROPERTY(QString title READ title NOTIFY metaDataChanged)
    Q_PROPERTY(QString subject READ subject NOTIFY metaDataChanged)
    Q_PROPERTY(QString author READ author NOTIFY metaDataChanged)
    Q_PROPERTY(QString keywords READ keywords NOTIFY metaDataChanged)
    Q_PROPERTY(QString producer READ producer NOTIFY metaDataChanged)
    Q_PROPERTY(QString creator READ creator NOTIFY metaDataChanged)
    Q_PROPERTY(QDateTime creationDate READ creationDate NOTIFY metaDataChanged)
    Q_PROPERTY(QDateTime modificationDate READ modificationDate NOTIFY metaDataChanged)
    QML_NAMED_ELEMENT(PdfDocument)
    QML_ADDED_IN_VERSION(5, 15)

public:
    explicit QQuickPdfDocument(QObject *parent = nullptr);
    ~QQuickPdfDocument() override;

    void classBegin() override {}
    void componentComplete() override;

    QUrl source() const { return m_source; }
    void setSource(QUrl source);
    QUrl resolvedSource() const { return m_resolvedSource; }

    int pageCount() const { return m_doc.pageCount(); }
    QPdfDocument::Status status() const { return m_doc.status(); }

    QString error() const;

    QString password() const { return m_doc.password(); }
    void setPassword(const QString &password);

    QString title() { return m_doc.metaData(QPdfDocument::Title).toString(); }
    QString author() { return m_doc.metaData(QPdfDocument::Author).toString(); }
    QString subject() { return m_doc.metaData(QPdfDocument::Subject).toString(); }
    QString keywords() { return m_doc.metaData(QPdfDocument::Keywords).toString(); }
    QString producer() { return m_doc.metaData(QPdfDocument::Producer).toString(); }
    QString creator() { return m_doc.metaData(QPdfDocument::Creator).toString(); }
    QDateTime creationDate() { return m_doc.metaData(QPdfDocument::CreationDate).toDateTime(); }
    QDateTime modificationDate() { return m_doc.metaData(QPdfDocument::ModificationDate).toDateTime(); }

    Q_INVOKABLE QSizeF pagePointSize(int page) const;
    qreal maxPageWidth() const;
    qreal maxPageHeight() const;
    Q_INVOKABLE qreal heightSumBeforePage(int page, qreal spacing = 0, int facingPages = 1) const;

Q_SIGNALS:
    void sourceChanged();
    void passwordChanged();
    void passwordRequired();
    void statusChanged();
    void pageCountChanged();
    void metaDataChanged();

private:
    QPdfDocument &document() { return m_doc; }
    void updateMaxPageSize();

private:
    QUrl m_source;
    QUrl m_resolvedSource;
    QPdfDocument m_doc;
    QSizeF m_maxPageWidthHeight;

    friend class QQuickPdfLinkModel;
    friend class QQuickPdfSearchModel;
    friend class QQuickPdfSelection;

    Q_DISABLE_COPY(QQuickPdfDocument)
};

QT_END_NAMESPACE

#endif // QQUICKPDFDOCUMENT_P_H
