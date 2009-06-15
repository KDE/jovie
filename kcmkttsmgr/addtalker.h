/***************************************************** vim:set ts=4 sw=4 sts=4:
  Dialog to allow user to add a new Talker by selecting a language and synthesizer
  (button).  Uses addtalkerwidget.ui.
  -------------------
  Copyright:
  (C) 2004 by Gary Cramblitt <garycramblitt@comcast.net>
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

#include "ui_addtalkerwidget.h"

class AddTalker : public QWidget, private Ui::AddTalkerWidget
{
    Q_OBJECT

public:
    /**
    * Constructor.
    * @param parent             Inherited KDialog parameter.
    * @param name               Inherited KDialog parameter.
    */
    explicit AddTalker(QWidget* parent = 0);

    /**
    * Destructor.
    */
    ~AddTalker();

    /**
    * Returns user's chosen language code.
    */
    QString getLanguageCode() const;

    /**
    * Returns user's chosen synthesizer.
    */
    QString getSynthesizer() const;

private:
    // Converts a language code plus optional country code to language description.
    QString languageCodeToLanguage(const QString &languageCode);

    // QMap of language descriptions to language codes.
    QMap<QString,QString> m_languageToLanguageCodeMap;

    // output modules found in speech-dispatcher
    QStringList m_outputModules;

    // map of output module to language list supported by each output module
    QMap<QString, QStringList> m_synthsToLanguagesMap;

private slots:
};

#endif

