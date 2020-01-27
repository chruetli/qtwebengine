/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtPDF module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QQUICKPDFSEARCHMODEL_P_H
#define QQUICKPDFSEARCHMODEL_P_H

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

#include "qquickpdfdocument_p.h"
#include "../api/qpdfsearchmodel.h"

#include <QVariant>
#include <QtQml/qqml.h>

QT_BEGIN_NAMESPACE

class QQuickPdfSearchModel : public QPdfSearchModel
{
    Q_OBJECT
    Q_PROPERTY(QQuickPdfDocument *document READ document WRITE setDocument NOTIFY documentChanged)
    Q_PROPERTY(int page READ page WRITE setPage NOTIFY pageChanged)
    Q_PROPERTY(QString searchString READ searchString WRITE setSearchString NOTIFY searchStringChanged)
    Q_PROPERTY(QVector<QPolygonF> matchGeometry READ matchGeometry NOTIFY matchGeometryChanged)

public:
    explicit QQuickPdfSearchModel(QObject *parent = nullptr);

    QQuickPdfDocument *document() const;
    void setDocument(QQuickPdfDocument * document);

    int page() const;
    void setPage(int page);

    QString searchString() const;
    void setSearchString(QString searchString);

    QVector<QPolygonF> matchGeometry() const;

signals:
    void documentChanged();
    void pageChanged();
    void searchStringChanged();
    void matchGeometryChanged();

private:
    void updateResults();

private:
    QQuickPdfDocument *m_quickDocument = nullptr;
    QString m_searchString;
    QVector<QPolygonF> m_matchGeometry;
    int m_page;

    Q_DISABLE_COPY(QQuickPdfSearchModel)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QQuickPdfSearchModel)

#endif // QQUICKPDFSEARCHMODEL_P_H
