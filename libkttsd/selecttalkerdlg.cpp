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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ******************************************************************************/

// Qt includes.
#include <QCheckBox>
#include <QRadioButton>

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
#include "talkerlistmodel.h"
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
    m_widget = new Ui::SelectTalkerWidget();
    QWidget* w = new QWidget();
    m_widget->setupUi( w );
    m_talkerListModel = new TalkerListModel();
    m_widget->talkersView->setModel(m_talkerListModel);

    // TODO: How do I do this in a general way and still get KDialogBase to properly resize?
    //w->setMinimumSize( QSize(700,500) );
    setMainWidget( w );
    m_runningTalkers = runningTalkers;
    m_talkerCode = TalkerCode( talkerCode, false );

    // Fill combo boxes.
    KComboBox* cb = m_widget->genderComboBox;
    cb->insertItem( QString() );
    cb->insertItem( TalkerCode::translatedGender("male") );
    cb->insertItem( TalkerCode::translatedGender("female") );
    cb->insertItem( TalkerCode::translatedGender("neutral") );

    cb = m_widget->volumeComboBox;
    cb->insertItem( QString() );
    cb->insertItem( TalkerCode::translatedVolume("medium") );
    cb->insertItem( TalkerCode::translatedVolume("loud") );
    cb->insertItem( TalkerCode::translatedVolume("soft") );

    cb = m_widget->rateComboBox;
    cb->insertItem( QString() );
    cb->insertItem( TalkerCode::translatedRate("medium") );
    cb->insertItem( TalkerCode::translatedRate("fast") );
    cb->insertItem( TalkerCode::translatedRate("slow") );

    cb = m_widget->synthComboBox;
    cb->insertItem( QString() );
    KTrader::OfferList offers = KTrader::self()->query("KTTSD/SynthPlugin");
    for(int i=0; i < offers.count() ; ++i)
        cb->insertItem(offers[i]->name());

    // Fill List View with list of Talkers.
    KConfig config("kttsdrc");
    m_talkerListModel->loadTalkerCodesFromConfig(&config);

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

    connect(m_widget->talkersView, SIGNAL(clicked()),
            this, SLOT(slotTalkersView_clicked()));

    m_widget->talkersView->setMinimumHeight( 120 );
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
    QWidget* hBox = new QWidget;
    hBox->setObjectName("SelectLanguage_hbox");
    QHBoxLayout* hBoxLayout = new QHBoxLayout;
    hBoxLayout->setMargin(0);
    // Create a KListView and fill with all known languages.
    KListView* langLView = new KListView(hBox);
    langLView->addColumn(i18n("Language"));
    langLView->addColumn(i18n("Code"));
    langLView->setSelectionMode(Q3ListView::Single);
    QStringList allLocales = KGlobal::locale()->allLanguagesTwoAlpha();
    QString locale;
    QString language;
    // Blank line so user can select no language.
    // Note: Don't use QString(), which gets displayed at bottom of list, rather than top.
    Q3ListViewItem* item = new KListViewItem(langLView, "", "");
    if (m_talkerCode.languageCode().isEmpty()) item->setSelected(true);
    int allLocalesCount = allLocales.count();
    for (int ndx=0; ndx < allLocalesCount; ++ndx)
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
        this,
        "SelectLanguage_dlg",
        true,
        true);
    hBoxLayout->addWidget(langLView);
    hBox->setLayout(hBoxLayout);
    dlg->setMainWidget(hBox);
    dlg->setHelp("", "kttsd");
    dlg->setInitialSize(QSize(300, 500), false);
    // TODO: This isn't working.  Furthermore, item appears selected but is not.
    langLView->ensureItemVisible(langLView->selectedItem());
    int dlgResult = dlg->exec();
    language.clear();
    if (dlgResult == QDialog::Accepted)
    {
        if (langLView->selectedItem())
        {
            language = langLView->selectedItem()->text(0);
            m_talkerCode.setFullLanguageCode( langLView->selectedItem()->text(1) );
        }
    }
    delete dlg;
    m_widget->languageLabel->setText(language);
    m_widget->languageCheckBox->setChecked( !language.isEmpty() );
    configChanged();
}

void SelectTalkerDlg::slotTalkersView_clicked()
{
    QModelIndex modelIndex = m_widget->talkersView->currentIndex();
    if (!modelIndex.isValid()) return;
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
    const TalkerCode::TalkerCodeList talkers = m_talkerListModel->datastore();
    int talkerIndex = TalkerCode::findClosestMatchingTalker(talkers, m_talkerCode.getTalkerCode(), false);
    m_widget->talkersView->setCurrentIndex(m_talkerListModel->index(talkerIndex, 0));
}

void SelectTalkerDlg::applyControlsToTalkerCode()
{
    if ( m_widget->useDefaultRadioButton->isChecked() )
        m_talkerCode = TalkerCode(QString(), false);
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
        QModelIndex talkerIndex = m_widget->talkersView->currentIndex();
        if (talkerIndex.isValid())
            m_talkerCode = m_talkerListModel->getRow(talkerIndex.row());
    }
}

void SelectTalkerDlg::enableDisableControls()
{
    bool enableClosest = ( m_widget->useClosestMatchRadioButton->isChecked() );
    bool enableSpecific = ( m_widget->useSpecificTalkerRadioButton->isChecked() );
    m_widget->closestMatchGroupBox->setEnabled( enableClosest );
    m_widget->talkersView->setEnabled( enableSpecific );
}
