/* ============================================================
 * File  : plugin_calendar.cpp
 * Author: Orgad Shaneh <orgads@gmail.com>
 * Date  : 2008-11-13
 * Description: plugin entry point
 *
 * Copyright 2008 by Orgad Shaneh

 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

// KDE includes.

#include <KDebug>
#include <KAction>
#include <KLibLoader>
#include <KApplication>
#include <KGenericFactory>
#include <KActionCollection>

// LibKIPI includes.

#include <libkipi/interface.h>

// Local includes.

#include "plugin_calendar.h"
#include "calwizard.h"

K_PLUGIN_FACTORY( CalendarFactory, registerPlugin<Plugin_Calendar>(); )
K_EXPORT_PLUGIN ( CalendarFactory("kipiplugin_calendar") )

Plugin_Calendar::Plugin_Calendar(QObject *parent, const QVariantList &)
               : KIPI::Plugin(CalendarFactory::componentData(), parent, "Calendar")
{
    kDebug( 51001 ) << "Plugin_Calendar plugin loaded";
}

Plugin_Calendar::~Plugin_Calendar()
{
}

void Plugin_Calendar::setup( QWidget* widget )
{
    KIPI::Plugin::setup(widget);

    m_actionCalendar = new KAction(KIcon("calendar"),
                                   i18n("Create Calendar..."),
                                   actionCollection());
    m_actionCalendar->setObjectName("calendar");
    connect(m_actionCalendar, SIGNAL(triggered(bool)),
            this, SLOT(slotActivate()));
    addAction(m_actionCalendar);

    m_interface = dynamic_cast< KIPI::Interface* >(parent());
    if (!m_interface)
    {
       kError( 51000 ) << "Kipi interface is null!" << endl;
       return;
    }
}

void Plugin_Calendar::slotActivate()
{
    KIPICalendarPlugin::CalWizard w( m_interface, kapp->activeWindow() );
    w.exec();
}

KIPI::Category Plugin_Calendar::category( KAction* action ) const
{
    if ( action == m_actionCalendar )
       return KIPI::ToolsPlugin;

    kWarning( 51000 ) << "Unrecognized action for plugin category identification";
    return KIPI::ToolsPlugin; // no warning from compiler, please
}
