/***************************************************** vim:set ts=4 sw=4 sts=4:
  Dialog to allow user to add a new Talker by selecting a language and synthesizer
  (button).  Uses addtalkerwidget.ui.
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

#ifndef ADDTALKER_H
#define ADDTALKER_H

// Qt includes.
#include <QtCore/QList>

// KDE includes
#include <KDialog>

#include "../libkttsd/talkercode.h"

namespace Ui
{
    class AddTalkerWidget;
}

class AddTalker : public KDialog
{
    Q_OBJECT

public:
    /**
    * Constructor.
    * @param parent             Inherited KDialog parameter.
    */
    explicit AddTalker(QWidget* parent = 0);

    /**
    * Destructor.
    */
    ~AddTalker();

    /**
    * Set the talker configuration to start with
    * @param talker             Talker configuration to initialize to
    */
    void setTalkerCode(const TalkerCode & talker);

    /**
    * Returns user's chosen talker configuration
    */
    TalkerCode getTalkerCode() const;

private:
    // output modules found in speech-dispatcher
    QStringList m_outputModules;

    // designer ui content
    Ui::AddTalkerWidget * mUi;
private slots:
    void slot_tableSelectionChanged();
};

#endif

