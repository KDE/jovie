/***************************************************** vim:set ts=4 sw=4 sts=4:
  Description: 
     A dialog for user to select a Talker, either by specifying
     selected Talker attributes, or by specifying all attributes
     of an existing configured Talker.

  Copyright:
  (C) 2005 by Gary Cramblitt <garycramblitt@comcast.net>
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
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 ******************************************************************************/

// Qt includes.
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qhbox.h>
#include <qgroupbox.h>
#include <qlayout.h>

// KDE includes.
#include <kcombobox.h>
#include <ktrader.h>
#include <kpushbutton.h>
#include <klistview.h>
#include <klineedit.h>
#include <kconfig.h>
#include <kdebug.h>

// KTTS includes.
#include "utils.h"
#include "selecttalkerdlg.h"

#include "selecttalkerdlg.moc"

SelectTalkerDlg::SelectTalkerDlg(
    QWidget* parent,
    const char* name,
    const QString& caption,
    const QString& talkerCode,
    bool runningTalkers) :

    KDialogBase(
        parent,
        name,
        true,
        caption,
        KDialogBase::Ok|KDialogBase::Cancel,
        KDialogBase::Ok)
{
    m_widget = new SelectTalkerWidget( this );
    setInitialSize( QSize(700,600) );
    setMainWidget( m_widget );
    // TODO: Why won't KDialogBase properly size itself?
    m_runningTalkers = runningTalkers;
    m_talkerCode = TalkerCode( talkerCode, false );

    // Fill combo boxes.
    KComboBox* cb = m_widget->genderComboBox;
    cb->insertItem( QString::null );
    cb->insertItem( TalkerCode::translatedGender("male") );
    cb->insertItem( TalkerCode::translatedGender("female") );
    cb->insertItem( TalkerCode::translatedGender("neutral") );

    cb = m_widget->volumeComboBox;
    cb->insertItem( QString::null );
    cb->insertItem( TalkerCode::translatedVolume("medium") );
    cb->insertItem( TalkerCode::translatedVolume("loud") );
    cb->insertItem( TalkerCode::translatedVolume("soft") );

    cb = m_widget->rateComboBox;
    cb->insertItem( QString::null );
    cb->insertItem( TalkerCode::translatedRate("medium") );
    cb->insertItem( TalkerCode::translatedRate("fast") );
    cb->insertItem( TalkerCode::translatedRate("slow") );

    cb = m_widget->synthComboBox;
    cb->insertItem( QString::null );
    KTrader::OfferList offers = KTrader::self()->query("KTTSD/SynthPlugin");
    for(unsigned int i=0; i < offers.count() ; ++i)
        cb->insertItem(offers[i]->name());

    // Fill List View with list of Talkers.
    m_widget->talkersListView->setSorting( -1 );
    loadTalkers( m_runningTalkers );

    // Set initial radio button state.
    if ( talkerCode.isEmpty() )
        m_widget->useDefaultRadioButton->setChecked(true);
    else
    {
        QString dummy;
        if (talkerCode == TalkerCode::normalizeTalkerCode(talkerCode, dummy))
            m_widget->useSpecificTalkerRadioButton->setChecked(true);
        else
            m_widget->useClosestMatchRadioButton->setChecked(true);
    }

    applyTalkerCodeToControls();
    enableDisableControls();

    connect(m_widget->useDefaultRadioButton, SIGNAL(clicked()),
            this, SLOT(configChanged()));
    connect(m_widget->useClosestMatchRadioButton, SIGNAL(clicked()),
            this, SLOT(configChanged()));
    connect(m_widget->useSpecificTalkerRadioButton, SIGNAL(clicked()),
            this, SLOT(configChanged()));

    connect(m_widget->languageBrowseButton, SIGNAL(clicked()),
            this, SLOT(slotLanguageBrowseButton_clicked()));

    connect(m_widget->synthComboBox, SIGNAL(activated(const QString&)),
            this, SLOT(configChanged()));
    connect(m_widget->genderComboBox, SIGNAL(activated(const QString&)),
            this, SLOT(configChanged()));
    connect(m_widget->volumeComboBox, SIGNAL(activated(const QString&)),
            this, SLOT(configChanged()));
    connect(m_widget->rateComboBox, SIGNAL(activated(const QString&)),
            this, SLOT(configChanged()));

    connect(m_widget->synthCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(configChanged()));
    connect(m_widget->genderCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(configChanged()));
    connect(m_widget->volumeCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(configChanged()));
    connect(m_widget->rateCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(configChanged()));

    connect(m_widget->talkersListView, SIGNAL(selectionChanged()),
            this, SLOT(slotTalkersListView_selectionChanged()));

    m_widget->talkersListView->setMinimumHeight( 120 );
}

SelectTalkerDlg::~SelectTalkerDlg() { }

QString SelectTalkerDlg::getSelectedTalkerCode()
{
    return m_talkerCode.getTalkerCode();
}

QString SelectTalkerDlg::getSelectedTranslatedDescription()
{
    return m_talkerCode.getTranslatedDescription();
}

void SelectTalkerDlg::slotLanguageBrowseButton_clicked()
{
    // Create a  QHBox to host KListView.
    QHBox* hBox = new QHBox(m_widget, "SelectLanguage_hbox");
    // Create a KListView and fill with all known languages.
    KListView* langLView = new KListView(hBox, "SelectLanguage_lview");
    langLView->addColumn(i18n("Language"));
    langLView->addColumn(i18n("Code"));
    langLView->setSelectionMode(QListView::Single);
    QStringList allLocales = KGlobal::locale()->allLanguagesTwoAlpha();
    QString locale;
    QString language;
    // Blank line so user can select no language.
    // Note: Don't use QString::null, which gets displayed at bottom of list, rather than top.
    QListViewItem* item = new KListViewItem(langLView, "", "");
    if (m_talkerCode.languageCode().isEmpty()) item->setSelected(true);
    int allLocalesCount = allLocales.count();
    for (int ndx=0; ndx < allLocalesCount; ndx++)
    {
        locale = allLocales[ndx];
        language = TalkerCode::languageCodeToLanguage(locale);
        item = new KListViewItem(langLView, language, locale);
        if (m_talkerCode.fullLanguageCode() == locale) item->setSelected(true);
    }
    // Sort by language.
    langLView->setSorting(0);
    langLView->sort();
    // Display the box in a dialog.
    KDialogBase* dlg = new KDialogBase(
        KDialogBase::Swallow,
        i18n("Select Languages"),
        KDialogBase::Help|KDialogBase::Ok|KDialogBase::Cancel,
        KDialogBase::Cancel,
        m_widget,
        "SelectLanguage_dlg",
        true,
        true);
    dlg->setMainWidget(hBox);
    dlg->setHelp("", "kttsd");
    dlg->setInitialSize(QSize(300, 500), false);
    // TODO: This isn't working.  Furthermore, item appears selected but is not.
    langLView->ensureItemVisible(langLView->selectedItem());
    int dlgResult = dlg->exec();
    language = QString::null;
    if (dlgResult == QDialog::Accepted)
    {
        if (langLView->selectedItem())
        {
            language = langLView->selectedItem()->text(0);
            m_talkerCode.setFullLanguageCode( langLView->selectedItem()->text(1) );
        }
    }
    delete dlg;
    m_widget->languageLineEdit->setText(language);
    m_widget->languageCheckBox->setChecked( !language.isEmpty() );
    configChanged();
}

void SelectTalkerDlg::slotTalkersListView_selectionChanged()
{
    QListViewItem* item = m_widget->talkersListView->selectedItem();
    if ( !item ) return;
    if (!m_widget->useSpecificTalkerRadioButton->isChecked()) return;
    configChanged();
}

void SelectTalkerDlg::configChanged()
{
    applyControlsToTalkerCode();
    applyTalkerCodeToControls();
    enableDisableControls();
}

void SelectTalkerDlg::applyTalkerCodeToControls()
{
    bool preferred = false;
    QString code = m_talkerCode.getTalkerCode();

    // TODO: Need to display translated Synth names.
    KttsUtils::setCbItemFromText(m_widget->synthComboBox,
        TalkerCode::stripPrefer( m_talkerCode.plugInName(), preferred) );
    m_widget->synthCheckBox->setEnabled( !m_talkerCode.plugInName().isEmpty() );
    m_widget->synthCheckBox->setChecked( preferred );

    KttsUtils::setCbItemFromText(m_widget->genderComboBox,
        TalkerCode::translatedGender( TalkerCode::stripPrefer( m_talkerCode.gender(), preferred ) ) );
    m_widget->genderCheckBox->setEnabled( !m_talkerCode.gender().isEmpty() );
    m_widget->genderCheckBox->setChecked( preferred );

    KttsUtils::setCbItemFromText(m_widget->volumeComboBox,
        TalkerCode::translatedVolume( TalkerCode::stripPrefer(  m_talkerCode.volume(), preferred ) ) );
    m_widget->volumeCheckBox->setEnabled( !m_talkerCode.volume().isEmpty() );
    m_widget->volumeCheckBox->setChecked( preferred );

    KttsUtils::setCbItemFromText(m_widget->rateComboBox,
        TalkerCode::translatedRate( TalkerCode::stripPrefer( m_talkerCode.rate(), preferred ) ) );
    m_widget->rateCheckBox->setEnabled( !m_talkerCode.rate().isEmpty() );
    m_widget->rateCheckBox->setChecked( preferred );

    // Select closest matching specific Talker.
    int talkerIndex = TalkerCode::findClosestMatchingTalker(m_talkers, m_talkerCode.getTalkerCode(), false);
    KListView* lv = m_widget->talkersListView;
    QListViewItem* item = lv->firstChild();
    if ( item )
    {
        while ( talkerIndex > 0 )
        {
            item = item->nextSibling();
            --talkerIndex;
        }
        lv->setSelected( item, true );
    }
}

void SelectTalkerDlg::applyControlsToTalkerCode()
{
    if ( m_widget->useDefaultRadioButton->isChecked() )
        m_talkerCode = TalkerCode(QString::null, false);
    else if ( m_widget->useClosestMatchRadioButton->isChecked() )
    {
        // Language already stored in talker code.

        QString t = m_widget->synthComboBox->currentText();
        if ( !t.isEmpty() && m_widget->synthCheckBox->isChecked() ) t.prepend("*");
        m_talkerCode.setPlugInName( t );

        t = TalkerCode::untranslatedGender( m_widget->genderComboBox->currentText() );
        if ( !t.isEmpty() && m_widget->genderCheckBox->isChecked() ) t.prepend("*");
        m_talkerCode.setGender( t );

        t = TalkerCode::untranslatedVolume( m_widget->volumeComboBox->currentText() );
        if ( !t.isEmpty() && m_widget->volumeCheckBox->isChecked() ) t.prepend("*");
        m_talkerCode.setVolume( t );

        t = TalkerCode::untranslatedRate( m_widget->rateComboBox->currentText() );
        if ( !t.isEmpty() && m_widget->rateCheckBox->isChecked() ) t.prepend("*");
        m_talkerCode.setRate( t );
    }
    else if (m_widget->useSpecificTalkerRadioButton->isChecked() )
    {
        QListViewItem* item = m_widget->talkersListView->selectedItem();
        if ( item )
        {
            int itemIndex = -1;
            while ( item )
            {
                item = item->itemAbove();
                itemIndex++;
            }
            m_talkerCode = TalkerCode( &(m_talkers[itemIndex]), false );
        }
    }
}

void SelectTalkerDlg::loadTalkers(bool /*runningTalkers*/)
{
    m_talkers.clear();
    KListView* lv = m_widget->talkersListView;
    lv->clear();
    QListViewItem* item;
    KConfig* config = new KConfig("kttsdrc");
    config->setGroup("General");
    QStringList talkerIDsList = config->readListEntry("TalkerIDs", ',');
    if (!talkerIDsList.isEmpty())
    {
        QStringList::ConstIterator itEnd(talkerIDsList.constEnd());
        for( QStringList::ConstIterator it = talkerIDsList.constBegin(); it != itEnd; ++it )
        {
            QString talkerID = *it;
            config->setGroup("Talker_" + talkerID);
            QString talkerCode = config->readEntry("TalkerCode", QString::null);
            // Parse and normalize the talker code.
            TalkerCode talker = TalkerCode(talkerCode, true);
            m_talkers.append(talker);
            QString desktopEntryName = config->readEntry("DesktopEntryName", QString::null);
            QString synthName = TalkerCode::TalkerDesktopEntryNameToName(desktopEntryName);
            // Display in List View using translated strings.
            item = new KListViewItem(lv, item);
            QString fullLanguageCode = talker.fullLanguageCode();
            QString language = TalkerCode::languageCodeToLanguage(fullLanguageCode);
            item->setText(tlvcLanguage, language);
            // Don't update the Synthesizer name with plugInName.  The former is a translated
            // name; the latter an English name.
            // if (!plugInName.isEmpty()) talkerItem->setText(tlvcSynthName, plugInName);
            if (!synthName.isEmpty())
                item->setText(tlvcSynthName, synthName);
            if (!talker.voice().isEmpty())
                item->setText(tlvcVoice, talker.voice());
            if (!talker.gender().isEmpty())
                item->setText(tlvcGender, TalkerCode::translatedGender(talker.gender()));
            if (!talker.volume().isEmpty())
                item->setText(tlvcVolume, TalkerCode::translatedVolume(talker.volume()));
            if (!talker.rate().isEmpty())
                item->setText(tlvcRate, TalkerCode::translatedRate(talker.rate()));
        }
    }
    delete config;
}

void SelectTalkerDlg::enableDisableControls()
{
    bool enableClosest = ( m_widget->useClosestMatchRadioButton->isChecked() );
    bool enableSpecific = ( m_widget->useSpecificTalkerRadioButton->isChecked() );
    m_widget->closestMatchGroupBox->setEnabled( enableClosest );
    m_widget->talkersListView->setEnabled( enableSpecific );
}
