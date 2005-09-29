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

#ifndef _SELECTTALKERDLG_H_
#define _SELECTTALKERDLG_H_

// Qt includes.
#include <qvaluelist.h>

// KDE includes.
#include <kdialogbase.h>
#include <klocale.h>
#include <kdemacros.h>

// KTTS includes.
#include "talkercode.h"
#include "selecttalkerwidget.h"

class KDE_EXPORT SelectTalkerDlg : public KDialogBase
{
    Q_OBJECT

    public:
        /**
         * Constructor.
         * @param parent                The parent for this dialog.
         * @param name                  Name for this dialog.
         * @param caption               Displayed title for this dialog.
         * @param talkerCode            A suggested starting Talker Code.
         * @param runningTalkers        If true, lists configured and Applied Talkers in the running
         *                              KTTSD.  If false, lists Talkers in the KTTSMgr Talker tab
         *                              (which may not yet have been Applied).
         */
        SelectTalkerDlg(
            QWidget* parent = 0,
            const char* name = "selecttalkerdialog",
            const QString& caption = i18n("Select Talker"),
            const QString& talkerCode = QString::null,
            bool runningTalkers = false);

        /**
         * Destructor.
         */
        ~SelectTalkerDlg();

        /**
         * Returns the Talker Code user chose.  QString::null if default Talker chosen.
         * Note that if user did not choose a specific Talker, this will be a partial Talker Code.
         */
        QString getSelectedTalkerCode();
        /**
         * Returns the Talker user chose in a translated displayable format.
         */
        QString getSelectedTranslatedDescription();

    private slots:
        void slotLanguageBrowseButton_clicked();
        void slotTalkersListView_selectionChanged();
        void configChanged();

    private:
        enum TalkerListViewColumn
        {
            tlvcLanguage,
            tlvcSynthName,
            tlvcVoice,
            tlvcGender,
            tlvcVolume,
            tlvcRate
        };

        void applyTalkerCodeToControls();
        void applyControlsToTalkerCode();
        void loadTalkers(bool runningTalkers);
        void enableDisableControls();

        // Main dialog widget.
        SelectTalkerWidget* m_widget;
        // True if list of Talkers should be taken from config file.
        bool m_runningTalkers;
        // Current Talker Code.
        TalkerCode m_talkerCode;
        // List of parsed talker codes for the configured Talkers.
        TalkerCode::TalkerCodeList m_talkers;
};

#endif                      // _SELECTTALKERDLG_H_
