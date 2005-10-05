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
#include <qstring.h>
#include <qlayout.h>
//Added by qt3to4:
#include <QVBoxLayout>

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
XmlTransformerConf::XmlTransformerConf( QWidget *parent, const char *name, const QStringList& /*args*/) :
    KttsFilterConf(parent, name)
{
    // kdDebug() << "XmlTransformerConf::XmlTransformerConf: Running" << endl;

    // Create configuration widget.
    QVBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(),
        KDialog::spacingHint(), "XmlTransformerConfWidgetLayout");
    layout->setAlignment (Qt::AlignTop);
    m_widget = new XmlTransformerConfWidget(this, "XmlTransformerConfigWidget");
    layout->addWidget(m_widget);

    // Set up defaults.
    defaults();

    // Connect signals.
    connect( m_widget->nameLineEdit, SIGNAL(textChanged(const QString&)),
         this, SLOT(configChanged()));
    connect( m_widget->xsltPath, SIGNAL(textChanged(const QString&)),
         this, SLOT(configChanged()) );
    connect( m_widget->xsltprocPath, SIGNAL(textChanged(const QString&)),
         this, SLOT(configChanged()) );
    connect( m_widget->rootElementLineEdit, SIGNAL(textChanged(const QString&)),
         this, SLOT(configChanged()) );
    connect( m_widget->doctypeLineEdit, SIGNAL(textChanged(const QString&)),
         this, SLOT(configChanged()) );
    connect( m_widget->appIdLineEdit, SIGNAL(textChanged(const QString&)),
         this, SLOT(configChanged()) );
}

/**
* Destructor.
*/
XmlTransformerConf::~XmlTransformerConf(){
    // kdDebug() << "XmlTransformerConf::~XmlTransformerConf: Running" << endl;
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
void XmlTransformerConf::load(KConfig* config, const QString& configGroup){
    // kdDebug() << "XmlTransformerConf::load: Running" << endl;
    config->setGroup( configGroup );
    m_widget->nameLineEdit->setText( config->readEntry( "UserFilterName", m_widget->nameLineEdit->text() ) );
    m_widget->xsltPath->setURL( config->readEntry( "XsltFilePath", m_widget->xsltPath->url() ) );
    m_widget->xsltprocPath->setURL( config->readEntry( "XsltprocPath", m_widget->xsltprocPath->url() ) );
    m_widget->rootElementLineEdit->setText(
            config->readEntry( "RootElement", m_widget->rootElementLineEdit->text() ) );
    m_widget->doctypeLineEdit->setText(
            config->readEntry( "DocType", m_widget->doctypeLineEdit->text() ) );
    m_widget->appIdLineEdit->setText(
            config->readEntry( "AppID", m_widget->appIdLineEdit->text() ) );
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
void XmlTransformerConf::save(KConfig* config, const QString& configGroup){
    // kdDebug() << "XmlTransformerConf::save: Running" << endl;
    config->setGroup( configGroup );
    config->writeEntry( "UserFilterName", m_widget->nameLineEdit->text() );
    config->writeEntry( "XsltFilePath", realFilePath( m_widget->xsltPath->url() ) );
    config->writeEntry( "XsltprocPath", realFilePath( m_widget->xsltprocPath->url() ) );
    config->writeEntry( "RootElement", m_widget->rootElementLineEdit->text() );
    config->writeEntry( "DocType", m_widget->doctypeLineEdit->text() );
    config->writeEntry( "AppID", m_widget->appIdLineEdit->text().replace(" ", "") );
}

/** 
* This function is called to set the settings in the module to sensible
* default values. It gets called when hitting the "Default" button. The 
* default values should probably be the same as the ones the application 
* uses when started without a config file.  Note that defaults should
* be applied to the on-screen widgets; not to the config file.
*/
void XmlTransformerConf::defaults(){
    // kdDebug() << "XmlTransformerConf::defaults: Running" << endl;
    // Default name.
    m_widget->nameLineEdit->setText(i18n( "XML Transformer" ));
    // Default XSLT path to installed xsl files.
    m_widget->xsltPath->setURL( locate("data", "kttsd/xmltransformer/") );
    // Default path to xsltproc.
    m_widget->xsltprocPath->setURL( "xsltproc" );
    // Default root element to "html".
    m_widget->rootElementLineEdit->setText( "html" );
    // Default doctype to blank.
    m_widget->doctypeLineEdit->setText( "" );
    // Default App ID to blank.
    m_widget->appIdLineEdit->setText( "" );
    // kdDebug() << "XmlTransformerConf::defaults: Exiting" << endl;
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
    QString filePath = realFilePath(m_widget->xsltprocPath->url());
    if (filePath.isEmpty()) return QString::null;
    if (getLocation(filePath).isEmpty()) return QString::null;

    filePath = realFilePath(m_widget->xsltPath->url());
    if (filePath.isEmpty()) return QString::null;
    if (getLocation(filePath).isEmpty()) return QString::null;
    if (!QFileInfo(filePath).isFile()) return QString::null;

    return m_widget->nameLineEdit->text();
}
