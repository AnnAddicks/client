/*
 * Copyright (C) by Daniel Molkentin <danimo@owncloud.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */

#define _WIN32_WINNT 0x0600
#define WINVER 0x0600
#include <shlobj.h>
#include <winbase.h>
#include <windows.h>
#include <winerror.h>
#include <shlguid.h>
#include <string>
#include <QLibrary>

static const char runPathC[] = "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run";

namespace OCC {

static void setupFavLink_private(const QString &folder)
{
    // Windows Explorer: Place under "Favorites" (Links)

    QString linkName;
    QDir folderDir(QDir::fromNativeSeparators(folder));

    /* Use new WINAPI functions */
    PWSTR path;

    if (SHGetKnownFolderPath(FOLDERID_Links, 0, NULL, &path) == S_OK) {
        QString links = QDir::fromNativeSeparators(QString::fromWCharArray(path));
        linkName = QDir(links).filePath(folderDir.dirName() + QLatin1String(".lnk"));
        CoTaskMemFree(path);
    }
    qCDebug(lcUtility) << " creating link from " << linkName << " to " << folder;
    if (!QFile::link(folder, linkName))
        qCWarning(lcUtility) << "linking" << folder << "to" << linkName << "failed!";
}


bool hasLaunchOnStartup_private(const QString &appName)
{
    QString runPath = QLatin1String(runPathC);
    QSettings settings(runPath, QSettings::NativeFormat);
    return settings.contains(appName);
}

void setLaunchOnStartup_private(const QString &appName, const QString &guiName, bool enable)
{
    Q_UNUSED(guiName);
    QString runPath = QLatin1String(runPathC);
    QSettings settings(runPath, QSettings::NativeFormat);
    if (enable) {
        settings.setValue(appName, QCoreApplication::applicationFilePath().replace('/', '\\'));
    } else {
        settings.remove(appName);
    }
}

static inline bool hasDarkSystray_private()
{
    return true;
}

} // namespace OCC
