/***************************************************** vim:set ts=4 sw=4 sts=4:
  Configuration widget and functions for Epos plug in
  -------------------
  Copyright : (C) 2004 Gary Cramblitt
  -------------------
  Original author: Gary Cramblitt <garycramblitt@comcast.net>
  Current Maintainer: Gary Cramblitt <garycramblitt@comcast.net>
 ******************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 ***************************************************************************/

// C++ includes.
#include <math.h>

// Qt includes.
#include <qfile.h>
#include <qapplication.h>
#include <qtextcodec.h>
#include <qlayout.h>
#include <qslider.h>

// KDE includes.
#include <kdialog.h>
#include <kartsserver.h>
#include <kartsdispatcher.h>
#include <kplayobject.h>
#include <kplayobjectfactory.h>
#include <ktempfile.h>
#include <kstandarddirs.h>
#include <kcombobox.h>
#include <klocale.h>
#include <knuminput.h>

// Epos Plugin includes.
#include "eposproc.h"
#include "eposconf.h"
#include "eposconf.moc"

/** Constructor */
EposConf::EposConf( QWidget* parent, const char* name, const QStringList& /*args*/) :
    PlugInConf(parent, name)
{
    // kdDebug() << "EposConf::EposConf: Running" << endl;
    m_eposProc = 0;
    m_artsServer = 0;
    m_playObj = 0;
    m_progressDlg = 0;

    QVBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(),
        KDialog::spacingHint(), "EposConfigWidgetLayout");
    layout->setAlignment (Qt::AlignTop);
    m_widget = new EposConfWidget(this, "EposConfigWidget");
    layout->addWidget(m_widget);

    defaults();

    connect(m_widget->eposServerPath, SIGNAL(textChanged(const QString&)),
        this, SLOT(configChanged()));
    connect(m_widget->eposClientPath, SIGNAL(textChanged(const QString&)),
            this, SLOT(configChanged()));
    connect(m_widget->timeBox, SIGNAL(valueChanged(int)),
            this, SLOT(timeBox_valueChanged(int)));
    connect(m_widget->frequencyBox, SIGNAL(valueChanged(int)),
            this, SLOT(frequencyBox_valueChanged(int)));
    connect(m_widget->timeSlider, SIGNAL(valueChanged(int)),
            this, SLOT(timeSlider_valueChanged(int)));
    connect(m_widget->frequencySlider, SIGNAL(valueChanged(int)),
            this, SLOT(frequencySlider_valueChanged(int)));
    connect(m_widget->timeBox, SIGNAL(valueChanged(int)), this, SLOT(configChanged()));
    connect(m_widget->timeSlider, SIGNAL(valueChanged(int)), this, SLOT(configChanged()));
    connect(m_widget->frequencyBox, SIGNAL(valueChanged(int)), this, SLOT(configChanged()));
    connect(m_widget->frequencySlider, SIGNAL(valueChanged(int)), this, SLOT(configChanged()));
    connect(m_widget->characterCodingBox, SIGNAL(activated(const QString&)),
        this, SLOT(configChanged()));
    connect(m_widget->eposServerOptions, SIGNAL(textChanged(const QString&)),
            this, SLOT(configChanged()));
    connect(m_widget->eposClientOptions, SIGNAL(textChanged(const QString&)),
            this, SLOT(configChanged()));
    connect(m_widget->eposTest, SIGNAL(clicked()),
        this, SLOT(slotEposTest_clicked()));
}

/** Destructor */
EposConf::~EposConf(){
    // kdDebug() << "Running: EposConf::~EposConf()" << endl;
    if (m_playObj) m_playObj->halt();
    delete m_playObj;
    delete m_artsServer;
    if (!m_waveFile.isNull()) QFile::remove(m_waveFile);
    delete m_eposProc;
    delete m_progressDlg;
}

void EposConf::load(KConfig *config, const QString &configGroup){
    // kdDebug() << "EposConf::load: Running " << endl;

    config->setGroup(configGroup);
    m_widget->eposServerPath->setURL(config->readPathEntry("EposServerExePath", "epos"));
    m_widget->eposClientPath->setURL(config->readPathEntry("EposClientExePath", "say"));
    m_widget->eposServerOptions->setText(config->readEntry("EposServerOptions", ""));
    m_widget->eposClientOptions->setText(config->readEntry("EposClientOptions", ""));
    QString codecString = config->readEntry("Codec", "Local");
    int codec;
    if (codecString == "Local")
        codec = EposProc::Local;
    else if (codecString == "Latin1")
        codec = EposProc::Latin1;
    else if (codecString == "Unicode")
        codec = EposProc::Unicode;
    else {
        codec = EposProc::Local;
        for (int i = EposProc::UseCodec; i < m_widget->characterCodingBox->count(); i++ )
            if (codecString == m_widget->characterCodingBox->text(i))
                codec = i;
    }
    m_widget->timeBox->setValue(config->readNumEntry("time", 100));
    m_widget->frequencyBox->setValue(config->readNumEntry("pitch", 100));
    m_widget->characterCodingBox->setCurrentItem(codec);
}

/**
* Converts a language code into the language setting passed to Epos synth.
*/
QString EposConf::languageCodeToEposLanguage(const QString &languageCode)
{
    QString eposLanguage;
    if (languageCode.left(2) == "cs") eposLanguage = "czech";
    if (languageCode.left(2) == "sk") eposLanguage = "slovak";
    return eposLanguage;
}

void EposConf::save(KConfig *config, const QString &configGroup){
    // kdDebug() << "EposConf::save: Running" << endl;

    config->setGroup("Epos");
    config->writePathEntry("EposServerExePath",
        realFilePath(m_widget->eposServerPath->url()));
    config->writePathEntry("EposClientExePath", 
        realFilePath(m_widget->eposClientPath->url()));
    config->writeEntry("Language", languageCodeToEposLanguage(m_languageCode));
    config->setGroup(configGroup);
    config->writePathEntry("EposServerExePath", 
        realFilePath(m_widget->eposServerPath->url()));
    config->writePathEntry("EposClientExePath", 
        realFilePath(m_widget->eposClientPath->url()));
    config->writeEntry("EposServerOptions", m_widget->eposServerOptions->text());
    config->writeEntry("EposClientOptions", m_widget->eposClientOptions->text());
    config->writeEntry("time", m_widget->timeBox->value());
    config->writeEntry("pitch", m_widget->frequencyBox->value());
    int codec = m_widget->characterCodingBox->currentItem();
    if (codec == EposProc::Local)
        config->writeEntry("Codec", "Local");
    else if (codec == EposProc::Latin1)
        config->writeEntry("Codec", "Latin1");
    else if (codec == EposProc::Unicode)
        config->writeEntry("Codec", "Unicode");
    else config->writeEntry("Codec",
        m_widget->characterCodingBox->text(codec));
}

void EposConf::defaults(){
    // kdDebug() << "EposConf::defaults: Running" << endl;
    m_widget->eposServerPath->setURL("epos");
    m_widget->eposClientPath->setURL("say");
    m_widget->eposServerOptions->setText("");
    m_widget->eposClientOptions->setText("");
    m_widget->timeBox->setValue(100);
    timeBox_valueChanged(100);
    m_widget->frequencyBox->setValue(100);
    frequencyBox_valueChanged(100);
    buildCodecList();
    m_widget->characterCodingBox->setCurrentItem(0);
}

void EposConf::setDesiredLanguage(const QString &lang)
{
    m_languageCode = lang;
}

QString EposConf::getTalkerCode()
{
    QString eposServerExe = realFilePath(m_widget->eposServerPath->url());
    QString eposClientExe = realFilePath(m_widget->eposClientPath->url());
    if (!eposServerExe.isEmpty() && !eposClientExe.isEmpty())
    {
        if (!getLocation(eposServerExe).isEmpty() && !getLocation(eposClientExe).isEmpty())
        {
            QString rate = "medium";
            if (m_widget->timeBox->value() < 75) rate = "slow";
            if (m_widget->timeBox->value() > 125) rate = "fast";
            return QString(
                    "<voice lang=\"%1\" name=\"%2\" gender=\"%3\" />"
                    "<prosody volume=\"%4\" rate=\"%5\" />"
                    "<kttsd synthesizer=\"%6\" />")
                    .arg(m_languageCode)
                    .arg("fixed")
                    .arg("neutral")
                    .arg("medium")
                    .arg(rate)
                    .arg("Epos TTS Synthesis System");
        }
    }
    return QString::null;
}

void EposConf::buildCodecList () {
   QString local = i18n("Local")+" (";
   local += QTextCodec::codecForLocale()->name();
   local += ")";
   m_widget->characterCodingBox->clear();
   m_widget->characterCodingBox->insertItem (local, EposProc::Local);
   m_widget->characterCodingBox->insertItem (i18n("Latin1"), EposProc::Latin1);
   m_widget->characterCodingBox->insertItem (i18n("Unicode"), EposProc::Unicode);
   for (int i = 0; (QTextCodec::codecForIndex(i)); i++ )
      m_widget->characterCodingBox->insertItem(QTextCodec::codecForIndex(i)->name(),
        EposProc::UseCodec + i);
}

void EposConf::slotEposTest_clicked()
{
    // kdDebug() << "EposConf::slotEposTest_clicked(): Running" << endl;
    // If currently synthesizing, stop it.
    if (m_eposProc)
        m_eposProc->stopText();
    else
    {
        m_eposProc = new EposProc();
        connect (m_eposProc, SIGNAL(stopped()), this, SLOT(slotSynthStopped()));
    }
    // Create a temp file name for the wave file.
    KTempFile tempFile (locateLocal("tmp", "eposplugin-"), ".wav");
    QString tmpWaveFile = tempFile.file()->name();
    tempFile.close();

    // Tell user to wait.
    m_progressDlg = new KProgressDialog(m_widget, "kttsmgr_epos_testdlg",
        i18n("Testing"),
        i18n("Testing."),
        true);
    m_progressDlg->progressBar()->hide();
    m_progressDlg->setAllowCancel(true);

    // Play an English test.
    // TODO: Need czeck or slavak test message.
    // TODO: Whenever server options change, the server must be restarted.
    connect (m_eposProc, SIGNAL(synthFinished()), this, SLOT(slotSynthFinished()));
    m_eposProc->synth(
        "K D E is a modern graphical desktop for Unix computers.",
        tmpWaveFile,
        realFilePath(m_widget->eposServerPath->url()),
        realFilePath(m_widget->eposClientPath->url()),
        m_widget->eposServerOptions->text(),
        m_widget->eposClientOptions->text(),
        m_widget->characterCodingBox->currentItem(),
        QTextCodec::codecForName(m_widget->characterCodingBox->text(m_widget->characterCodingBox->currentItem())),
        languageCodeToEposLanguage(m_languageCode),
        m_widget->timeBox->value(),
        m_widget->frequencyBox->value()
        );

    // Display progress dialog modally.  Processing continues when plugin signals synthFinished,
    // or if user clicks Cancel button.
    m_progressDlg->exec();
    disconnect (m_eposProc, SIGNAL(synthFinished()), this, SLOT(slotSynthFinished()));
    if (m_progressDlg->wasCancelled()) m_eposProc->stopText();
    delete m_progressDlg;
    m_progressDlg = 0;
}

void EposConf::slotSynthFinished()
{
    // If user canceled, progress dialog is gone, so exit.
    if (!m_progressDlg)
    {
        m_eposProc->ackFinished();
        return;
    }
    // Hide the Cancel button so user can't cancel in the middle of playback.
    m_progressDlg->showCancelButton(false);
    // If currently playing (or finished playing), stop and delete play object.
    if (m_playObj)
    {
       m_playObj->halt();
       // Clean up.
       QFile::remove(m_waveFile);
    }
    delete m_playObj;
    delete m_artsServer;
    // Get new wavefile name.
    m_waveFile = m_eposProc->getFilename();
    m_eposProc->ackFinished();
    // Start playback of the wave file.
    KArtsDispatcher dispatcher;
    m_artsServer = new KArtsServer;
    KDE::PlayObjectFactory factory (m_artsServer->server());
    m_playObj = factory.createPlayObject (m_waveFile, true);
    m_playObj->play();

    // TODO: The following hunk of code would ideally be unnecessary.  We would just
    // return at this point and let EposConf destructor take care of
    // cleaning up the play object.  However, because we've been called from DCOP,
    // this seems to be necessary.  The call to processEvents is problematic because
    // it can cause re-entrancy.
    while (m_playObj->state() == Arts::posPlaying) qApp->processEvents();
    m_playObj->halt();
    delete m_playObj;
    m_playObj = 0;
    delete m_artsServer;
    m_artsServer = 0;
    QFile::remove(m_waveFile);
    m_waveFile = QString::null;
    if (m_progressDlg) m_progressDlg->close();
}

void EposConf::slotSynthStopped()
{
    // Clean up after canceling test.
    QString filename = m_eposProc->getFilename();
    if (!filename.isNull()) QFile::remove(filename);
}

// Basically the slider values are logarithmic (0,...,1000) whereas percent
// values are linear (50%,...,200%).
//
// slider = alpha * (log(percent)-log(50))
// with alpha = 1000/(log(200)-log(50))

int EposConf::percentToSlider(int percentValue) {
    double alpha = 1000 / (log(200) - log(50));
    return (int)floor (0.5 + alpha * (log(percentValue)-log(50)));
}

int EposConf::sliderToPercent(int sliderValue) {
    double alpha = 1000 / (log(200) - log(50));
    return (int)floor(0.5 + exp (sliderValue/alpha + log(50)));
}

void EposConf::timeBox_valueChanged(int percentValue) {
    m_widget->timeSlider->setValue (percentToSlider (percentValue));
}

void EposConf::frequencyBox_valueChanged(int percentValue) {
    m_widget->frequencySlider->setValue(percentToSlider(percentValue));
}

void EposConf::timeSlider_valueChanged(int sliderValue) {
    m_widget->timeBox->setValue (sliderToPercent (sliderValue));
}

void EposConf::frequencySlider_valueChanged(int sliderValue) {
    m_widget->frequencyBox->setValue(sliderToPercent(sliderValue));
}
