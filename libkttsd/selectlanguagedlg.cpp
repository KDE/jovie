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

#include "selectlanguagedlg.h"

// Qt includes.
#include <QtGui/QTableWidget>
#include <QtGui/QHeaderView>

// KDE includes.
#include <kdialog.h>
#include <kdebug.h>
#include <kglobal.h>

// KTTS includes.
#include "talkercode.h"
#include "selectlanguagedlg.moc"

SelectLanguageDlg::SelectLanguageDlg(
    QWidget* parent,
    const QString& caption,
    const QStringList& languageCodes,
    bool selectMode,
    bool blankMode) :

    KDialog(parent)
{
    setCaption(caption);
    setButtons(KDialog::Help|KDialog::Ok|KDialog::Cancel);
    // Create a QTableWidget and fill with all known languages.
    m_langList = new QTableWidget( this );
    m_langList->setColumnCount(2);
    m_langList->verticalHeader()->hide();
    m_langList->setHorizontalHeaderItem(0, new QTableWidgetItem(i18n("Language")));
    m_langList->setHorizontalHeaderItem(1, new QTableWidgetItem(i18n("Code")));
    m_langList->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    m_langList->setSelectionBehavior(QAbstractItemView::SelectRows);
    if (selectMode == MultipleSelect)
        m_langList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    else
        m_langList->setSelectionMode(QAbstractItemView::SingleSelection);
    QStringList allLocales = KGlobal::locale()->allLanguagesList();
    QString locale;
    QString language;
    const int allLocalesCount = allLocales.count();
    for (int ndx=0; ndx < allLocalesCount; ++ndx)
    {
        locale = allLocales[ndx];
        language = locale;
        if (!language.isEmpty()) {
            int row = m_langList->rowCount();
            m_langList->setRowCount(row + 1);
            m_langList->setItem(row, 0, new QTableWidgetItem(language));
            m_langList->setItem(row, 1, new QTableWidgetItem(locale));
            if (languageCodes.contains(locale)) m_langList->selectRow(row);
        }
    }
    // Sort by language.
    m_langList->sortItems(0);
    // If blank is allowed, add to top of the list.
    if (blankMode == BlankAllowed) {
        m_langList->insertRow(0);
        m_langList->setItem(0, 0, new QTableWidgetItem(""));
        m_langList->setItem(0, 1, new QTableWidgetItem(""));
    }
    setMainWidget(m_langList);
    setHelp("select-language", "kttsd");
    QSize size = m_langList->minimumSize();
    size.setHeight(500);
    m_langList->setMinimumSize(size);
}

QString SelectLanguageDlg::selectedLanguage()
{
    return firstSelectedItem(0);
}

QString SelectLanguageDlg::selectedLanguageCode()
{
    return firstSelectedItem(1);
}

QStringList SelectLanguageDlg::selectedLanguages()
{
    return allSelectedItems(0);
}

QStringList SelectLanguageDlg::selectedLanguageCodes()
{
    return allSelectedItems(1);
}

QString SelectLanguageDlg::firstSelectedItem(int col)
{
    QString s;
    for (int row = 0; row < m_langList->rowCount(); ++row) {
        if (m_langList->isItemSelected(m_langList->item(row, col)))
            return m_langList->item(row, col)->text();
    }
    return QString();
}

QStringList SelectLanguageDlg::allSelectedItems(int col)
{
    QStringList sl;
    for (int row = 0; row < m_langList->rowCount(); ++row) {
        if (m_langList->isItemSelected(m_langList->item(row, col)))
            sl.append(m_langList->item(row, col)->text());
    }
    return sl;
}

