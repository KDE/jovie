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

#include "selecttalkerdlg.h"

// Qt includes.
#include <QtGui/QCheckBox>
#include <QtGui/QRadioButton>
#include <QtGui/QTableWidget>
#include <QtGui/QHeaderView>

// KDE includes.
#include <kdialog.h>
#include <kcombobox.h>
#include <kpushbutton.h>
#include <klineedit.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kservicetypetrader.h>

// KTTS includes.
#include "utils.h"
#include "talkerlistmodel.h"
#include "selecttalkerdlg.moc"

SelectTalkerDlg::SelectTalkerDlg(
    QWidget* parent,
    const char* name,
    const QString& caption,
    const QString& talkerCode,
    bool runningTalkers) :

    KDialog(parent)
{
    Q_UNUSED(name);
    setCaption(caption);
    setButtons(KDialog::Ok|KDialog::Cancel);
    m_widget = new Ui::SelectTalkerWidget();
    QWidget* w = new QWidget();
    m_widget->setupUi( w );
    m_talkerListModel = new TalkerListModel();
    m_widget->talkersView->setModel(m_talkerListModel);

    setMainWidget( w );
    m_runningTalkers = runningTalkers;
    m_talkerCode = TalkerCode( talkerCode, false );

    // Fill combo boxes.
    //KComboBox* cb;
    //= m_widget->genderComboBox;
    //cb->addItem( QString() );
    //cb->addItem( TalkerCode::translatedGender("male") );
    //cb->addItem( TalkerCode::translatedGender("female") );
    //cb->addItem( TalkerCode::translatedGender("neutral") );

    //cb = m_widget->volumeComboBox;
    //cb->addItem( QString() );
    //cb->addItem( TalkerCode::translatedVolume("medium") );
    //cb->addItem( TalkerCode::translatedVolume("loud") );
    //cb->addItem( TalkerCode::translatedVolume("soft") );

    //cb = m_widget->rateComboBox;
    //cb->addItem( QString() );
    //cb->addItem( TalkerCode::translatedRate("medium") );
    //cb->addItem( TalkerCode::translatedRate("fast") );
    //cb->addItem( TalkerCode::translatedRate("slow") );

    //cb = m_widget->synthComboBox;
    //cb->addItem( QString() );
	//KService::List offers = KServiceTypeTrader::self()->query("KTTSD/SynthPlugin");
    //for(int i=0; i < offers.count() ; ++i)
    //    cb->addItem(offers[i]->name());

    // Fill List View with list of Talkers.
    KConfig config(QLatin1String( "kttsdrc" ));
    m_talkerListModel->loadTalkerCodesFromConfig(&config);

    // Set initial radio button state.
    if ( talkerCode.isEmpty() )
        m_widget->useDefaultRadioButton->setChecked(true);
    else
    {
        m_widget->useSpecificTalkerRadioButton->setChecked(true);
    }

    //applyTalkerCodeToControls();
    enableDisableControls();

    connect(m_widget->useDefaultRadioButton, SIGNAL(clicked()),
            this, SLOT(configChanged()));
    connect(m_widget->useSpecificTalkerRadioButton, SIGNAL(clicked()),
            this, SLOT(configChanged()));

    connect(m_widget->talkersView, SIGNAL(clicked(QModelIndex)),
            this, SLOT(slotTalkersView_clicked()));

    m_widget->talkersView->setMinimumHeight( 120 );
}

SelectTalkerDlg::~SelectTalkerDlg()
{
   delete m_widget;
}

QString SelectTalkerDlg::getSelectedTalkerCode()
{
    return m_talkerCode.getTalkerCode();
}

QString SelectTalkerDlg::getSelectedTranslatedDescription()
{
    return m_talkerCode.getTranslatedDescription();
}

void SelectTalkerDlg::slotTalkersView_clicked()
{
    QModelIndex modelIndex = m_widget->talkersView->currentIndex();
    if (!modelIndex.isValid()) return;
    if (!m_widget->useSpecificTalkerRadioButton->isChecked()) return;
    m_talkerCode = m_talkerListModel->getRow(modelIndex.row());
    configChanged();
}

void SelectTalkerDlg::configChanged()
{
    enableDisableControls();
}

void SelectTalkerDlg::enableDisableControls()
{
    bool enableSpecific = ( m_widget->useSpecificTalkerRadioButton->isChecked() );
    m_widget->talkersView->setEnabled( enableSpecific );
}
