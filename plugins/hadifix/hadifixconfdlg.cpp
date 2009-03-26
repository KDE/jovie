/*
  This file is part of the KDE project

  Copyright (C) 2009 Laurent Montel <montel@kde.org>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; see the file COPYING.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/
#include "hadifixconfdlg.h"
#include <kmessagebox.h>
#include "hadifixproc.h"
#include "ui_voicefileui.h"

HadifixConfDialog::HadifixConfDialog( QWidget *parent, bool maleVoice,const QString& filename )
    : KDialog( parent )
{
    setCaption(i18n("Voice File - Hadifix Plugin"));
    setButtons(KDialog::Ok|KDialog::Cancel);
    setDefaultButton(KDialog::Cancel);

    QWidget *w = new QWidget(this);
    voicefile.setupUi(w);
    setMainWidget(w);

    voicefile.femaleOption->setChecked(!maleVoice);
    voicefile.maleOption->setChecked(maleVoice);
    voicefile.voiceFileURL->setUrl(KUrl::fromPath(filename));
    connect( voicefile.genderButton, SIGNAL( clicked() ), this, SLOT( genderButton_clicked() ) );
}

void HadifixConfDialog::genderButton_clicked()
{
    HadifixProc::VoiceGender gender;
    QString details;
    //TODO fix me
    QString url;
    gender = HadifixProc::determineGender(/*mbrola*/url, voicefile.voiceFileURL->url().path(), &details);

    if (gender == HadifixProc::MaleGender) {
       voicefile.maleOption->setChecked (true);
       voicefile.femaleOption->setChecked (false);
    }
    else if (gender == HadifixProc::FemaleGender) {
       voicefile.maleOption->setChecked (false);
       voicefile.femaleOption->setChecked (true);
    }
    else if (gender == HadifixProc::NoGender) {
       KMessageBox::sorry (this,
                    i18n("The gender of the voice file %1 could not be detected.", voicefile.voiceFileURL->url().path()),
                    i18n("Trying to Determine the Gender - Hadifix Plug In"));
    }
    else {
       KMessageBox::detailedSorry (this,
                    i18n("The file %1 does not seem to be a voice file.", voicefile.voiceFileURL->url().path()),
                    details, i18n("Trying to Determine the Gender - Hadifix Plug In"));
    }
}

QString HadifixConfDialog::path() const
{
    return voicefile.voiceFileURL->url().path();
}

bool HadifixConfDialog::voice() const
{
    return voicefile.maleOption->isChecked();
}
