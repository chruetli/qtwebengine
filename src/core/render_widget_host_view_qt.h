/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#ifndef RENDER_WIDGET_HOST_VIEW_QT_H
#define RENDER_WIDGET_HOST_VIEW_QT_H

#include "compositor/compositor.h"
#include "delegated_frame_host_client_qt.h"
#include "render_widget_host_view_qt_delegate.h"

#include "base/memory/weak_ptr.h"
#include "components/viz/common/resources/transferable_resource.h"
#include "components/viz/common/surfaces/parent_local_surface_id_allocator.h"
#include "components/viz/host/host_frame_sink_client.h"
#include "content/browser/accessibility/browser_accessibility_manager.h"
#include "content/browser/accessibility/web_contents_accessibility.h"
#include "content/browser/renderer_host/input/mouse_wheel_phase_handler.h"
#include "content/browser/renderer_host/render_widget_host_view_base.h"
#include "content/browser/renderer_host/text_input_manager.h"
#include "ui/events/gesture_detection/filtered_gesture_provider.h"

QT_FORWARD_DECLARE_CLASS(QAccessibleInterface)

namespace content {
class RenderFrameHost;
class RenderWidgetHostImpl;
}

namespace ui {
class TouchSelectionController;
}

namespace QtWebEngineCore {

class RenderWidgetHostViewQtDelegateClient;
class GuestInputEventObserverQt;
class TouchSelectionControllerClientQt;
class WebContentsAccessibilityQt;
class WebContentsAdapterClient;

class RenderWidgetHostViewQt
    : public content::RenderWidgetHostViewBase
    , public ui::GestureProviderClient
    , public base::SupportsWeakPtr<RenderWidgetHostViewQt>
    , public content::TextInputManager::Observer
    , public content::RenderFrameMetadataProvider::Observer
{
public:
    RenderWidgetHostViewQt(content::RenderWidgetHost* widget);
    ~RenderWidgetHostViewQt();

    RenderWidgetHostViewQtDelegate *delegate() { return m_delegate.get(); }
    void setDelegate(RenderWidgetHostViewQtDelegate *delegate);
    WebContentsAdapterClient *adapterClient() { return m_adapterClient; }
    void setAdapterClient(WebContentsAdapterClient *adapterClient);
    RenderWidgetHostViewQtDelegateClient *delegateClient() const { return m_delegateClient.get(); }
    void setGuest(content::RenderWidgetHostImpl *);

    void InitAsChild(gfx::NativeView) override;
    void InitAsPopup(content::RenderWidgetHostView*, const gfx::Rect&) override;
    void SetSize(const gfx::Size& size) override;
    void SetBounds(const gfx::Rect&) override;
    gfx::NativeView GetNativeView() override;
    gfx::NativeViewAccessible GetNativeViewAccessible() override;
    void Focus() override;
    bool HasFocus() override;
    bool IsMouseLocked() override;
    viz::FrameSinkId GetRootFrameSinkId() override;
    bool IsSurfaceAvailableForCopy() override;
    void CopyFromSurface(const gfx::Rect &src_rect,
                         const gfx::Size &output_size,
                         base::OnceCallback<void(const SkBitmap &)> callback) override;
    void Show() override;
    void Hide() override;
    bool IsShowing() override;
    gfx::Rect GetViewBounds() override;
    void UpdateBackgroundColor() override;
    blink::mojom::PointerLockResult LockMouse(bool) override;
    blink::mojom::PointerLockResult ChangeMouseLock(bool) override;
    void UnlockMouse() override;
    void UpdateCursor(const content::WebCursor&) override;
    void DisplayCursor(const content::WebCursor&) override;
    content::CursorManager *GetCursorManager() override;
    void SetIsLoading(bool) override;
    void ImeCancelComposition() override;
    void ImeCompositionRangeChanged(const gfx::Range&, const std::vector<gfx::Rect>&) override;
    void RenderProcessGone() override;
    bool TransformPointToCoordSpaceForView(const gfx::PointF &point,
                                           content::RenderWidgetHostViewBase *target_view,
                                           gfx::PointF *transformed_point) override;
    void Destroy() override;
    void UpdateTooltipUnderCursor(const std::u16string &tooltip_text) override;
    void UpdateTooltip(const std::u16string& tooltip_text) override;
    void WheelEventAck(const blink::WebMouseWheelEvent &event,
                       blink::mojom::InputEventResultState ack_result) override;
    void GestureEventAck(const blink::WebGestureEvent &event,
                         blink::mojom::InputEventResultState ack_result) override;
    content::MouseWheelPhaseHandler *GetMouseWheelPhaseHandler() override;
    viz::ScopedSurfaceIdAllocator DidUpdateVisualProperties(const cc::RenderFrameMetadata &metadata) override;
    void OnDidUpdateVisualPropertiesComplete(const cc::RenderFrameMetadata &metadata);

    // Overridden from RenderWidgetHostViewBase:
    void GetScreenInfo(display::ScreenInfo *screen_info) override;
    gfx::Rect GetBoundsInRootWindow() override;
    void ProcessAckedTouchEvent(const content::TouchEventWithLatencyInfo &touch,
                                blink::mojom::InputEventResultState ack_result) override;
    viz::SurfaceId GetCurrentSurfaceId() const override;
    const viz::FrameSinkId &GetFrameSinkId() const override;
    const viz::LocalSurfaceId &GetLocalSurfaceId() const override;
    void FocusedNodeChanged(bool is_editable_node, const gfx::Rect& node_bounds_in_screen) override;
    base::flat_map<std::string, std::string> GetKeyboardLayoutMap() override;

    void TakeFallbackContentFrom(content::RenderWidgetHostView *view) override;
    void EnsureSurfaceSynchronizedForWebTest() override;
    uint32_t GetCaptureSequenceNumber() const override;
    void ResetFallbackToFirstNavigationSurface() override;
    void DidStopFlinging() override;
    std::unique_ptr<content::SyntheticGestureTarget> CreateSyntheticGestureTarget() override;
    ui::Compositor *GetCompositor() override;
    absl::optional<content::DisplayFeature> GetDisplayFeature() override;
    void SetDisplayFeatureForTesting(const content::DisplayFeature*) override;
#if defined(OS_MAC)
    void ShowSharePicker(
        const std::string &title,
        const std::string &text,
        const std::string &url,
        const std::vector<std::string> &file_paths,
            blink::mojom::ShareService::ShareCallback callback) override { QT_NOT_YET_IMPLEMENTED }
    void SetActive(bool active) override { QT_NOT_YET_IMPLEMENTED }
    void SpeakSelection() override { QT_NOT_YET_IMPLEMENTED }
    void ShowDefinitionForSelection() override { QT_NOT_YET_IMPLEMENTED }
    void SetWindowFrameInScreen(const gfx::Rect&) override { QT_NOT_YET_IMPLEMENTED }
#endif // defined(OS_MAC)

    // Overridden from ui::GestureProviderClient.
    void OnGestureEvent(const ui::GestureEventData& gesture) override;

    // Overridden from content::TextInputManager::Observer
    void OnUpdateTextInputStateCalled(content::TextInputManager *text_input_manager, RenderWidgetHostViewBase *updated_view, bool did_update_state) override;
    void OnSelectionBoundsChanged(content::TextInputManager *text_input_manager, RenderWidgetHostViewBase *updated_view) override;
    void OnTextSelectionChanged(content::TextInputManager *text_input_manager, RenderWidgetHostViewBase *updated_view) override;

    // Overridden from content::BrowserAccessibilityDelegate
    content::WebContentsAccessibility *GetWebContentsAccessibility() override;

    // Overridden from content::RenderFrameMetadataProvider::Observer
    void OnRenderFrameMetadataChangedAfterActivation(base::TimeTicks activation_time) override;
    void OnRenderFrameMetadataChangedBeforeActivation(const cc::RenderFrameMetadata &) override {}
    void OnRenderFrameSubmission() override {}
    void OnLocalSurfaceIdChanged(const cc::RenderFrameMetadata &) override {}

    // Called from RenderWidgetHostViewQtDelegateClient.
    Compositor::Id compositorId();
    void notifyShown();
    void notifyHidden();
    bool updateScreenInfo();
    void handleWheelEvent(QWheelEvent *);
    void processMotionEvent(const ui::MotionEvent &motionEvent);
    void resetInputManagerState() { m_imState = 0; }

    // Called from WebContentsAdapter.
    gfx::SizeF lastContentsSize() const { return m_lastContentsSize; }
    gfx::Vector2dF lastScrollOffset() const { return m_lastScrollOffset; }

    ui::TouchSelectionController *getTouchSelectionController() const { return m_touchSelectionController.get(); }
    TouchSelectionControllerClientQt *getTouchSelectionControllerClient() const { return m_touchSelectionControllerClient.get(); }
    blink::mojom::FrameWidgetInputHandler *getFrameWidgetInputHandler();
    ui::TextInputType getTextInputType() const;

    void synchronizeVisualProperties(
            const absl::optional<viz::LocalSurfaceId> &childSurfaceId);

private:
    friend class DelegatedFrameHostClientQt;
    friend class WebContentsAccessibilityQt;

    bool isPopup() const;

    bool updateCursorFromResource(ui::mojom::CursorType type);

    scoped_refptr<base::SingleThreadTaskRunner> m_taskRunner;

    std::unique_ptr<content::CursorManager> m_cursorManager;

    ui::FilteredGestureProvider m_gestureProvider;
    std::unique_ptr<GuestInputEventObserverQt> m_guestInputEventObserver;

    viz::FrameSinkId m_frameSinkId;
    std::unique_ptr<RenderWidgetHostViewQtDelegateClient> m_delegateClient;
    std::unique_ptr<RenderWidgetHostViewQtDelegate> m_delegate;
    std::unique_ptr<WebContentsAccessibilityQt> m_webContentsAccessibility;
    QMetaObject::Connection m_adapterClientDestroyedConnection;
    WebContentsAdapterClient *m_adapterClient = nullptr;

    bool m_isMouseLocked = false;
    bool m_visible = false;
    bool m_deferredShow = false;
    gfx::Vector2dF m_lastScrollOffset;
    gfx::SizeF m_lastContentsSize;
    DelegatedFrameHostClientQt m_delegatedFrameHostClient { this };

    // VIZ
    display::ScreenInfo m_screenInfo;
    std::unique_ptr<content::DelegatedFrameHost> m_delegatedFrameHost;
    std::unique_ptr<ui::Layer> m_rootLayer;
    std::unique_ptr<ui::Compositor> m_uiCompositor;
    viz::ParentLocalSurfaceIdAllocator m_dfhLocalSurfaceIdAllocator;
    viz::ParentLocalSurfaceIdAllocator m_uiCompositorLocalSurfaceIdAllocator;

    // IME
    uint m_imState = 0;

    // Wheel
    bool m_wheelAckPending = false;
    QList<blink::WebMouseWheelEvent> m_pendingWheelEvents;
    content::MouseWheelPhaseHandler m_mouseWheelPhaseHandler { this };

    // TouchSelection
    std::unique_ptr<TouchSelectionControllerClientQt> m_touchSelectionControllerClient;
    std::unique_ptr<ui::TouchSelectionController> m_touchSelectionController;
    gfx::SelectionBound m_selectionStart;
    gfx::SelectionBound m_selectionEnd;

    base::WeakPtrFactory<RenderWidgetHostViewQt> m_weakPtrFactory { this };
};

class WebContentsAccessibilityQt : public content::WebContentsAccessibility
{
    RenderWidgetHostViewQt *m_rwhv;
public:
    WebContentsAccessibilityQt(RenderWidgetHostViewQt *rwhv) : m_rwhv(rwhv) {}
    QObject *accessibilityParentObject() const;
};

} // namespace QtWebEngineCore

#endif // RENDER_WIDGET_HOST_VIEW_QT_H
