/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
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
#ifndef WEB_ENGINE_PAGE_RENDER_FRAME_H
#define WEB_ENGINE_PAGE_RENDER_FRAME_H

#include "content/public/renderer/render_frame_observer.h"
#include "mojo/public/cpp/bindings/associated_receiver.h"
#include "qtwebengine/browser/qtwebenginepage.mojom.h"

namespace content {
class RenderFrame;
}

namespace QtWebEngineCore {

class WebEnginePageRenderFrame : private content::RenderFrameObserver,
                                 public qtwebenginepage::mojom::WebEnginePageRenderFrame
{
public:
    WebEnginePageRenderFrame(content::RenderFrame *render_frame);
    WebEnginePageRenderFrame(const WebEnginePageRenderFrame &) = delete;
    WebEnginePageRenderFrame &operator=(const WebEnginePageRenderFrame &) = delete;

private:
    void FetchDocumentMarkup(uint64_t requestId, FetchDocumentMarkupCallback callback) override;
    void FetchDocumentInnerText(uint64_t requestId,
                                FetchDocumentInnerTextCallback callback) override;
    void SetBackgroundColor(uint32_t color) override;
    void OnDestruct() override;
    void DidMeaningfulLayout(blink::WebMeaningfulLayout layout_type) override;
    void WasShown() override;
    void WasHidden() override;
    void
    BindReceiver(mojo::PendingAssociatedReceiver<qtwebenginepage::mojom::WebEnginePageRenderFrame>
                         receiver);

private:
    mojo::AssociatedReceiver<qtwebenginepage::mojom::WebEnginePageRenderFrame> m_binding;
    bool m_needsLayout;
};
} // namespace

#endif // WEB_ENGINE_PAGE_RENDER_FRAME_H
