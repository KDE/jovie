/***************************************************** vim:set ts=4 sw=4 sts=4:
  Sentence Boundary Detection Filter Configuration class.
  -------------------
  Copyright:
  (C) 2005 by Gary Cramblitt <garycramblitt@comcast.net>
  -------------------
  Original author: Gary Cramblitt <garycramblitt@comcast.net>
 ******************************************************************************/

/******************************************************************************
 *                                                                            *
 *    This program is free software; you can redistribute it and/or modify    *
 *    it under the terms of the GNU General Public License as published by    *
 *    the Free Software Foundation; version 2 of the License.                 *
 *                                                                            *
 ******************************************************************************/

// Qt includes.
#include <qfile.h>
#include <qfileinfo.h>
#include <qstring.h>
#include <qhbox.h>
#include <qlayout.h>
#include <qdom.h>
#include <qfile.h>
#include <qradiobutton.h>

// KDE includes.
#include <kglobal.h>
#include <klocale.h>
#include <klistview.h>
#include <klineedit.h>
#include <kdialog.h>
#include <kdialogbase.h>
#include <kpushbutton.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kregexpeditorinterface.h>
#include <ktrader.h>
#include <kparts/componentfactory.h>

// KTTS includes.
#include "filterconf.h"

// StringReplacer includes.
// #include "stringreplacerconf.h"
#include "sbdconf.h"
#include "sbdconf.moc"

/**
* Constructor 
*/
SbdConf::SbdConf( QWidget *parent, const char *name, const QStringList& /*args*/) :
    KttsFilterConf(parent, name)
{
    // kdDebug() << "SbdConf::SbdConf: Running" << endl;

/*    // Create configuration widget.
    QVBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(),
        KDialog::spacingHint(), "SbdConfigWidgetLayout");
    layout->setAlignment (Qt::AlignTop);
    m_widget = new SbdConfWidget(this, "SbdConfigWidget");
    layout->addWidget(m_widget);

    // Determine if kdeutils Regular Expression Editor is installed.
    m_reEditorInstalled = !KTrader::self()->query("KRegExpEditor/KRegExpEditor").isEmpty();*/

    // Set up defaults.
    defaults();
}

/**
* Destructor.
*/
SbdConf::~SbdConf(){
    // kdDebug() << "SbdConf::~SbdConf: Running" << endl;
}

/**
* This method is invoked whenever the module should read its 
* configuration (most of the times from a config file) and update the 
* user interface. This happens when the user clicks the "Reset" button in 
* the control center, to undo all of his changes and restore the currently 
* valid settings.  Note that kttsmgr calls this when the plugin is
* loaded, so it not necessary to call it in your constructor.
* The plugin should read its configuration from the specified group
* in the specified config file.
* @param config      Pointer to a KConfig object.
* @param configGroup Call config->setGroup with this argument before
*                    loading your configuration.
*/
void SbdConf::load(KConfig* /*config*/, const QString& configGroup){
    // kdDebug() << "SbdConf::load: Running" << endl;
}

/**
* This function gets called when the user wants to save the settings in 
* the user interface, updating the config files or wherever the 
* configuration is stored. The method is called when the user clicks "Apply" 
* or "Ok". The plugin should save its configuration in the specified
* group of the specified config file.
* @param config      Pointer to a KConfig object.
* @param configGroup Call config->setGroup with this argument before
*                    saving your configuration.
*/
void SbdConf::save(KConfig* /*config*/, const QString& configGroup){
    // kdDebug() << "SbdConf::save: Running" << endl;
}

/** 
* This function is called to set the settings in the module to sensible
* default values. It gets called when hitting the "Default" button. The 
* default values should probably be the same as the ones the application 
* uses when started without a config file.  Note that defaults should
* be applied to the on-screen widgets; not to the config file.
*/
void SbdConf::defaults(){
    // kdDebug() << "SbdConf::defaults: Running" << endl;
    // Default language is English.
    m_languageCode = "en";
    // kdDebug() << "SbdConf::defaults: Exiting" << endl;
}

/**
 * Indicates whether the plugin supports multiple instances.  Return
 * False if only one instance of the plugin can be configured.
 * @return            True if multiple instances are possible.
 */
bool SbdConf::supportsMultiInstance() { return false; }

/**
 * Returns the name of the plugin.  Displayed in Filters tab of KTTSMgr.
 * If there can be more than one instance of a filter, it should return
 * a unique name for each instance.  The name should be translated for
 * the user if possible.  If the plugin is not correctly configured,
 * return an empty string.
 * @return          Filter instance name.
 */
QString SbdConf::userPlugInName()
{
    return i18n("Sentence Boundary Detector");
}

