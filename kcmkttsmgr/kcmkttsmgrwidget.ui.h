/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

// $Id$


void KCMKttsMgrWidget::slotAddLanguage(){
    kdDebug() << "Running: KCMKttsMgrWidget::slotAddLanguage()" << endl;
    emit addLanguage();
}

void KCMKttsMgrWidget::slotConfigChanged(){
    kdDebug() << "Running: KCMKttsMgrWidget:slotConfigChanged():" << endl;
    emit configChanged();
}

void KCMKttsMgrWidget::slotRemoveLanguage(){
    kdDebug() << "Running: KCMKttsMgrWidget::slotRemoveLanguage()" << endl;
    emit removeLanguage();
}

void KCMKttsMgrWidget::slotSetDefaultLanguage(){
    kdDebug() << "Running: KCMKttsMgrWidget::slotSetDefaultLanguage()" << endl;
    emit setDefaultLanguage();
}

void KCMKttsMgrWidget::slotUpdateRemoveButton(){
    kdDebug() << "Running: KCMKttsMgrWidget::slotUpdateRemoveButton()" << endl;
    emit updateRemoveButton();
}

void KCMKttsMgrWidget::slotUpdateDefaultButton(){
    kdDebug() << "Running: KCMKttsMgrWidget::slotUpdateDefaultButton()" << endl;
    emit updateDefaultButton();
}


void KCMKttsMgrWidget::textPreSndCheck_clicked()
{

}
