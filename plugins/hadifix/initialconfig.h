/**
 * Tries to find hadifix and mbrola by looking onto the hard disk. This is
 * neccessary because both hadifix and mbrola do not have standard
 * installation directories.
 */
void findInitialConfig() {
   QString hadifixDataPath = findHadifixDataPath();
   
   defaultHadifixExec = findExecutable(QStringList("txt2pho"), hadifixDataPath+"/../");
   
   QStringList list; list += "mbrola"; list += "mbrola-linux-i386";
   defaultMbrolaExec = findExecutable(list, hadifixDataPath+"/../../mbrola/");
   
   defaultVoices = findVoices (defaultMbrolaExec, hadifixDataPath);
}

/** Tries to find the hadifix data path by looking into a number of files. */
QString findHadifixDataPath () {
   QStringList files;
   files += "/etc/txt2pho";
   files += QDir::homeDirPath()+"/.txt2phorc";
   
   QStringList::iterator it;
   for (it = files.begin(); it != files.end(); ++it) {

      QFile file(*it);
      if ( file.open(QIODevice::ReadOnly) ) {
         QTextStream stream(&file);
         
         while (!stream.atEnd()) {
            QString s = stream.readLine().trimmed();
	    // look for a line "DATAPATH=..."
	    
	    if (s.startsWith("DATAPATH")) {
	       s = s.mid(8, s.length()-8).trimmed();
	       if (s.startsWith("=")) {
	          s = s.mid(1, s.length()-1).trimmed();
		  if (s.startsWith("/"))
		     return s;
		  else {
		     QFileInfo info (QFileInfo(*it).dirPath() + "/" + s);
		     return info.absFilePath();
		  }
	       }
	    }
         }
         file.close();
      }
   }
   return "/usr/local/txt2pho/";
}

/** Tries to find the an executable by looking onto the hard disk. */
QString findExecutable (const QStringList &names, const QString &possiblePath) {
   // a) Try to find it directly
   QStringList::ConstIterator it;
   QStringList::ConstIterator itEnd = names.constEnd();
   for (it = names.constBegin(); it != itEnd; ++it) {
      QString executable = KStandardDirs::findExe (*it);
      if (!executable.isNull() && !executable.isEmpty())
         return executable;
   }

   // b) Try to find it in the path specified by the second parameter
   for (it = names.constBegin(); it != itEnd; ++it) {
      QFileInfo info (possiblePath+*it);
      if (info.exists() && info.isExecutable() && info.isFile()) {
         return info.absFilePath();
      }
   }

   // Both tries failed, so the user has to locate the executable.
   return QString::null;
}

/** Tries to find the voice files by looking onto the hard disk. */
QStringList findVoices(QString mbrolaExec, const QString &hadifixDataPath) {
   
   // First of all:
   // dereference links to the mbrola executable (if mbrolaExec is a link).
   for (int i = 0; i < 10; ++i) {
      // If we have a chain of more than ten links something is surely wrong.
      QFileInfo info (mbrolaExec);
      if (info.exists() && info.isSymLink())
            mbrolaExec = info.readLink();
   }

   // Second:
   // create a list of directories that possibly contain voice files
   QStringList list;
   
   // 2a) search near the mbrola executable
   QFileInfo info (mbrolaExec);
   if (info.exists() && info.isFile() && info.isExecutable()) {
      QString mbrolaPath = info.dirPath (true);
      list += mbrolaPath;
   }

   // 2b) search near the hadifix data path
   info.setFile(hadifixDataPath + "../../mbrola");
   QString mbrolaPath = info.dirPath (true) + "/mbrola";
   if (!list.contains(mbrolaPath))
      list += mbrolaPath;

   // 2c) broaden the search by adding subdirs (with a depth of 2)
   QStringList subDirs = findSubdirs (list);
   QStringList subSubDirs = findSubdirs (subDirs);
   list += subDirs;
   list += subSubDirs;

   // Third:
   // look into each of these directories and search for voice files.
   QStringList result;
   QStringList::iterator it;
   for (it = list.begin(); it != list.end(); ++it) {
      QDir baseDir (*it, QString::null,
                    QDir::Name|QDir::IgnoreCase, QDir::Files);
      QStringList files = baseDir.entryList();

      QStringList::iterator iter;
      for (iter = files.begin(); iter != files.end(); ++iter) {
         // Voice files start with "MBROLA", but are afterwards binary files
	 QString filename = *it + "/" + *iter;
         QFile file (filename);
         if (file.open(QIODevice::ReadOnly)) {
            QTextStream stream(&file);
            if (!stream.atEnd()) {
               QString s = stream.readLine();
               if (s.startsWith("MBROLA"))
	          if (HadifixProc::determineGender(mbrolaExec, filename)
		   != HadifixProc::NoVoice
		  )
                     result += filename;
               file.close();
            }
         }
      }
   }
   return result;
}

/** Returns a list of subdirs (with absolute paths) */
QStringList findSubdirs (const QStringList &baseDirs) {
   QStringList result;

   QStringList::ConstIterator it;
   QStringList::ConstIterator itEnd = baseDirs.constEnd();
   for (it = baseDirs.constBegin(); it != itEnd; ++it) {
      // a) get a list of directory names
      QDir baseDir (*it, QString::null,
                    QDir::Name|QDir::IgnoreCase, QDir::Dirs);
      QStringList list = baseDir.entryList();

      // b) produce absolute paths
      QStringList::ConstIterator iter;
      QStringList::ConstIterator iterEnd = list.constEnd();
      for (iter = list.constBegin(); iter != iterEnd; ++iter) {
         if ((*iter != ".") && (*iter != ".."))
            result += *it + "/" + *iter;
      }
   }
   return result;
}
