/****************************************************************************
	freettsconf.h
	Configuration widget and functions for FreeTTS (interactive) plug in
	-------------------
	Copyright : (C) 2004 Paul Giannaros
	-------------------
	Original author: Paul Giannaros <ceruleanblaze@gmail.com>
	Current Maintainer: Paul Giannaros <ceruleanblaze@gmail.com>
 ******************************************************************************/

/***************************************************************************
 *																					*
 *	 This program is free software; you can redistribute it and/or modify	*
 *	 it under the terms of the GNU General Public License as published by	*
 *	 the Free Software Foundation; version 2 of the License.				 *
 *																					 *
 ***************************************************************************/

#ifndef _FRETTSCONF_H_
#define _FRETTSCONF_H_

#include <qstringlist.h>

#include <kdebug.h>
#include <kconfig.h>
#include <kplayobject.h>

#include <pluginconf.h>

#include "freettsconfigwidget.h"
#include "freettsproc.h"

class KArtsServer;
class QStringList;

class FreeTTSConf : public PlugInConf {
	Q_OBJECT 
	
	public:
		/** Constructor */
		FreeTTSConf( QWidget* parent = 0, const char* name = 0, const QStringList &args = QStringList());
		
		/** Destructor */
		~FreeTTSConf();
		
		/** This method is invoked whenever the module should read its 
				configuration (most of the times from a config file) and update the 
				user interface. This happens when the user clicks the "Reset" button in 
				the control center, to undo all of his changes and restore the currently 
				valid settings. NOTE that this is not called after the modules is loaded,
				so you probably want to call this method in the constructor.*/
		void load(KConfig *config, const QString &langGroup);
		
		/** This function gets called when the user wants to save the settings in 
				the user interface, updating the config files or wherever the 
				configuration is stored. The method is called when the user clicks "Apply" 
				or "Ok". */
		void save(KConfig *config, const QString &langGroup);
		
		/** This function is called to set the settings in the module to sensible
				default values. It gets called when hitting the "Default" button. The 
				default values should probably be the same as the ones the application 
				uses when started without a config file. */
		void defaults();
		
		/**
		 * Function searches the $PATH for a file. If it exists, it returns the full path
		 * to that file. If not, then it returns an empty QString.
		 * @param name          The name of the file to search for.
		 * @returns                   The full path to the file or an empty QString.
		 */
		QString getLocation(const QString &name);
	
	public slots:
		void configChanged(bool t = true) { 
			emit changed(t); 
		};
		void slotFreeTTSTest_clicked();
		void slotSynthFinished();
	signals:
		void changed(bool);
	
	private:
		/// Configuration Widget.
		FreeTTSConfWidget *m_widget;
		
		/// FreeTTS synthesizer.
		FreeTTSProc *m_freettsProc;
		
		/// aRts server.
		KArtsServer *m_artsServer;
		
		/// aRts player.
		KDE::PlayObject *m_playObj;
		
		/// Synthesized wave file name.
		QString m_waveFile;
		
		/// The system path in a QStringList
		QStringList m_path;		
};
#endif
