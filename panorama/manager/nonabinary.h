/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2011-05-23
 * Description : Autodetect nona binary program and version
 *
 * Copyright (C) 2011-2015 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef NONABINARY_H
#define NONABINARY_H

// Local includes

#include "kpbinaryiface.h"

using namespace KIPIPlugins;

namespace KIPIPanoramaPlugin
{

class NonaBinary : public KPBinaryIface
{

public:

    NonaBinary()
        : KPBinaryIface(QString::fromUtf8("nona"), 
                        QString::fromUtf8("2010.4"),
                        QString::fromUtf8("nona version "),
                        2, 
                        QString::fromUtf8("Hugin"), 
                        QString::fromUtf8("http://hugin.sourceforge.net"),
                        QString::fromUtf8("Panorama"), 
                        QStringList(QString::fromUtf8("-h"))
                       )
        { 
            setup(); 
        }

    ~NonaBinary()
    {
    }
};

} // namespace KIPIPanoramaPlugin

#endif  // NONABINARY_H