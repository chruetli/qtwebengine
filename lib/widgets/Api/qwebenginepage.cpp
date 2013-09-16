/*
    Copyright (C) 2012, 2013 Digia Plc and/or its subsidiary(-ies).
    Copyright (C) 2008, 2009, 2012 Nokia Corporation and/or its subsidiary(-ies)
    Copyright (C) 2007 Staikos Computing Services Inc.
    Copyright (C) 2007 Apple Inc.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "qwebenginepage.h"
#include "qwebenginepage_p.h"

#include "qwebenginehistory.h"
#include "qwebenginehistory_p.h"
#include "qwebengineview.h"
#include "qwebengineview_p.h"
#include "render_widget_host_view_qt_delegate_widget.h"
#include "web_contents_adapter.h"

#include <QAction>
#include <QLayout>
#include <QUrl>

QT_BEGIN_NAMESPACE

QWebEnginePagePrivate::QWebEnginePagePrivate()
    : QObjectPrivate(QObjectPrivateVersion)
    , adapter(new WebContentsAdapter)
    , history(new QWebEngineHistory(new QWebEngineHistoryPrivate(adapter.data())))
    , view(0)
    , m_isLoading(false)
{
    adapter->initialize(this);
    memset(actions, 0, sizeof(actions));
}

QWebEnginePagePrivate::~QWebEnginePagePrivate()
{
    delete history;
}

RenderWidgetHostViewQtDelegate *QWebEnginePagePrivate::CreateRenderWidgetHostViewQtDelegate(CompositingMode mode)
{
    return new RenderWidgetHostViewQtDelegateWidget(mode);
}

void QWebEnginePagePrivate::titleChanged(const QString &title)
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->titleChanged(title);
}

void QWebEnginePagePrivate::urlChanged(const QUrl &url)
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->urlChanged(url);
}

void QWebEnginePagePrivate::loadingStateChanged()
{
    Q_Q(QWebEnginePage);
    const bool wasLoading = m_isLoading;
    m_isLoading = adapter->isLoading();
    if (m_isLoading != wasLoading) {
        if (m_isLoading)
            Q_EMIT q->loadStarted();
    }
    updateNavigationActions();
}

QRectF QWebEnginePagePrivate::viewportRect() const
{
    return view ? view->geometry() : QRectF();
}

void QWebEnginePagePrivate::loadFinished(bool success)
{
    Q_Q(QWebEnginePage);
    m_isLoading = adapter->isLoading();
    Q_EMIT q->loadFinished(success);
}

void QWebEnginePagePrivate::focusContainer()
{
    if (view)
        view->setFocus();
}

void QWebEnginePagePrivate::adoptNewWindow(WebContentsAdapter *newWebContents, WindowOpenDisposition disposition)
{
    Q_Q(QWebEnginePage);
    QWebEnginePage *newPage = q->createWindow(disposition == WebContentsAdapterClient::NewPopupDisposition ? QWebEnginePage::WebModalDialog : QWebEnginePage::WebBrowserWindow);
    // Overwrite the new page's WebContents with ours.
    if (newPage) {
        newPage->d_func()->adapter = newWebContents;
        newWebContents->initialize(newPage->d_func());
    }
}

void QWebEnginePagePrivate::updateAction(QWebEnginePage::WebAction action) const
{
#ifdef QT_NO_ACTION
    Q_UNUSED(action)
#else
    QAction *a = actions[action];
    if (!a)
        return;

    bool enabled = false;

    switch (action) {
    case QWebEnginePage::Back:
        enabled = adapter->canGoBack();
        break;
    case QWebEnginePage::Forward:
        enabled = adapter->canGoForward();
        break;
    case QWebEnginePage::Stop:
        enabled = adapter->isLoading();
        break;
    case QWebEnginePage::Reload:
    case QWebEnginePage::ReloadAndBypassCache:
        enabled = !adapter->isLoading();
        break;
    default:
        break;
    }

    a->setEnabled(enabled);
#endif // QT_NO_ACTION
}

void QWebEnginePagePrivate::updateNavigationActions()
{
    updateAction(QWebEnginePage::Back);
    updateAction(QWebEnginePage::Forward);
    updateAction(QWebEnginePage::Stop);
    updateAction(QWebEnginePage::Reload);
    updateAction(QWebEnginePage::ReloadAndBypassCache);
}

#ifndef QT_NO_ACTION
void QWebEnginePagePrivate::_q_webActionTriggered(bool checked)
{
    Q_Q(QWebEnginePage);
    QAction *a = qobject_cast<QAction *>(q->sender());
    if (!a)
        return;
    QWebEnginePage::WebAction action = static_cast<QWebEnginePage::WebAction>(a->data().toInt());
    q->triggerAction(action, checked);
}
#endif // QT_NO_ACTION

QWebEnginePage::QWebEnginePage(QObject* parent)
    : QObject(*new QWebEnginePagePrivate, parent)
{
}

QWebEnginePage::~QWebEnginePage()
{
}

QWebEngineHistory *QWebEnginePage::history() const
{
    Q_D(const QWebEnginePage);
    return d->history;
}

void QWebEnginePage::setView(QWidget *view)
{
    QWebEngineViewPrivate::bind(qobject_cast<QWebEngineView*>(view), this);
}

QWidget *QWebEnginePage::view() const
{
    Q_D(const QWebEnginePage);
    return d->view;
}

#ifndef QT_NO_ACTION
QAction *QWebEnginePage::action(WebAction action) const
{
    Q_D(const QWebEnginePage);
    if (action == QWebEnginePage::NoWebAction)
        return 0;
    if (d->actions[action])
        return d->actions[action];

    QString text;
    QIcon icon;
    QStyle *style = d->view ? d->view->style() : qApp->style();

    switch (action) {
    case Back:
        text = tr("Back");
        icon = style->standardIcon(QStyle::SP_ArrowBack);
        break;
    case Forward:
        text = tr("Forward");
        icon = style->standardIcon(QStyle::SP_ArrowForward);
        break;
    case Stop:
        text = tr("Stop");
        icon = style->standardIcon(QStyle::SP_BrowserStop);
        break;
    case Reload:
        text = tr("Reload");
        icon = style->standardIcon(QStyle::SP_BrowserReload);
        break;
    default:
        break;
    }

    QAction *a = new QAction(const_cast<QWebEnginePage*>(this));
    a->setText(text);
    a->setData(action);
    a->setIcon(icon);

    connect(a, SIGNAL(triggered(bool)), this, SLOT(_q_webActionTriggered(bool)));

    d->actions[action] = a;
    d->updateAction(action);
    return a;
}
#endif // QT_NO_ACTION

void QWebEnginePage::triggerAction(WebAction action, bool)
{
    Q_D(QWebEnginePage);
    switch (action) {
    case Back:
        d->adapter->navigateToOffset(-1);
        break;
    case Forward:
        d->adapter->navigateToOffset(1);
        break;
    case Stop:
        d->adapter->stop();
        break;
    case Reload:
        d->adapter->reload();
        break;
    default:
        Q_UNREACHABLE();
    }
}

void QWebEnginePage::load(const QUrl& url)
{
    Q_D(QWebEnginePage);
    d->adapter->load(url);
}

QString QWebEnginePage::title() const
{
    Q_D(const QWebEnginePage);
    return d->adapter->pageTitle();
}

void QWebEnginePage::setUrl(const QUrl &url)
{
    load(url);
}

QUrl QWebEnginePage::url() const
{
    Q_D(const QWebEnginePage);
    return d->adapter->activeUrl();
}

QWebEnginePage *QWebEnginePage::createWindow(WebWindowType type)
{
    Q_D(const QWebEnginePage);
    if (d->view) {
        QWebEngineView *newView = d->view->createWindow(type);
        if (newView)
            return newView->page();
    }
    return 0;
}

QT_END_NAMESPACE

#include "moc_qwebenginepage.cpp"
