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

#include "content_client_qt.h"

#include "base/command_line.h"
#include "base/files/file_util.h"
#include "base/strings/string_piece.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/version.h"
#include "content/public/common/cdm_info.h"
#include "content/public/common/content_constants.h"
#include "content/public/common/content_switches.h"
#include "extensions/buildflags/buildflags.h"
#include "extensions/common/constants.h"
#include "media/base/media_switches.h"
#include "media/base/video_codecs.h"
#include "media/media_buildflags.h"
#include "ui/base/layout.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/resource_bundle.h"
#include "type_conversion.h"

#include <QCoreApplication>
#include <QFile>
#include <QLibraryInfo>
#include <QString>

#if BUILDFLAG(ENABLE_LIBRARY_CDMS)
#include "media/cdm/cdm_paths.h"  // nogncheck
#include "third_party/widevine/cdm/buildflags.h"
#include "third_party/widevine/cdm/widevine_cdm_common.h"
#if BUILDFLAG(ENABLE_WIDEVINE) && !BUILDFLAG(ENABLE_WIDEVINE_CDM_COMPONENT)
#define WIDEVINE_CDM_AVAILABLE_NOT_COMPONENT

// File name of the CDM on different platforms.
const char kWidevineCdmFileName[] =
#if defined(OS_MAC)
    "widevinecdm.plugin";
#elif defined(OS_WIN)
    "widevinecdm.dll";
#else  // OS_LINUX, etc.
    "libwidevinecdm.so";
#endif
#endif
#endif  // BUILDFLAG(ENABLE_LIBRARY_CDMS)

#if QT_CONFIG(webengine_printing_and_pdf)
#include "pdf/pdf.h"
#include "pdf/pdf_ppapi.h"
const char kPdfPluginMimeType[] = "application/x-google-chrome-pdf";
const char kPdfPluginPath[] = "internal-pdf-viewer";
#endif // QT_CONFIG(webengine_printing_and_pdf)

using Robustness = content::CdmInfo::Robustness;

static QString webenginePluginsPath()
{
    // Look for plugins in /plugins/webengine or application dir.
    static bool initialized = false;
    static QString potentialPluginsPath = QLibraryInfo::location(QLibraryInfo::PluginsPath) % QLatin1String("/webengine");
    if (!initialized) {
        initialized = true;
        if (!QFileInfo::exists(potentialPluginsPath))
            potentialPluginsPath = QCoreApplication::applicationDirPath();
    }
    return potentialPluginsPath;
}

#if defined(Q_OS_WIN)
#include <shlobj.h>
static QString getLocalAppDataDir()
{
    QString result;
    wchar_t path[MAX_PATH];
    if (SHGetSpecialFolderPath(0, path, CSIDL_LOCAL_APPDATA, FALSE))
        result = QDir::fromNativeSeparators(QString::fromWCharArray(path));
    return result;
}

static QString getProgramFilesDir(bool x86Dir = false)
{
    QString result;
    wchar_t path[MAX_PATH];
    if (SHGetSpecialFolderPath(0, path, x86Dir ? CSIDL_PROGRAM_FILESX86 : CSIDL_PROGRAM_FILES, FALSE))
        result = QDir::fromNativeSeparators(QString::fromWCharArray(path));
    return result;
}
#endif

#if QT_CONFIG(webengine_pepper_plugins)

// The plugin logic is based on chrome/common/chrome_content_client.cc:
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

#include "content/public/common/pepper_plugin_info.h"
#include "ppapi/shared_impl/ppapi_permissions.h"

static QString ppapiPluginsPath()
{
    // Look for plugins in /plugins/ppapi or application dir.
    static bool initialized = false;
    static QString potentialPluginsPath = QLibraryInfo::location(QLibraryInfo::PluginsPath) % QLatin1String("/ppapi");
    if (!initialized) {
        initialized = true;
        if (!QFileInfo::exists(potentialPluginsPath))
            potentialPluginsPath = QCoreApplication::applicationDirPath();
    }
    return potentialPluginsPath;
}

void ComputeBuiltInPlugins(std::vector<content::PepperPluginInfo>* plugins)
{
#if QT_CONFIG(webengine_printing_and_pdf)
    content::PepperPluginInfo pdf_info;
    pdf_info.is_internal = true;
    pdf_info.is_out_of_process = true;
    pdf_info.name = "Chromium PDF Viewer";
    pdf_info.description = "Portable Document Format";
    pdf_info.path = base::FilePath::FromUTF8Unsafe(kPdfPluginPath);
    content::WebPluginMimeType pdf_mime_type(kPdfPluginMimeType, "pdf", "Portable Document Format");
    pdf_info.mime_types.push_back(pdf_mime_type);
    pdf_info.internal_entry_points.get_interface = chrome_pdf::PPP_GetInterface;
    pdf_info.internal_entry_points.initialize_module = chrome_pdf::PPP_InitializeModule;
    pdf_info.internal_entry_points.shutdown_module = chrome_pdf::PPP_ShutdownModule;
    pdf_info.permissions = ppapi::PERMISSION_DEV | ppapi::PERMISSION_PDF;
    plugins->push_back(pdf_info);
#endif // QT_CONFIG(webengine_printing_and_pdf)
}

namespace QtWebEngineCore {

void ContentClientQt::AddPepperPlugins(std::vector<content::PepperPluginInfo>* plugins)
{
    ComputeBuiltInPlugins(plugins);
}

} // namespace QtWebEngineCore
#endif // QT_CONFIG(webengine_pepper_plugins)

namespace QtWebEngineCore {

#if defined(WIDEVINE_CDM_AVAILABLE_NOT_COMPONENT)
static bool IsWidevineAvailable(base::FilePath *cdm_path,
                                media::CdmCapability *capability)
{
    QStringList pluginPaths;
    const base::CommandLine::StringType widevine_argument = base::CommandLine::ForCurrentProcess()->GetSwitchValueNative(switches::kCdmWidevinePath);
    if (!widevine_argument.empty())
        pluginPaths << QtWebEngineCore::toQt(widevine_argument);
    else {
        pluginPaths << webenginePluginsPath() + QStringLiteral("/") + QString::fromLatin1(kWidevineCdmFileName);
#if QT_CONFIG(webengine_pepper_plugins)
        pluginPaths << ppapiPluginsPath() + QStringLiteral("/") + QString::fromLatin1(kWidevineCdmFileName);
#endif
#if defined(Q_OS_OSX)
    QDir potentialWidevineDir("/Applications/Google Chrome.app/Contents/Frameworks");
    if (potentialWidevineDir.exists()) {
        QFileInfoList widevineVersionDirs = potentialWidevineDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot,
                                                                               QDir::Name | QDir::Reversed);
        const QString library = QLatin1String("/Versions/Current/Libraries/"
                                              "WidevineCdm/_platform_specific/mac_x64/libwidevinecdm.dylib");
        for (const QFileInfo &info : widevineVersionDirs)
            pluginPaths << info.absoluteFilePath() + library;
    }

    QDir oldPotentialWidevineDir(QDir::homePath() + "/Library/Application Support/Google/Chrome/WidevineCDM");
    if (oldPotentialWidevineDir.exists()) {
        QFileInfoList widevineVersionDirs = oldPotentialWidevineDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name | QDir::Reversed);
        for (int i = 0; i < widevineVersionDirs.size(); ++i) {
            QString versionDirPath(widevineVersionDirs.at(i).absoluteFilePath());
            QString potentialWidevinePluginPath = versionDirPath + "/_platform_specific/mac_x64/" + QString::fromLatin1(kWidevineCdmFileName);
            pluginPaths << potentialWidevinePluginPath;
        }
    }
#elif defined(Q_OS_WIN)
    const QString googleChromeDir = QLatin1String("/Google/Chrome/Application");
    const QStringList programFileDirs{getProgramFilesDir() + googleChromeDir,
                                      getProgramFilesDir(true) + googleChromeDir};
    for (const QString &dir : programFileDirs) {
        QDir d(dir);
        if (d.exists()) {
            QFileInfoList widevineVersionDirs = d.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name | QDir::Reversed);
            for (int i = 0; i < widevineVersionDirs.size(); ++i) {
                QString versionDirPath(widevineVersionDirs.at(i).absoluteFilePath());
#ifdef WIN64
                QString potentialWidevinePluginPath = versionDirPath +
                                                        "/WidevineCdm/_platform_specific/win_x64/" +
                                                        QString::fromLatin1(kWidevineCdmFileName);
#else
                QString potentialWidevinePluginPath = versionDirPath +
                                                        "/WidevineCdm/_platform_specific/win_x86/" +
                                                        QString::fromLatin1(kWidevineCdmFileName);
#endif
                pluginPaths << potentialWidevinePluginPath;
            }
        }
    }
    QDir potentialWidevineDir(getLocalAppDataDir() + "/Google/Chrome/User Data/WidevineCDM");
    if (potentialWidevineDir.exists()) {
        QFileInfoList widevineVersionDirs = potentialWidevineDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name | QDir::Reversed);
        for (int i = 0; i < widevineVersionDirs.size(); ++i) {
            QString versionDirPath(widevineVersionDirs.at(i).absoluteFilePath());
#ifdef WIN64
            QString potentialWidevinePluginPath = versionDirPath + "/_platform_specific/win_x64/" + QString::fromLatin1(kWidevineCdmFileName);
#else
            QString potentialWidevinePluginPath = versionDirPath + "/_platform_specific/win_x86/" + QString::fromLatin1(kWidevineCdmFileName);
#endif
            pluginPaths << potentialWidevinePluginPath;
        }
    }
#elif defined(Q_OS_LINUX)
        pluginPaths << QStringLiteral("/opt/google/chrome/libwidevinecdm.so") // Old Google Chrome
#if Q_PROCESSOR_WORDSIZE == 8
                    << QStringLiteral("/opt/google/chrome/WidevineCdm/_platform_specific/linux_x64/libwidevinecdm.so")
#else
                    << QStringLiteral("/opt/google/chrome/WidevineCdm/_platform_specific/linux_x86/libwidevinecdm.so")
#endif
                    << QStringLiteral("/usr/lib/chromium/libwidevinecdm.so") // Arch
                    << QStringLiteral("/usr/lib/chromium-browser/libwidevinecdm.so") // Ubuntu/neon
                    << QStringLiteral("/usr/lib64/chromium/libwidevinecdm.so"); // OpenSUSE style
#endif
    }

    for (const QString &pluginPath : qAsConst(pluginPaths)) {
        *cdm_path = QtWebEngineCore::toFilePath(pluginPath);
        if (base::PathExists(*cdm_path)) {
            // Add the supported codecs as if they came from the component manifest.
            // This list must match the CDM that is being bundled with Chrome.
            const std::vector<media::VideoCodecProfile> kAllProfiles = {};
            capability->video_codecs.emplace(media::VideoCodec::kCodecVP8, kAllProfiles);
            capability->video_codecs.emplace(media::VideoCodec::kCodecVP9, kAllProfiles);
            capability->video_codecs.emplace(media::VideoCodec::kCodecAV1, kAllProfiles);
#if BUILDFLAG(USE_PROPRIETARY_CODECS)
            capability->video_codecs.emplace(media::VideoCodec::kCodecH264, kAllProfiles);
#endif  // BUILDFLAG(USE_PROPRIETARY_CODECS)

            // Add the supported encryption schemes as if they came from the
            // component manifest. This list must match the CDM that is being
            // bundled with Chrome.
            capability->encryption_schemes.insert(media::EncryptionScheme::kCenc);
            capability->encryption_schemes.insert(media::EncryptionScheme::kCbcs);

            // Temporary session is always supported.
            capability->session_types.insert(media::CdmSessionType::kTemporary);

            return true;
        }
    }
    return false;
}
#endif  // defined(WIDEVINE_CDM_AVAILABLE_NOT_COMPONENT)

void ContentClientQt::AddContentDecryptionModules(std::vector<content::CdmInfo> *cdms,
                                                  std::vector<media::CdmHostFilePath> *cdm_host_file_paths)
{
    Q_UNUSED(cdm_host_file_paths);
    if (cdms) {
#if defined(WIDEVINE_CDM_AVAILABLE_NOT_COMPONENT)
        base::FilePath cdm_path;
        media::CdmCapability capability;
        if (IsWidevineAvailable(&cdm_path, &capability)) {
            const base::Version version;
            cdms->push_back(content::CdmInfo(kWidevineKeySystem, Robustness::kSoftwareSecure, std::move(capability),
                                             /*supports_sub_key_systems=*/false, kWidevineCdmDisplayName,
                                             kWidevineCdmGuid, version, cdm_path, kWidevineCdmFileSystemId));
        }
#endif  // defined(WIDEVINE_CDM_AVAILABLE_NOT_COMPONENT)

#if BUILDFLAG(ENABLE_LIBRARY_CDMS)
        // Register Clear Key CDM if specified in command line.
        base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
        base::FilePath clear_key_cdm_path = command_line->GetSwitchValuePath(switches::kClearKeyCdmPathForTesting);
        if (!clear_key_cdm_path.empty() && base::PathExists(clear_key_cdm_path)) {
            // TODO(crbug.com/764480): Remove these after we have a central place for
            // External Clear Key (ECK) related information.
            // Normal External Clear Key key system.
            const char kExternalClearKeyKeySystem[] = "org.chromium.externalclearkey";
            // A variant of ECK key system that has a different GUID.
            const char kExternalClearKeyDifferentGuidTestKeySystem[] =
                    "org.chromium.externalclearkey.differentguid";

            // Supported codecs are hard-coded in ExternalClearKeyProperties.
            media::CdmCapability capability(
                {}, {}, {media::EncryptionScheme::kCenc, media::EncryptionScheme::kCbcs},
                {media::CdmSessionType::kTemporary,
                 media::CdmSessionType::kPersistentLicense});

            // Register kExternalClearKeyDifferentGuidTestKeySystem first separately.
            // Otherwise, it'll be treated as a sub-key-system of normal
            // kExternalClearKeyKeySystem. See MultipleCdmTypes test in
            // ECKEncryptedMediaTest.
            cdms->push_back(content::CdmInfo(kExternalClearKeyDifferentGuidTestKeySystem,
                                             Robustness::kSoftwareSecure, capability,
                                             /*supports_sub_key_systems=*/false, media::kClearKeyCdmDisplayName,
                                             media::kClearKeyCdmDifferentGuid, base::Version("0.1.0.0"),
                                             clear_key_cdm_path, media::kClearKeyCdmFileSystemId));

            cdms->push_back(content::CdmInfo(kExternalClearKeyKeySystem,
                                             Robustness::kSoftwareSecure, capability,
                                             /*supports_sub_key_systems=*/true, media::kClearKeyCdmDisplayName,
                                             media::kClearKeyCdmGuid, base::Version("0.1.0.0"),
                                             clear_key_cdm_path, media::kClearKeyCdmFileSystemId));
        }
#endif  // BUILDFLAG(ENABLE_LIBRARY_CDMS)
    }
}

void ContentClientQt::AddAdditionalSchemes(Schemes* schemes)
{
    // Matching ChromeContentClient::AddAdditionalSchemes
    schemes->standard_schemes.push_back(extensions::kExtensionScheme);
    schemes->secure_schemes.push_back(extensions::kExtensionScheme);

#if BUILDFLAG(ENABLE_EXTENSIONS)
    schemes->service_worker_schemes.push_back(extensions::kExtensionScheme);
    schemes->cors_enabled_schemes.push_back(extensions::kExtensionScheme);
    schemes->csp_bypassing_schemes.push_back(extensions::kExtensionScheme);
#endif
}

base::StringPiece ContentClientQt::GetDataResource(int resource_id, ui::ResourceScaleFactor scale_factor)
{
    return ui::ResourceBundle::GetSharedInstance().GetRawDataResourceForScale(resource_id, scale_factor);
}

base::RefCountedMemory *ContentClientQt::GetDataResourceBytes(int resource_id)
{
    return ui::ResourceBundle::GetSharedInstance().LoadDataResourceBytes(resource_id);
}

gfx::Image &ContentClientQt::GetNativeImageNamed(int resource_id)
{
    return ui::ResourceBundle::GetSharedInstance().GetNativeImageNamed(resource_id);
}

std::u16string ContentClientQt::GetLocalizedString(int message_id)
{
    return l10n_util::GetStringUTF16(message_id);
}

} // namespace QtWebEngineCore
