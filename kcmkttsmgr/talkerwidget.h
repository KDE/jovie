/***************************************************** vim:set ts=4 sw=4 sts=4:
  Widget to configure Talker parameters including language, synthesizer, volume,
  rate, and pitch. Uses talkerwidget.ui.

  -------------------
  Copyright 2010 by Jeremy Whiting <jpwhiting@kde.org>
  -------------------

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

#ifndef _TALKERWIDGET_H
#define _TALKERWIDGET_H

// Qt includes.
#include <QtCore/QList>

// KDE includes
#include <KDialog>

#include "../libkttsd/talkercode.h"

namespace Ui
{
    class TalkerWidget;
}

class TalkerWidget : public QWidget
{
    Q_OBJECT

public:
    /**
    * Constructor.
    * @param parent             Inherited KDialog parameter.
    */
    explicit TalkerWidget(QWidget* parent = 0);

    /**
    * Destructor.
    */
    ~TalkerWidget();

    /**
     * Set the talker's name
     * @param name              Name to set
     */
    void setName(const QString &name);

    /**
     * Get the talker's name
     * @returns                 Talker's name
     */
    QString getName() const;

    /**
     * Set whether the name should be read-only (to the user)
     * @param value             True if the user should not be able to edit the name
     *                          False otherwise
     */
    void setNameReadOnly(bool value);

    /**
    * Set the talker configuration to start with
    * @param talker             Talker configuration to initialize to
    */
    void setTalkerCode(const TalkerCode & talker);

    /**
    * Returns user's chosen talker configuration
    */
    TalkerCode getTalkerCode() const;


Q_SIGNALS:
    void talkerChanged();

private:
    // output modules found in speech-dispatcher
    QStringList m_outputModules;

    // designer ui content
    Ui::TalkerWidget * mUi;

};

#endif

