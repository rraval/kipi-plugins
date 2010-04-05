/* ============================================================
 *
 * Date        : 2010-03-22
 * Description : Drag and drop handler for the image list
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

#ifndef GPSIMAGELISTDRAGDROPHANDLER_H
#define GPSIMAGELISTDRAGDROPHANDLER_H

// Qt includes

#include <QTreeView>

// local includes

#include "mapdragdrophandler.h"
#include "kipiimagelist.h"

namespace KIPIGPSSyncPlugin
{

class GPSImageListDragDropHandler : public KipiImageListDragDropHandler
{
Q_OBJECT

public:
    GPSImageListDragDropHandler(QObject* const parent = 0);
    ~GPSImageListDragDropHandler();

    virtual QMimeData* createMimeData(const QList<QPersistentModelIndex>& modelIndices);
};

} /* GPSIMAGELISTDRAGDROPHANDLER_H */

#endif /* GPSIMAGELISTDRAGDROPHANDLER_H */

