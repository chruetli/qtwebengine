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

#ifndef CLIPBOARD_QT_H
#define CLIPBOARD_QT_H

#include "ui/base/clipboard/clipboard.h"

namespace QtWebEngineCore {

class ClipboardQt : public ui::Clipboard
{
public:
    const ui::ClipboardSequenceNumberToken &GetSequenceNumber(ui::ClipboardBuffer type) const override;
    bool IsFormatAvailable(const ui::ClipboardFormatType &format,
                           ui::ClipboardBuffer buffer,
                           const ui::DataTransferEndpoint *data_dst) const override;
    void Clear(ui::ClipboardBuffer type) override;
    void ReadAvailableTypes(ui::ClipboardBuffer type,
                            const ui::DataTransferEndpoint *data_dst,
                            std::vector<std::u16string> *types) const override;
    void ReadText(ui::ClipboardBuffer type, const ui::DataTransferEndpoint *data_dst, std::u16string *result) const override;
    void ReadAsciiText(ui::ClipboardBuffer type, const ui::DataTransferEndpoint *data_dst, std::string *result) const override;
    void ReadHTML(ui::ClipboardBuffer type, const ui::DataTransferEndpoint *data_dst, std::u16string *markup, std::string *src_url, uint32_t *fragment_start,
                  uint32_t *fragment_end) const override;
    void ReadRTF(ui::ClipboardBuffer type, const ui::DataTransferEndpoint *data_dst, std::string *result) const override;
    void ReadImage(ui::ClipboardBuffer buffer, const ui::DataTransferEndpoint *data_dst, ReadImageCallback callback) const override;
    void ReadCustomData(ui::ClipboardBuffer clipboard_type, const std::u16string &type, const ui::DataTransferEndpoint *data_dst, std::u16string *result) const override;
    void ReadBookmark(const ui::DataTransferEndpoint *data_dst, std::u16string *title, std::string *url) const override;
    void ReadData(const ui::ClipboardFormatType &format, const ui::DataTransferEndpoint *data_dst, std::string *result) const override;
#if defined(USE_OZONE)
    bool IsSelectionBufferAvailable() const override;
#endif
    void OnPreShutdown() override {}
    void ReadSvg(ui::ClipboardBuffer, const ui::DataTransferEndpoint *, std::u16string *) const override;
    void ReadPng(ui::ClipboardBuffer, const ui::DataTransferEndpoint *, ui::Clipboard::ReadPngCallback) const override;
    std::vector<std::u16string> ReadAvailablePlatformSpecificFormatNames(ui::ClipboardBuffer buffer, const ui::DataTransferEndpoint *data_dst) const override;


    const ui::DataTransferEndpoint *GetSource(ui::ClipboardBuffer buffer) const override;
    void ReadFilenames(ui::ClipboardBuffer buffer,
                       const ui::DataTransferEndpoint *data_dst,
                       std::vector<ui::FileInfo> *result) const override;

protected:
    void WritePortableAndPlatformRepresentations(
        ui::ClipboardBuffer buffer,
        const ObjectMap &objects,
        std::vector<Clipboard::PlatformRepresentation> platform_representations,
        std::unique_ptr<ui::DataTransferEndpoint> data_src) override;
    void WriteText(const char *text_data, size_t text_len) override;
    void WriteHTML(const char *markup_data, size_t markup_len, const char *url_data, size_t url_len) override;
    void WriteRTF(const char *rtf_data, size_t data_len) override;
    void WriteBookmark(const char *title_data, size_t title_len, const char *url_data, size_t url_len) override;
    void WriteWebSmartPaste() override;
    void WriteBitmap(const SkBitmap &bitmap) override;
    void WriteData(const ui::ClipboardFormatType &format, const char *data_data, size_t data_len) override;
    void WriteSvg(const char *, size_t) override;
    void WriteFilenames(std::vector<ui::FileInfo> filenames) override;

    base::flat_map<ui::ClipboardBuffer, std::unique_ptr<ui::DataTransferEndpoint>> m_dataSrc;
};

} // namespace QtWebEngineCore

#endif
