/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.kipi-plugins.org
 *
 * Date        : 2006-10-11
 * Description : a plugin to edit pictures metadata
 *
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2011 by Victor Dodon <dodon dot victor at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "plugin_metadataedit.moc"

// Qt includes

#include <QPointer>

// KDE includes

#include <kaction.h>
#include <kactioncollection.h>
#include <kactionmenu.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kgenericfactory.h>
#include <kglobalsettings.h>
#include <klibloader.h>
#include <klocale.h>
#include <kmenu.h>
#include <kmessagebox.h>

// LibKIPI includes

#include <libkipi/imagecollection.h>
#include <libkipi/interface.h>

// LibKExiv2 includes

#include <libkexiv2/version.h>
#include <libkexiv2/kexiv2.h>

// Local includes

#include "metadataedit.h"

using namespace KExiv2Iface;
using namespace KIPIMetadataEditPlugin;

K_PLUGIN_FACTORY( MetadataEditFactory, registerPlugin<Plugin_MetadataEdit>(); )
K_EXPORT_PLUGIN ( MetadataEditFactory("kipiplugin_metadataedit") )

Plugin_MetadataEdit::Plugin_MetadataEdit(QObject* parent, const QVariantList&)
                   : Plugin(MetadataEditFactory::componentData(), parent, "MetadataEdit"),
                     m_actionMetadataEdit(0),
                     m_interface(0),
                     m_lastSelectedDirectory()
{
    kDebug(AREA_CODE_LOADING) << "Plugin_MetadataEdit plugin loaded";
}

void Plugin_MetadataEdit::setup(QWidget* widget)
{
    Plugin::setup(widget);

    m_actionMetadataEdit = new KActionMenu(KIcon("metadataedit"), i18n("Metadata"), actionCollection());
    m_actionMetadataEdit->setObjectName("metadataedit");


    KAction* metadataEdit = actionCollection()->addAction("editallmetadata");
    metadataEdit->setText(i18n("Edit &All Metadata..."));
    connect(metadataEdit, SIGNAL(triggered(bool)),
            this,SLOT(slotEditAllMetadata()));
    m_actionMetadataEdit->addAction(metadataEdit);

    // ---------------------------------------------------

    m_actionMetadataEdit->menu()->addSeparator();

    KAction* importEXIF = actionCollection()->addAction("importexif");
    importEXIF->setText(i18n("Import EXIF..."));
    connect(importEXIF, SIGNAL(triggered(bool)),
            this, SLOT(slotImportExif()));
    m_actionMetadataEdit->addAction(importEXIF);

    KAction* importIPTC = actionCollection()->addAction("importiptc");
    importIPTC->setText(i18n("Import IPTC..."));
    connect(importIPTC, SIGNAL(triggered(bool)),
            this, SLOT(slotImportIptc()));
    m_actionMetadataEdit->addAction(importIPTC);

    KAction* importXMP = actionCollection()->addAction("importxmp");
    importXMP->setText(i18n("Import XMP..."));
    connect(importXMP, SIGNAL(triggered(bool)),
            this, SLOT(slotImportXmp()));
    m_actionMetadataEdit->addAction(importXMP);

    // ------------------------------------------------------

    addAction( m_actionMetadataEdit );

    m_interface = dynamic_cast<Interface*>( parent() );
    if ( !m_interface )
    {
        kError() << "Kipi interface is null!";
        return;
    }

    ImageCollection selection = m_interface->currentSelection();
    m_actionMetadataEdit->setEnabled( selection.isValid() && !selection.images().isEmpty() );

    connect(m_interface, SIGNAL(selectionChanged(bool)),
            m_actionMetadataEdit, SLOT(setEnabled(bool)));
}

void Plugin_MetadataEdit::slotEditAllMetadata()
{
    ImageCollection images = m_interface->currentSelection();

    if ( !images.isValid() || images.images().isEmpty() )
        return;

    QPointer<MetadataEditDialog> dialog = new MetadataEditDialog(kapp->activeWindow(), images.images(), m_interface);
    dialog->exec();
    m_interface->refreshImages(images.images());

    delete dialog;
}

void Plugin_MetadataEdit::slotImportExif()
{
    ImageCollection images = m_interface->currentSelection();

    if ( !images.isValid() || images.images().isEmpty() )
        return;

    // extract the path to the first image:
    if ( m_lastSelectedDirectory.isEmpty() )
    {
        m_lastSelectedDirectory = images.images().first().upUrl();
    }
    KUrl importEXIFFile = KFileDialog::getOpenUrl(m_lastSelectedDirectory,
                                                  QString(), kapp->activeWindow(),
                                                  i18n("Select File to Import EXIF metadata") );
    if( importEXIFFile.isEmpty() )
       return;

    m_lastSelectedDirectory = importEXIFFile.upUrl();

    KExiv2 exiv2Iface;
    if (!exiv2Iface.load(importEXIFFile.path()))
    {
        KMessageBox::error(kapp->activeWindow(),
                           i18n("Cannot load metadata from \"%1\"", importEXIFFile.fileName()),
                           i18n("Import EXIF Metadata"));
        return;
    }

#if KEXIV2_VERSION >= 0x010000
    QByteArray exifData = exiv2Iface.getExifEncoded();
#else
    QByteArray exifData = exiv2Iface.getExif();
#endif

    if (exifData.isEmpty())
    {
        KMessageBox::error(kapp->activeWindow(),
                           i18n("\"%1\" does not have EXIF metadata.", importEXIFFile.fileName()),
                           i18n("Import EXIF Metadata"));
        return;
    }

    if (KMessageBox::warningYesNo(
                     kapp->activeWindow(),
                     i18n("EXIF metadata from the currently selected pictures will be permanently "
                          "replaced by the EXIF content of \"%1\".\n"
                          "Do you want to continue?", importEXIFFile.fileName()),
                     i18n("Import EXIF Metadata")) != KMessageBox::Yes)
        return;

    KUrl::List  imageURLs = images.images();
    KUrl::List  updatedURLs;
    QStringList errorFiles;

    for( KUrl::List::iterator it = imageURLs.begin() ;
         it != imageURLs.end(); ++it)
    {
        KUrl url = *it;
        bool ret = false;

        if (KExiv2::canWriteExif(url.path()))
        {
            ret = true;
            KExiv2 exiv2Iface;
            exiv2Iface.setWriteRawFiles(m_interface->hostSetting("WriteMetadataToRAW").toBool());

#if KEXIV2_VERSION >= 0x000600
            exiv2Iface.setUpdateFileTimeStamp(m_interface->hostSetting("WriteMetadataUpdateFiletimeStamp").toBool());
#endif

            ret &= exiv2Iface.load(url.path());
            ret &= exiv2Iface.setExif(exifData);
            ret &= exiv2Iface.save(url.path());
        }

        if (!ret)
            errorFiles.append(url.fileName());
        else
            updatedURLs.append(url);
    }

    // We use kipi interface refreshImages() method to tell to host than
    // metadata from pictures have changed and need to be re-read.

    m_interface->refreshImages(updatedURLs);

    if (!errorFiles.isEmpty())
    {
        KMessageBox::errorList(
                    kapp->activeWindow(),
                    i18n("Unable to set EXIF metadata from:"),
                    errorFiles,
                    i18n("Import EXIF Metadata"));
    }
}

void Plugin_MetadataEdit::slotImportIptc()
{
    ImageCollection images = m_interface->currentSelection();

    if ( !images.isValid() || images.images().isEmpty() )
        return;

    // extract the path to the first image:
    if ( m_lastSelectedDirectory.isEmpty() )
    {
        m_lastSelectedDirectory = images.images().first().upUrl();
    }
    KUrl importIPTCFile = KFileDialog::getOpenUrl(m_lastSelectedDirectory,
                                                  QString(), kapp->activeWindow(),
                                                  i18n("Select File to Import IPTC metadata") );
    if( importIPTCFile.isEmpty() )
       return;

    m_lastSelectedDirectory = importIPTCFile.upUrl();

    KExiv2 exiv2Iface;
    if (!exiv2Iface.load(importIPTCFile.path()))
    {
        KMessageBox::error(kapp->activeWindow(),
                           i18n("Cannot load metadata from \"%1\"", importIPTCFile.fileName()),
                           i18n("Import IPTC Metadata"));
        return;
    }

    QByteArray iptcData = exiv2Iface.getIptc();
    if (iptcData.isEmpty())
    {
        KMessageBox::error(kapp->activeWindow(),
                           i18n("\"%1\" does not have IPTC metadata.", importIPTCFile.fileName()),
                           i18n("Import IPTC Metadata"));
        return;
    }

    if (KMessageBox::warningYesNo(
                     kapp->activeWindow(),
                     i18n("IPTC metadata from the currently selected pictures will be permanently "
                          "replaced by the IPTC content of \"%1\".\n"
                          "Do you want to continue?", importIPTCFile.fileName()),
                     i18n("Import IPTC Metadata")) != KMessageBox::Yes)
        return;

    KUrl::List  imageURLs = images.images();
    KUrl::List  updatedURLs;
    QStringList errorFiles;

    for( KUrl::List::iterator it = imageURLs.begin() ;
         it != imageURLs.end(); ++it)
    {
        KUrl url = *it;
        bool ret = false;

        if (KExiv2::canWriteIptc(url.path()))
        {
            ret = true;
            KExiv2 exiv2Iface;
            exiv2Iface.setWriteRawFiles(m_interface->hostSetting("WriteMetadataToRAW").toBool());

#if KEXIV2_VERSION >= 0x000600
            exiv2Iface.setUpdateFileTimeStamp(m_interface->hostSetting("WriteMetadataUpdateFiletimeStamp").toBool());
#endif

            ret &= exiv2Iface.load(url.path());
            ret &= exiv2Iface.setIptc(iptcData);
            ret &= exiv2Iface.save(url.path());
        }

        if (!ret)
            errorFiles.append(url.fileName());
        else
            updatedURLs.append(url);
    }

    // We use kipi interface refreshImages() method to tell to host than
    // metadata from pictures have changed and need to be re-read.

    m_interface->refreshImages(updatedURLs);

    if (!errorFiles.isEmpty())
    {
        KMessageBox::errorList(
                    kapp->activeWindow(),
                    i18n("Unable to set IPTC metadata from:"),
                    errorFiles,
                    i18n("Import IPTC Metadata"));
    }
}

void Plugin_MetadataEdit::slotImportXmp()
{
    ImageCollection images = m_interface->currentSelection();

    if ( !images.isValid() || images.images().isEmpty() )
        return;

    // extract the path to the first image:
    if ( m_lastSelectedDirectory.isEmpty() )
    {
        m_lastSelectedDirectory = images.images().first().upUrl();
    }
    KUrl importXMPFile = KFileDialog::getOpenUrl(m_lastSelectedDirectory,
                                                 QString(), kapp->activeWindow(),
                                                 i18n("Select File to Import XMP metadata") );
    if( importXMPFile.isEmpty() )
       return;

    m_lastSelectedDirectory = importXMPFile.upUrl();

    KExiv2 exiv2Iface;
    if (!exiv2Iface.load(importXMPFile.path()))
    {
        KMessageBox::error(kapp->activeWindow(),
                           i18n("Cannot load metadata from \"%1\"", importXMPFile.fileName()),
                           i18n("Import XMP Metadata"));
        return;
    }

    QByteArray xmpData = exiv2Iface.getXmp();
    if (xmpData.isEmpty())
    {
        KMessageBox::error(kapp->activeWindow(),
                           i18n("\"%1\" does not have XMP metadata.", importXMPFile.fileName()),
                           i18n("Import XMP Metadata"));
        return;
    }

    if (KMessageBox::warningYesNo(
                     kapp->activeWindow(),
                     i18n("XMP metadata from the currently selected pictures will be permanently "
                          "replaced by the XMP content of \"%1\".\n"
                          "Do you want to continue?", importXMPFile.fileName()),
                     i18n("Import XMP Metadata")) != KMessageBox::Yes)
        return;

    KUrl::List  imageURLs = images.images();
    KUrl::List  updatedURLs;
    QStringList errorFiles;

    for( KUrl::List::iterator it = imageURLs.begin() ;
         it != imageURLs.end(); ++it)
    {
        KUrl url = *it;
        bool ret = false;

        if (KExiv2::canWriteXmp(url.path()))
        {
            ret = true;
            KExiv2 exiv2Iface;
            exiv2Iface.setWriteRawFiles(m_interface->hostSetting("WriteMetadataToRAW").toBool());

#if KEXIV2_VERSION >= 0x000600
            exiv2Iface.setUpdateFileTimeStamp(m_interface->hostSetting("WriteMetadataUpdateFiletimeStamp").toBool());
#endif

            ret &= exiv2Iface.load(url.path());
            ret &= exiv2Iface.setXmp(xmpData);
            ret &= exiv2Iface.save(url.path());
        }

        if (!ret)
            errorFiles.append(url.fileName());
        else
            updatedURLs.append(url);
    }

    // We use kipi interface refreshImages() method to tell to host than
    // metadata from pictures have changed and need to be re-read.

    m_interface->refreshImages(updatedURLs);

    if (!errorFiles.isEmpty())
    {
        KMessageBox::errorList(
                    kapp->activeWindow(),
                    i18n("Unable to set XMP metadata from:"),
                    errorFiles,
                    i18n("Import XMP Metadata"));
    }
}

Category Plugin_MetadataEdit::category(KAction* action) const
{
    if ( action == m_actionMetadataEdit )
       return ImagesPlugin;

    kWarning() << "Unrecognized action for plugin category identification";
    return ImagesPlugin; // no warning from compiler, please
}