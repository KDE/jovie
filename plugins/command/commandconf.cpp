#include <qlayout.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qstring.h>
#include <qstringlist.h>

#include <kdebug.h>
#include <klocale.h>
#include <kdialog.h>
#include <kcombobox.h>

#include <pluginconf.h>

#include "texttospeechconfigurationwidget.h"
#include "commandconf.h"
#include "commandconf.moc"

/** Constructor */
CommandConf::CommandConf( QWidget* parent, const char* name, const QStringList &args) : 
  PlugInConf( parent, name ){
  kdDebug() << "Running: CommandConf::CommandConf( QWidget* parent, const char* name, const QStringList &args)" << endl;
  QVBoxLayout *layout = new QVBoxLayout (this, KDialog::marginHint(), KDialog::spacingHint(), "CommandConfigWidgetLayout");
  layout->setAlignment (Qt::AlignTop); 

  configWidget = new TextToSpeechConfigurationWidget (this, "configWidget");
  connect( configWidget->urlReq, SIGNAL( textChanged(const QString&) ), this, SLOT( configChanged() ) );
  // Can anybody spot the problem of this connection ? it doesn't work.
  connect( configWidget->characterCodingBox, SIGNAL( returnPressed() ), this, SLOT( configChanged() ) );
  connect( configWidget->stdInButton, SIGNAL( clicked() ), this, SLOT( configChanged() ) );
  layout->addWidget (configWidget);
}

/** Desctructor */
CommandConf::~CommandConf(){
  kdDebug() << "Running: CommandConf::~CommandConf()" << endl;
}

void CommandConf::load(KConfig *config, const QString &langGroup) {
  kdDebug() << "Running: CommandConf::load()" << endl;
   configWidget->readOptions (config, langGroup);
}

void CommandConf::save(KConfig *config, const QString &langGroup) {
  kdDebug() << "Running: CommandConf::save()" << endl;
   configWidget->ok ();
   configWidget->saveOptions (config, langGroup);
}

void CommandConf::defaults(){
  kdDebug() << "Running: CommandConf::defaults()" << endl;
}
