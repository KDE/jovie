/***************************************************** vim:set ts=4 sw=4 sts=4:
  Generic XML Transformation Filter Configuration class.
  -------------------
  Copyright:
  (C) 2005 by Gary Cramblitt <garycramblitt@comcast.net>
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

// Qt includes.


// KDE includes.
#include <klocale.h>
#include <klineedit.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kurlrequester.h>
#include <kstandarddirs.h>

// KTTS includes.
#include "filterconf.h"

// XmlTransformer includes.
#include "xmltransformerconf.h"
#include "xmltransformerconf.moc"

/**
* Constructor 
*/
XmlTransformerConf::XmlTransformerConf( QWidget *parent, const QStringList &args) :
    KttsFilterConf(parent)
{
    Q_UNUSED(args);
    kDebug() << "XmlTransformerConf::XmlTransformerConf: Running";

    // Create configuration widget.
    setupUi(this);

    // Set up defaults.
    kDebug() << "XmlTransformerConf:: setting up defaults";
    defaults();

    // Connect signals.
    connect( nameLineEdit, SIGNAL(textChanged(const QString&)),
         this, SLOT(configChanged()));
    connect( xsltPath, SIGNAL(textChanged(const QString&)),
         this, SLOT(configChanged()) );
    connect( xsltprocPath, SIGNAL(textChanged(const QString&)),
         this, SLOT(configChanged()) );
    connect( rootElementLineEdit, SIGNAL(textChanged(const QString&)),
         this, SLOT(configChanged()) );
    connect( doctypeLineEdit, SIGNAL(textChanged(const QString&)),
         this, SLOT(configChanged()) );
    connect( appIdLineEdit, SIGNAL(textChanged(const QString&)),
         this, SLOT(configChanged()) );
}

/**
* Destructor.
*/
XmlTransformerConf::~XmlTransformerConf(){
    // kDebug() << "XmlTransformerConf::~XmlTransformerConf: Running";
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
void XmlTransformerConf::load(KConfig* c, const QString& configGroup){
    // kDebug() << "XmlTransformerConf::load: Running";
    KConfigGroup config( c, configGroup );
    nameLineEdit->setText( config.readEntry( "UserFilterName", nameLineEdit->text() ) );
    xsltPath->setUrl( KUrl::fromPath( config.readEntry( "XsltFilePath", xsltPath->url().path() ) ) );
    xsltprocPath->setUrl( KUrl::fromPath( config.readEntry( "XsltprocPath", xsltprocPath->url().path() ) ) );
    rootElementLineEdit->setText(
            config.readEntry( "RootElement", rootElementLineEdit->text() ) );
    doctypeLineEdit->setText(
            config.readEntry( "DocType", doctypeLineEdit->text() ) );
    appIdLineEdit->setText(
            config.readEntry( "AppID", appIdLineEdit->text() ) );
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
void XmlTransformerConf::save(KConfig* c, const QString& configGroup){
    // kDebug() << "XmlTransformerConf::save: Running";
    KConfigGroup config( c, configGroup );
    config.writeEntry( "UserFilterName", nameLineEdit->text() );
    config.writeEntry( "XsltFilePath", realFilePath( xsltPath->url().path() ) );
    config.writeEntry( "XsltprocPath", realFilePath( xsltprocPath->url().path() ) );
    config.writeEntry( "RootElement", rootElementLineEdit->text() );
    config.writeEntry( "DocType", doctypeLineEdit->text() );
    config.writeEntry( "AppID", appIdLineEdit->text().replace(" ", "") );
}

/** 
* This function is called to set the settings in the module to sensible
* default values. It gets called when hitting the "Default" button. The 
* default values should probably be the same as the ones the application 
* uses when started without a config file.  Note that defaults should
* be applied to the on-screen widgets; not to the config file.
*/
void XmlTransformerConf::defaults(){
    // kDebug() << "XmlTransformerConf::defaults: Running";
    // Default name.
    nameLineEdit->setText(i18n( "XML Transformer" ));
    // Default XSLT path to installed xsl files.
    xsltPath->setUrl( KUrl::fromPath( KStandardDirs::locate("data", "kttsd/xmltransformer/") ) );
    // Default path to xsltproc.
    xsltprocPath->setUrl( KUrl("xsltproc") );
    // Default root element to "html".
    rootElementLineEdit->setText( "html" );
    // Default doctype to blank.
    doctypeLineEdit->setText( "" );
    // Default App ID to blank.
    appIdLineEdit->setText( "" );
    // kDebug() << "XmlTransformerConf::defaults: Exiting";
}

/**
 * Indicates whether the plugin supports multiple instances.  Return
 * False if only one instance of the plugin can be configured.
 * @return            True if multiple instances are possible.
 */
bool XmlTransformerConf::supportsMultiInstance() { return true; }

/**
 * Returns the name of the plugin.  Displayed in Filters tab of KTTSMgr.
 * If there can be more than one instance of a filter, it should return
 * a unique name for each instance.  The name should be translated for
 * the user if possible.  If the plugin is not correctly configured,
 * return an empty string.
 * @return          Filter instance name.
 */
QString XmlTransformerConf::userPlugInName()
{
    QString filePath = realFilePath(xsltprocPath->url().path());
    if (filePath.isEmpty()) return QString();
    if (getLocation(filePath).isEmpty()) return QString();

    filePath = realFilePath(xsltPath->url().path());
    if (filePath.isEmpty()) return QString();
    if (getLocation(filePath).isEmpty()) return QString();
    if (!QFileInfo(filePath).isFile()) return QString();

    return nameLineEdit->text();
}
