/***************************************************** vim:set ts=4 sw=4 sts=4:
  Dialog to allow user to add a new Talker by selecting a language, synthesizer,
  and voice/pitch/speed options also.
  Uses addtalkerwidget.ui.
  -------------------
  Copyright: (C) 2004 by Gary Cramblitt <garycramblitt@comcast.net>
  Copyright: (C) 2009 by Jeremy Whiting <jpwhiting@kde.org>
  -------------------
  Original author: Gary Cramblitt <garycramblitt@comcast.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ******************************************************************************/

// KTTS includes.
#include "addtalker.h"

// Qt includes.
#include <QtGui/QDialog>

// KDE includes.
#include <kcombobox.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>

#include "kspeechinterface.h"

#include "ui_addtalkerwidget.h"

const int kLanguageColumn = 0;
const int kSynthesizerColumn = 1;

AddTalker::AddTalker(QWidget* parent)
    : KDialog(parent)
{
    this->setCaption(i18n("Add Talker"));
    this->setButtons(KDialog::Help|KDialog::Ok|KDialog::Cancel);
    this->setDefaultButton(KDialog::Cancel);
    this->enableButtonOk(false);
    this->setHelp("select-plugin", "jovie");

    QWidget * widget = new QWidget(this);
    mUi = new Ui::AddTalkerWidget;
    mUi->setupUi(widget);
    connect(mUi->AvailableTalkersTable, SIGNAL(itemSelectionChanged()), this, SLOT(slot_tableSelectionChanged()));
    this->setMainWidget(widget);

    org::kde::KSpeech* kspeech = new OrgKdeKSpeechInterface("org.kde.kttsd", "/KSpeech", QDBusConnection::sessionBus());

    m_outputModules = kspeech->outputModules();

    mUi->AvailableTalkersTable->setSortingEnabled(false);

    QString fullLanguageCode = KGlobal::locale()->defaultLanguage();
    QString languageCode;
    QString countryCode;
    TalkerCode::splitFullLanguageCode(fullLanguageCode, languageCode, countryCode);

    QTableWidgetItem * defaultItem = 0;

    foreach (const QString & module, m_outputModules)
    {
        QStringList languages = kspeech->languagesByModule(module);
        
        foreach (const QString & language, languages)
        {
            int rowcount = mUi->AvailableTalkersTable->rowCount();
            mUi->AvailableTalkersTable->setRowCount(rowcount + 1);
            
            // set the synthesizer item
            QTableWidgetItem * item = new QTableWidgetItem(module);
            mUi->AvailableTalkersTable->setItem(rowcount, kSynthesizerColumn, item);

            QString langName = TalkerCode::languageCodeToLanguage(language);
            if (language == languageCode)
            {
                defaultItem = item;
            }
            
            // set the language name item
            item = new QTableWidgetItem(langName.isEmpty() ? language : langName);
            item->setToolTip(language);
            mUi->AvailableTalkersTable->setItem(rowcount, kLanguageColumn, item);
        }
    }

    // turn sorting on now that the table is populated
    mUi->AvailableTalkersTable->setSortingEnabled(true);
    
    // sort by language by default
    mUi->AvailableTalkersTable->sortItems(kLanguageColumn);

    if (defaultItem)
    {
        mUi->AvailableTalkersTable->setCurrentItem(defaultItem);
    }
}

AddTalker::~AddTalker()
{
}

TalkerCode AddTalker::getTalkerCode() const
{
    TalkerCode retval;
    int row = mUi->AvailableTalkersTable->currentRow();
    if (row > 0 && row < mUi->AvailableTalkersTable->rowCount())
    {
        retval.setName(mUi->nameEdit->text());
        retval.setLanguage(mUi->AvailableTalkersTable->item(row, kLanguageColumn)->toolTip());
        retval.setVoiceType(mUi->voiceComboBox->currentIndex() + 1); // add 1 because the enumeration starts at 1
        retval.setVolume(mUi->volumeSlider->value());
        retval.setRate(mUi->speedSlider->value());
        retval.setPitch(mUi->pitchSlider->value());
        retval.setOutputModule(mUi->AvailableTalkersTable->item(row, kSynthesizerColumn)->text());
    }
    return retval;
}

void AddTalker::slot_tableSelectionChanged()
{
    this->enableButtonOk(true);
}

#include "addtalker.moc"

