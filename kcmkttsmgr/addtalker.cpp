/***************************************************** vim:set ts=4 sw=4 sts=4:
  Dialog to allow user to add a new Talker by selecting a language, synthesizer,
  and voice/pitch/speed options also.
  Uses addtalkerwidget.ui.
  -------------------
  Copyright 2004 by Gary Cramblitt <garycramblitt@comcast.net>
  Copyright 2009 by Jeremy Whiting <jpwhiting@kde.org>
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
#include "talkerwidget.h"

// Qt includes.
#include <QtGui/QDialog>

// KDE includes.
#include <klocale.h>

AddTalker::AddTalker(QWidget* parent)
    : KDialog(parent)
{
    this->setCaption(i18n("Add Talker"));
    this->setButtons(KDialog::Help|KDialog::Ok|KDialog::Cancel);
    this->setDefaultButton(KDialog::Ok);
    this->enableButtonOk(false);
    this->setHelp(QLatin1String( "select-plugin" ), QLatin1String( "jovie" ));

    mWidget = new TalkerWidget(this);
    connect(mWidget, SIGNAL(talkerChanged()), this, SLOT(slotTalkerChanged()));
    this->setMainWidget(mWidget);
}

AddTalker::~AddTalker()
{
    delete mWidget;
}

void AddTalker::setTalkerCode(const TalkerCode &talker)
{
    mWidget->setTalkerCode(talker);
}

TalkerCode AddTalker::getTalkerCode() const
{
    return mWidget->getTalkerCode();
}

void AddTalker::slotTalkerChanged()
{
    this->enableButtonOk(!mWidget->getName().isEmpty());
}

#include "addtalker.moc"

