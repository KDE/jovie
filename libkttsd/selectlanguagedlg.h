/***************************************************** vim:set ts=4 sw=4 sts=4:
  Description: 
     A dialog for user to select one or more languages from the list
     of KDE global languages.

  Copyright:
  (C) 2006 by Gary Cramblitt <garycramblitt@comcast.net>
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

#ifndef _SELECTLANGUAGEDLG_H_
#define _SELECTLANGUAGEDLG_H_

// Qt includes.
#include <QtGui/QWidget>

// KDE includes.
#include <kdialog.h>
#include <klocale.h>

class QTableWidget;

class KDE_EXPORT SelectLanguageDlg : public KDialog
{
    Q_OBJECT

    public:
        enum SelectMode {
            MultipleSelect  = 0,
            SingleSelect    = 1
        };
        enum BlankMode {
            BlankNotAllowed = 0,
            BlankAllowed    = 1
        };
        
        /**
         * Constructor.
         * @param parent                The parent for this dialog.
         * @param caption               Displayed title for this dialog.
         * @param languageCode          A list of language codes that should start
         *                              out selected when dialog is shown.
         * @param selectMode            0 if user may choose more than one
         *                              language in the list.  1 if only one
         *                              language may be chosen.
         * @param blankMode             If 1, a blank row is displayed in the
         *                              list and user may choose it.
         */
        SelectLanguageDlg(
            QWidget* parent = 0,
            const QString& caption = i18n("Select Language"),
            const QStringList& languageCodes = QStringList(),
            bool selectMode = SingleSelect,
            bool blankMode = BlankAllowed);

        /**
         * Destructor.
         */
        ~SelectLanguageDlg() { }
        
        /**
         * In single select mode, returns the language user chose.
         */
        QString selectedLanguage();
         
        /**
         * In single select mode, returns the language code user chose.
         */
        QString selectedLanguageCode();
         
        /**
         * In multiple select mode, returns list of languages user chose.
         */
        QStringList selectedLanguages();
         
        /**
         * In multiple select mode, returns the list of language codes
         * user chose.
         */
        QStringList selectedLanguageCodes();
    
    private:
        QString firstSelectedItem(int col);
        QStringList allSelectedItems(int col);
    
        QTableWidget* m_langList;
};

#endif                      // _SELECTLANGUAGEDLG_H_
