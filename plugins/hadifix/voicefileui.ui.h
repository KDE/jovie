/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#ifndef VOICEFILEUI_UI_H
#define VOICEFILEUI_UI_H

void VoiceFileWidget::genderButton_clicked()
{
    HadifixProc::VoiceGender gender;
    QString details;
    gender = HadifixProc::determineGender(mbrola, voiceFileURL->url(), &details);
    
    if (gender == HadifixProc::MaleGender) {
       maleOption->setChecked (true);
       femaleOption->setChecked (false);
    }
    else if (gender == HadifixProc::FemaleGender) {
       maleOption->setChecked (false);
       femaleOption->setChecked (true);
    }
    else if (gender == HadifixProc::NoGender) {
       KMessageBox::sorry (this,
                    i18n("The gender of the voice file %1 could not be detected.", voiceFileURL->url()),
                    i18n("Trying to Determine the Gender - Hadifix Plug In"));
    }
    else {
       KMessageBox::detailedSorry (this,
                    i18n("The file %1 does not seem to be a voice file.", voiceFileURL->url()),
                    details, i18n("Trying to Determine the Gender - Hadifix Plug In"));
    }
}

#endif // VOICEFILEUI_UI_H
