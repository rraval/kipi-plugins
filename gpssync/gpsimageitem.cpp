/* ============================================================
 *
 * Date        : 2010-03-21
 * Description : An item to hold information about an image
 *
 * Copyright (C) 2010 by Michael G. Hansen <mike at mghansen dot de>
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

#include "gpsimageitem.h"

// KDE includes

#include <klocale.h>

// LibKExiv2 includes

#include <libkexiv2/version.h>
#include <libkexiv2/kexiv2.h>

// local includes
#include "kdebug.h"
#include "kipiimagemodel.h"

namespace KIPIGPSSyncPlugin
{

GPSImageItem::GPSImageItem(KIPI::Interface* const interface, const KUrl& url, const bool autoLoad)
: KipiImageItem(interface, url, false), m_gpsData(), m_savedState(), m_dirty(false)
{
    if (autoLoad)
    {
        loadImageData();
    }
}

GPSImageItem::~GPSImageItem()
{
}

void GPSImageItem::loadImageDataInternal()
{
    KipiImageItem::loadImageDataInternal();

    m_gpsData.clear();

    if (m_interface)
    {
        // try to load the GPS data from the KIPI interface:
        QMap<QString, QVariant> attributes;
        KIPI::ImageInfo info = m_interface->info(m_url);
        attributes = info.attributes();

        if (attributes.contains("latitude") &&
            attributes.contains("longitude"))
        {
            m_gpsData.setLatLon(attributes["latitude"].toDouble(), attributes["longitude"].toDouble());
            if (attributes.contains("altitude"))
            {
                m_gpsData.setAltitude(attributes["altitude"].toDouble());
            }
        }
    }

    if (!m_gpsData.hasCoordinates())
    {
        // could not load the coordinates from the interface,
        // read them directly from the file
        openExiv2IFaceIfNotOpen(false);

        double alt, lat, lng;
        // TODO: handle missing altitude properly!
        bool infoIsValid = m_exiv2Iface->getGPSInfo(alt, lat, lng);
        if (infoIsValid)
        {
            m_gpsData.setCoordinates(KMapIface::WMWGeoCoordinate(lat, lng, alt));
        }
    }

    // mark us as not-dirty, because the data was just loaded:
    m_dirty = false;
    m_savedState = m_gpsData;
}

int getWarningLevelFromGPSDataContainer(const GPSDataContainer& data)
{
    if (data.hasPDop())
    {
        const int dopValue = data.getPDop();
        if (dopValue<2)
            return 1;
        if (dopValue<4)
            return 2;
        if (dopValue<10)
            return 3;
        return 4;
    }
    else if (data.hasHDop())
    {
        const int dopValue = data.getHDop();
        if (dopValue<2)
            return 1;
        if (dopValue<4)
            return 2;
        if (dopValue<10)
            return 3;
        return 4;
    }
    else if (data.hasFixType())
    {
        if (data.getFixType()<3)
            return 4;
    }
    else if (data.hasNSatellites())
    {
        if (data.getNSatellites()<4)
            return 4;
    }

    // no warning level
    return -1;
}

QVariant GPSImageItem::data(const int column, const int role) const
{
    if (role==RoleCoordinates)
    {
        return QVariant::fromValue(m_gpsData.getCoordinates());
    }
    else if ((column==ColumnLatitude)&&(role==Qt::DisplayRole))
    {
        if (!m_gpsData.getCoordinates().hasLatitude())
            return QString();

        return KGlobal::locale()->formatNumber(m_gpsData.getCoordinates().lat(), 7);
    }
    else if ((column==ColumnLongitude)&&(role==Qt::DisplayRole))
    {
        if (!m_gpsData.getCoordinates().hasLongitude())
            return QString();

        return KGlobal::locale()->formatNumber(m_gpsData.getCoordinates().lon(), 7);
    }
    else if ((column==ColumnAltitude)&&(role==Qt::DisplayRole))
    {
        if (!m_gpsData.getCoordinates().hasAltitude())
            return QString();

        return KGlobal::locale()->formatNumber(m_gpsData.getCoordinates().alt());
    }
    else if (column==ColumnAccuracy)
    {
        if (role==Qt::DisplayRole)
        {
            if (m_gpsData.hasPDop())
            {
                return i18n("PDOP: %1", m_gpsData.getPDop());
            }

            if (m_gpsData.hasHDop())
            {
                return i18n("HDOP: %1", m_gpsData.getHDop());
            }

            if (m_gpsData.hasFixType())
            {
                return i18n("Fix: %1d", m_gpsData.getFixType());
            }

            if (m_gpsData.hasNSatellites())
            {
                return i18n("#Sat: %1", m_gpsData.getNSatellites());
            }
        }
        else if (role==Qt::BackgroundRole)
        {
            const int warningLevel = getWarningLevelFromGPSDataContainer(m_gpsData);
            switch (warningLevel)
            {
            case 1:
                return QBrush(Qt::green);
            case 2:
                return QBrush(Qt::yellow);
            case 3:
                // orange
                return QBrush(QColor(0xff, 0x80, 0x00));
            case 4:
                return QBrush(Qt::red);
            default:
                break;
            }
        }
    }
    else if ((column==ColumnHDOP)&&(role==Qt::DisplayRole))
    {
        if (!m_gpsData.hasHDop())
            return QString();

        return KGlobal::locale()->formatNumber(m_gpsData.getHDop());
    }
    else if ((column==ColumnPDOP)&&(role==Qt::DisplayRole))
    {
        if (!m_gpsData.hasPDop())
            return QString();

        return KGlobal::locale()->formatNumber(m_gpsData.getPDop());
    }
    else if ((column==ColumnFixType)&&(role==Qt::DisplayRole))
    {
        if (!m_gpsData.hasFixType())
            return QString();

        return i18n("%1d", m_gpsData.getFixType());
    }
    else if ((column==ColumnNSatellites)&&(role==Qt::DisplayRole))
    {
        if (!m_gpsData.hasNSatellites())
            return QString();

        return KGlobal::locale()->formatNumber(m_gpsData.getNSatellites(), 0);
    }
    else if ((column==ColumnStatus)&&(role==Qt::DisplayRole))
    {
        if (m_dirty)
        {
            return i18n("Modified");
        }

        return QString();
    }
    else if ((column==ColumnTags)&&(role==Qt::DisplayRole))
    {
        if(!tags.isEmpty())
        {
            return tags;
        }
        
        return QString();        
    }

    return KipiImageItem::data(column, role);
}

bool GPSImageItem::setData(const int column, const int role, const QVariant& value)
{
    if (role==RoleCoordinates)
    {
        if (value.canConvert<KMapIface::WMWGeoCoordinate>())
        {
            m_gpsData.setCoordinates(value.value<KMapIface::WMWGeoCoordinate>());
            m_dirty = true;
        }
    }
    else
    {
        return KipiImageItem::setData(column, role, value);
    }

    return true;
}

void GPSImageItem::setCoordinates(const KMapIface::WMWGeoCoordinate& newCoordinates)
{
    m_gpsData.setCoordinates(newCoordinates);
    m_dirty = true;
    emitDataChanged();
}

void GPSImageItem::setHeaderData(KipiImageModel* const model)
{
    KipiImageItem::setHeaderData(model);

    model->setColumnCount(ColumnGPSImageItemCount);
    model->setHeaderData(ColumnLatitude, Qt::Horizontal, i18n("Latitude"), Qt::DisplayRole);
    model->setHeaderData(ColumnLongitude, Qt::Horizontal, i18n("Longitude"), Qt::DisplayRole);
    model->setHeaderData(ColumnAltitude, Qt::Horizontal, i18n("Altitude"), Qt::DisplayRole);
    model->setHeaderData(ColumnAccuracy, Qt::Horizontal, i18n("Accuracy"), Qt::DisplayRole);
    model->setHeaderData(ColumnHDOP, Qt::Horizontal, i18n("HDOP"), Qt::DisplayRole);
    model->setHeaderData(ColumnPDOP, Qt::Horizontal, i18n("PDOP"), Qt::DisplayRole);
    model->setHeaderData(ColumnFixType, Qt::Horizontal, i18n("Fix type"), Qt::DisplayRole);
    model->setHeaderData(ColumnNSatellites, Qt::Horizontal, i18n("# satellites"), Qt::DisplayRole);
    model->setHeaderData(ColumnStatus, Qt::Horizontal, i18n("Status"), Qt::DisplayRole);
    model->setHeaderData(ColumnTags, Qt::Horizontal, i18n("Tags"), Qt::DisplayRole);
}

QString GPSImageItem::saveChanges()
{
    // determine what is to be done first
    bool shouldRemoveCoordinates = false;
    bool shouldRemoveAltitude = false;
    bool shouldWriteCoordinates = false;
    bool shouldWriteAltitude = false;
    qreal altitude = 0;
    qreal latitude = 0;
    qreal longitude = 0;

    // do we have gps information?
    if (m_gpsData.hasCoordinates())
    {
        shouldWriteCoordinates = true;
        latitude = m_gpsData.getCoordinates().lat();
        longitude = m_gpsData.getCoordinates().lon();
        
        if (m_gpsData.hasAltitude())
        {
            shouldWriteAltitude = true;
            altitude = m_gpsData.getCoordinates().alt();
        }
        else
        {
            shouldRemoveAltitude = true;
        }
    }
    else
    {
        shouldRemoveCoordinates = true;
        shouldRemoveAltitude = true;
    }

    QString returnString;
    
    // first try to write the information to the image file
    KExiv2Iface::KExiv2* const exiv2Iface = new KExiv2Iface::KExiv2;
    bool success = exiv2Iface->load(m_url.path());
    if (!success)
    {
        // TODO: more verbosity!
        returnString = i18n("Failed to open file.");
    }
    if (success)
    {
        exiv2Iface->setWriteRawFiles(m_interface->hostSetting("WriteMetadataToRAW").toBool());

#if KEXIV2_VERSION >= 0x000600
        exiv2Iface->setUpdateFileTimeStamp(m_interface->hostSetting("WriteMetadataUpdateFiletimeStamp").toBool());
#endif

        if (shouldWriteCoordinates)
        {
            // TODO: write the altitude only if we have it
            // TODO: write HDOP and #satellites
            success = exiv2Iface->setGPSInfo(altitude, latitude, longitude);
            if (!success)
            {
                returnString = i18n("Failed to add GPS info to image");
            }
        }
        if (shouldRemoveCoordinates)
        {
            // TODO: remove only the altitude if requested
            success = exiv2Iface->removeGPSInfo();
            if (!success)
            {
                returnString = i18n("Failed to remove GPS info from image");
            }
        }
    }
    if (success)
    {
        success = exiv2Iface->save(m_url.path());
        if (!success)
        {
            returnString = i18n("Unable to save changes to file");
        }
        else
        {
            m_dirty = false;
            m_savedState = m_gpsData;
        }
    }

    delete exiv2Iface;

    // now tell the interface about the changes
    // TODO: remove the altitude if it is not available
    if (m_interface)
    {
        if (shouldWriteCoordinates)
        {
            QMap<QString, QVariant> attributes;
            attributes.insert("latitude", latitude);
            attributes.insert("longitude", longitude);
            if (shouldWriteAltitude)
            {
                attributes.insert("altitude", altitude);
            }

            KIPI::ImageInfo info = m_interface->info(m_url);
            info.addAttributes(attributes);
        }

        if (shouldRemoveCoordinates)
        {
            QStringList listToRemove;
            listToRemove << "gpslocation";
            KIPI::ImageInfo info = m_interface->info(m_url);
            info.delAttributes(listToRemove);
        }
    }

    if (returnString.isEmpty())
    {
        // mark all changes as not dirty and tell the model:
        emitDataChanged();
    }

    return returnString;
}

/**
 * @brief Restore the gps data to @p container. Sets m_dirty to false if container equals savedState.
 */
void GPSImageItem::restoreGPSData(const GPSDataContainer& container)
{
    m_dirty = !(container == m_savedState);
    m_gpsData = container;
    emitDataChanged();
}

void GPSImageItem::restoreRGTagList(QStringList& tagList)
{
    m_dirty = !(tagList == m_tagList);
    m_tagList = tagList;
    //emit RGTagListChanged(); 

}

bool GPSImageItem::lessThan(const KipiImageItem* const otherItem, const int column) const
{
    const GPSImageItem* const otherGPSItem = dynamic_cast<const GPSImageItem*>(otherItem);
    if (!otherGPSItem)
        return false;

    switch (column)
    {
    case ColumnAltitude:
    {
        if (!m_gpsData.hasAltitude())
            return false;

        if (!otherGPSItem->m_gpsData.hasAltitude())
            return true;

        return m_gpsData.getCoordinates().alt() < otherGPSItem->m_gpsData.getCoordinates().alt();
    }

    case ColumnNSatellites:
    {
        if (!m_gpsData.hasNSatellites())
            return false;

        if (!otherGPSItem->m_gpsData.hasNSatellites())
            return true;

        return m_gpsData.getNSatellites() < otherGPSItem->m_gpsData.getNSatellites();
    }

    case ColumnAccuracy:
    {
        const int myWarning = getWarningLevelFromGPSDataContainer(m_gpsData);
        const int otherWarning = getWarningLevelFromGPSDataContainer(otherGPSItem->m_gpsData);

        if (myWarning<0)
            return false;

        if (otherWarning<0)
            return true;

        if (myWarning!=otherWarning)
            return myWarning < otherWarning;

        // TODO: this may not be the best way to sort images with equal warning levels
        //       but it works for now

        if (m_gpsData.hasPDop()!=otherGPSItem->m_gpsData.hasPDop())
            return !m_gpsData.hasPDop();
        if (m_gpsData.hasPDop()&&otherGPSItem->m_gpsData.hasPDop())
        {
            return m_gpsData.getPDop()<otherGPSItem->m_gpsData.getPDop();
        }

        if (m_gpsData.hasHDop()!=otherGPSItem->m_gpsData.hasHDop())
            return !m_gpsData.hasHDop();
        if (m_gpsData.hasHDop()&&otherGPSItem->m_gpsData.hasHDop())
        {
            return m_gpsData.getHDop()<otherGPSItem->m_gpsData.getHDop();
        }

        if (m_gpsData.hasFixType()!=otherGPSItem->m_gpsData.hasFixType())
            return m_gpsData.hasFixType();
        if (m_gpsData.hasFixType()&&otherGPSItem->m_gpsData.hasFixType())
        {
            return m_gpsData.getFixType()>otherGPSItem->m_gpsData.getFixType();
        }

        if (m_gpsData.hasNSatellites()!=otherGPSItem->m_gpsData.hasNSatellites())
            return m_gpsData.hasNSatellites();
        if (m_gpsData.hasNSatellites()&&otherGPSItem->m_gpsData.hasNSatellites())
        {
            return m_gpsData.getNSatellites()>otherGPSItem->m_gpsData.getNSatellites();
        }

        return false;
    }

    case ColumnHDOP:
    {
        if (!m_gpsData.hasHDop())
            return false;

        if (!otherGPSItem->m_gpsData.hasHDop())
            return true;

        return m_gpsData.getHDop() < otherGPSItem->m_gpsData.getHDop();
    }

    case ColumnPDOP:
    {
        if (!m_gpsData.hasPDop())
            return false;

        if (!otherGPSItem->m_gpsData.hasPDop())
            return true;

        return m_gpsData.getPDop() < otherGPSItem->m_gpsData.getPDop();
    }

    case ColumnFixType:
    {
        if (!m_gpsData.hasFixType())
            return false;

        if (!otherGPSItem->m_gpsData.hasFixType())
            return true;

        return m_gpsData.getFixType() < otherGPSItem->m_gpsData.getFixType();
    }

    case ColumnLatitude:
    {
        if (!m_gpsData.hasCoordinates())
            return false;

        if (!otherGPSItem->m_gpsData.hasCoordinates())
            return true;

        return m_gpsData.getCoordinates().lat() < otherGPSItem->m_gpsData.getCoordinates().lat();
    }

    case ColumnLongitude:
    {
        if (!m_gpsData.hasCoordinates())
            return false;

        if (!otherGPSItem->m_gpsData.hasCoordinates())
            return true;

        return m_gpsData.getCoordinates().lon() < otherGPSItem->m_gpsData.getCoordinates().lon();
    }

    case ColumnStatus:
    {
        return m_dirty && !otherGPSItem->m_dirty;
    }

    default:
        return KipiImageItem::lessThan(otherItem, column);
    }
}

void GPSImageItem::setTagList(QStringList& externalTagData)
{
   this->m_tagList = externalTagData; 
   //this->imageTags.modifiedTags = externalTagData.tags;
}

QStringList GPSImageItem::getTagList()
{
    return this->m_tagList;
} 

} /* KIPIGPSSyncPlugin */

