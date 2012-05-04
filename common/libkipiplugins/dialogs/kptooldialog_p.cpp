/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2012-04-04
 * Description : Tool dialog private
 *
 * Copyright (C) 2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "kptooldialog_p.h"

// KDE includes

#include <kaction.h>
#include <khelpmenu.h>
#include <kicon.h>
#include <klocale.h>
#include <kmenu.h>
#include <kpushbutton.h>
#include <ktoolinvocation.h>

// Local includes

#include "kpaboutdata.h"

namespace KIPIPlugins
{

KPDialogPrivate::KPDialogPrivate(KDialog* const dlg)
{
    m_about  = 0;
    m_dialog = dlg;
}

KPDialogPrivate::~KPDialogPrivate()
{
    delete m_about;
}

void KPDialogPrivate::setAboutData(KPAboutData* const data, KPushButton* help)
{
    if (!data || !m_dialog) return;

    if (!help) help = m_dialog->button(KDialog::Help);
    if (!help) return;

    m_about = data;

    m_about->setHelpButton(help);
}

} // namespace KIPIPlugins
