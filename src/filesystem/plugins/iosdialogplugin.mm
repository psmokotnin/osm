/**
 *  OSM
 *  Copyright (C) 2021  Pavel Smokotnin

 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "iosdialogplugin.h"
#import <UIKit/UIKit.h>
#import <MobileCoreServices/MobileCoreServices.h>

#include <QQuickWindow>
#include <QDir>
#include <QFile>

@interface IosDialogPluginController : UIDocumentPickerViewController
    <UIDocumentPickerDelegate, UINavigationControllerDelegate>
- (instancetype)initWithPluginForOpen:(filesystem::IosDialogPlugin *)dialogPlugin;
- (instancetype)initWithPluginForSave:(filesystem::IosDialogPlugin *)dialogPlugin;
@end

namespace filesystem {

IosDialogPlugin::IosDialogPlugin(QObject *parent) : DialogPlugin(parent)
{
}

void IosDialogPlugin::open(const QQuickWindow *window)
{
    IosDialogPluginController *ui;

    switch (mode()) {
    case Dialog::Mode::OpenFile:
        ui = [[IosDialogPluginController alloc] initWithPluginForOpen: this];
        break;
    case Dialog::Mode::SaveFile:
        ui = [[IosDialogPluginController alloc] initWithPluginForSave: this];
        break;
    }

    if (ui) {
        UIWindow *uiwindow = reinterpret_cast<UIView *>(window->winId()).window;
        [uiwindow.rootViewController presentViewController:ui animated:YES completion:nil];
    }
}

} // namespace filesystem

@implementation IosDialogPluginController {
    filesystem::IosDialogPlugin *m_dialogPlugin;
    QFile *m_tempFile;
}

- (instancetype)initWithPluginForOpen:(filesystem::IosDialogPlugin *)dialogPlugin
{
    UIDocumentPickerMode importMode = UIDocumentPickerModeImport;

    NSMutableArray <NSString *> *docTypes = [[[NSMutableArray alloc] init] autorelease];
    [docTypes addObject:(__bridge NSString *)kUTTypeContent];
    [docTypes addObject:(__bridge NSString *)kUTTypeItem];
    [docTypes addObject:(__bridge NSString *)kUTTypeData];
    self = [super initWithDocumentTypes:docTypes inMode:importMode];
    if (self) {
        m_dialogPlugin = dialogPlugin;
        self.modalPresentationStyle = UIModalPresentationFormSheet;
        self.delegate = self;

        if (@available(ios 11.0, *)) {
            self.allowsMultipleSelection = NO;
        }

        if (@available(ios 13.0, *)) {
            self.directoryURL = m_dialogPlugin->folderURL().toNSURL();
        }
    }
    return self;
}

- (instancetype)initWithPluginForSave:(filesystem::IosDialogPlugin *)dialogPlugin
{
    m_tempFile = new QFile(QDir::tempPath() + "/untitled." + dialogPlugin->defaultSuffix());
    m_tempFile->open(QFile::WriteOnly);
    m_tempFile->write("\0", 1);
    m_tempFile->close();

    auto url = QUrl::fromLocalFile(m_tempFile->fileName());
    QList urls = {url};
    emit dialogPlugin->accepted(urls, {});
    if (QFile::exists(m_tempFile->fileName())) {
        NSURL *URL = url.toNSURL();
        UIDocumentPickerMode mode = UIDocumentPickerModeExportToService;

        self = [super initWithURL:URL inMode:mode];
    } else {
        qCritical() << "Document wasn't created";
        return nullptr;
    }
    if (self) {
        self.modalPresentationStyle = UIModalPresentationFormSheet;
        self.delegate = self;

        if (@available(ios 11.0, *)) {
            self.allowsMultipleSelection = NO;
        }

        if (@available(ios 13.0, *)) {
            self.directoryURL = dialogPlugin->folderURL().toNSURL();
        }
    }
    return self;
}

- (void)documentPicker:(UIDocumentPickerViewController *)controller didPickDocumentsAtURLs:(NSArray <NSURL *> *)urls
{
    Q_UNUSED(controller);
    QList<QUrl> files = {};
    for (NSURL *url : urls) {
        QUrl fileUrl = QUrl::fromNSURL(url);
        files.append(QUrl::fromNSURL(url));
    }
    if (m_tempFile) {
        m_tempFile->remove();
        m_tempFile = nullptr;
    }
    if (m_dialogPlugin && !files.empty()) {
        emit m_dialogPlugin->accepted(files, {});
    }
}

- (void)documentPickerWasCancelled:(UIDocumentPickerViewController *)controller
{
    Q_UNUSED(controller)
    if (m_tempFile) {
        m_tempFile->remove();
        m_tempFile = nullptr;
    }
    if (m_dialogPlugin) {
        emit m_dialogPlugin->rejected();
    }
}
@end;
