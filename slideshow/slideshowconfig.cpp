/* ============================================================
 * File  : slideshowconfig.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-02-17
 * Description : KIPI slideshow plugin.
 *
 * Copyright 2003 by Renchi Raju
 *
 * Update: 09/07/2003 - Gilles Caulier <caulier dot gilles at free.fr>
                        Some changes in Dialog box.
                        Add about informations.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <qmap.h>

#include <klocale.h>
#include <kconfig.h>
#include <kmessagebox.h>

#include "slideshow.h"
#include "slideshowgl.h"
#include "slideshowconfig.h"

namespace KIPISlideShowPlugin
{

SlideShowConfig::SlideShowConfig()
    : KDialogBase(0, "", true, i18n("SlideShow"),
                  Help|User1|Ok|Cancel, Ok, true, i18n("&About"))
{
    config_ = 0;

    setHelp("slideshow", "kipi-plugins");
    QWidget *page = new QWidget( this );
    setMainWidget( page );

    QGridLayout *grid = new QGridLayout( page, 1, 1, 6, 6);

    // ------------------------------------------------------------------

    FileSrcButtonGroup_ = new QButtonGroup( page );
    FileSrcButtonGroup_->setRadioButtonExclusive( true );
    FileSrcButtonGroup_->setColumnLayout(0, Qt::Vertical );
    FileSrcButtonGroup_->layout()->setSpacing( 5 );
    FileSrcButtonGroup_->layout()->setMargin( 5 );
    QVBoxLayout* FileSrcButtonGroupLayout = new QVBoxLayout( FileSrcButtonGroup_->layout() );
    FileSrcButtonGroupLayout->setAlignment( Qt::AlignTop );

    // ------------------------------------------------------------------

    allFilesButton_ = new QRadioButton( FileSrcButtonGroup_);
    allFilesButton_->setText( i18n( "Show All Images in Current Album" ) );
    FileSrcButtonGroupLayout->addWidget( allFilesButton_ );

    // ------------------------------------------------------------------

    selectedFilesButton_ = new QRadioButton( FileSrcButtonGroup_);
    selectedFilesButton_->setText( i18n( "Show Only Selected Images" ) );
    FileSrcButtonGroupLayout->addWidget( selectedFilesButton_ );

    grid->addMultiCellWidget( FileSrcButtonGroup_, 0, 0, 0, 2 );

    // ------------------------------------------------------------------

    openglCheckBox_ = new QCheckBox( page );
    openglCheckBox_->setText( i18n( "Use OpenGL Slideshow Transitions" ) );
    grid->addMultiCellWidget( openglCheckBox_, 1, 1, 0, 2 );

    // ------------------------------------------------------------------

    printNameCheckBox_ = new QCheckBox( page );
    printNameCheckBox_->setText( i18n( "Print Filename" ) );
    grid->addMultiCellWidget( printNameCheckBox_, 2, 2, 0, 2 );

    // ------------------------------------------------------------------

    loopCheckBox_ = new QCheckBox( page );
    loopCheckBox_->setText( i18n( "Loop" ) );
    grid->addMultiCellWidget( loopCheckBox_, 3, 3, 0, 2 );

    // ------------------------------------------------------------------

    QLabel* label1 = new QLabel( page);
    label1->setText( i18n( "Delay between images (ms):" ) );
    grid->addWidget( label1, 4, 0 );

    // ------------------------------------------------------------------

    delaySpinBox_ = new QSpinBox( 1000, 10000, 10, page );
    delaySpinBox_->setSizePolicy(QSizePolicy( (QSizePolicy::SizeType)0,
                                              (QSizePolicy::SizeType)0));
    grid->addWidget( delaySpinBox_, 4, 1 );

    // ------------------------------------------------------------------

    QLabel* label2 = new QLabel( page, "label2" );
    label2->setText( i18n( "Transition effect:" ) );
    grid->addWidget( label2, 5, 0 );

    // ------------------------------------------------------------------

    effectsComboBox_ = new QComboBox( FALSE, page, "effectsComboBox_" );
    effectsComboBox_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7,
                                                  (QSizePolicy::SizeType)0 ) );
    grid->addMultiCellWidget( effectsComboBox_, 5, 5, 1, 2 );

    // ------------------------------------------------------------------

    connect(openglCheckBox_, SIGNAL(toggled(bool)), SLOT(slotOpenGLToggled()));
    connect(this, SIGNAL(okClicked()), this, SLOT(slotOkClicked()));
    connect(this, SIGNAL(user1Clicked()), this, SLOT(slotAboutClicked()));

    // ------------------------------------------------------------------

    config_ = new KConfig("kipirc");
    config_->setGroup("SlideShow Settings");

    readSettings();
}

SlideShowConfig::~SlideShowConfig()
{
    if (config_) {
        delete config_;
    }
}


void SlideShowConfig::loadEffectNames()
{
    effectsComboBox_->clear();

    QMap<QString,QString> effectNames = SlideShow::effectNamesI18N();
    QStringList effects;

    QMap<QString,QString>::Iterator it;
    for (it = effectNames.begin(); it != effectNames.end(); ++it)
        effects.append(it.data());

    effectsComboBox_->insertStringList(effects);

    for (int i=0; i<effectsComboBox_->count(); i++) {
        if (effectNames[effectName_] == effectsComboBox_->text(i)) {
            effectsComboBox_->setCurrentItem(i);
            break;
        }
    }
}

void SlideShowConfig::loadEffectNamesGL()
{
    effectsComboBox_->clear();

    QMap<QString,QString> effectNames = SlideShowGL::effectNamesI18N();
    QStringList effects;

    QMap<QString,QString>::Iterator it;
    for (it = effectNames.begin(); it != effectNames.end(); ++it)
        effects.append(it.data());

    effectsComboBox_->insertStringList(effects);

    for (int i=0; i<effectsComboBox_->count(); i++) {
        if (effectNames[effectNameGL_] == effectsComboBox_->text(i)) {
            effectsComboBox_->setCurrentItem(i);
            break;
        }
    }
}

void SlideShowConfig::readSettings()
{
    bool  opengl;
    int   delay;
    bool  printFileName;
    bool  loop;
    bool  showSelectedFilesOnly;

    opengl                = config_->readBoolEntry("OpenGL", false);
    delay                 = config_->readNumEntry("Delay", 1500);
    printFileName         = config_->readBoolEntry("Print Filename", true);
    loop                  = config_->readBoolEntry("Loop", false);
    showSelectedFilesOnly = config_->readBoolEntry("Show Selected Files Only", false);
    effectName_           = config_->readEntry("Effect Name", "Random");
    effectNameGL_         = config_->readEntry("Effect Name (OpenGL)", "Random");

    // -- Apply Settings to widgets ------------------------------

    openglCheckBox_->setChecked(opengl);

    delaySpinBox_->setValue(delay);

    printNameCheckBox_->setChecked(printFileName);

    loopCheckBox_->setChecked(loop);

    if (showSelectedFilesOnly)
        selectedFilesButton_->setChecked(true);
    else
        allFilesButton_->setChecked(true);

    slotOpenGLToggled();
}

void SlideShowConfig::saveSettings()
{
    if (!config_) return;

    config_->writeEntry("OpenGL", openglCheckBox_->isChecked());
    config_->writeEntry("Delay", delaySpinBox_->value());
    config_->writeEntry("Print Filename", printNameCheckBox_->isChecked());
    config_->writeEntry("Loop", loopCheckBox_->isChecked());
    config_->writeEntry("Show Selected Files Only", selectedFilesButton_->isChecked());

    if (!openglCheckBox_->isChecked()) {

        QString effect;
        QMap<QString,QString> effectNames = SlideShow::effectNamesI18N();
        QMap<QString,QString>::Iterator it;

        for (it = effectNames.begin(); it != effectNames.end(); ++it) {
            if (it.data() == effectsComboBox_->currentText()) {
                effect = it.key();
                break;
            }
        }

        config_->writeEntry("Effect Name", effect);

    }
    else {

        QString effect;
        QMap<QString,QString> effectNames = SlideShowGL::effectNamesI18N();
        QMap<QString,QString>::Iterator it;

        for (it = effectNames.begin(); it != effectNames.end(); ++it) {
            if (it.data() == effectsComboBox_->currentText()) {
                effect = it.key();
                break;
            }
        }

        config_->writeEntry("Effect Name (OpenGL)", effect);

    }

    config_->sync();
}

void SlideShowConfig::slotOpenGLToggled()
{
    if (openglCheckBox_->isChecked()) {
        loadEffectNamesGL();
        printNameCheckBox_->setEnabled(false);
    }
    else {
        loadEffectNames();
        printNameCheckBox_->setEnabled(true);
    }
}

void SlideShowConfig::slotOkClicked()
{
    saveSettings();
}

void SlideShowConfig::slotAboutClicked()
{
    KMessageBox::about(this, i18n("A KIPI plugin for image slideshows\n\n"
                                  "Author: Renchi Raju\n\n"
                                  "Email: renchi@pooh.tam.uiuc.edu"),
                                  i18n("About SlideShow"));
}

}  // NameSpace KIPISlideShowPlugin

#include "slideshowconfig.moc"
