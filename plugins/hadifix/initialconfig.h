
/**
 * Tries to find hadifix and mbrola by looking onto the hard disk. This is
 * neccessary because both hadifix and mbrola do not have standard
 * installation directories.
 */
void findInitialConfig() {
   TQString hadifixDataPath = findHadifixDataPath();
   
   defaultHadifixExec = findExecutable("txt2pho", hadifixDataPath+"/../");
   
   TQStringList list; list += "mbrola"; list += "mbrola-linux-i386";
   defaultMbrolaExec = findExecutable(list, hadifixDataPath+"/../../mbrola/");
   
   defaultVoices = findVoices (defaultMbrolaExec, hadifixDataPath);
}

/** Tries to find the hadifix data path by looking into a number of files. */
TQString findHadifixDataPath () {
   TQStringList files;
   files += "/etc/txt2pho";
   files += TQDir::homeDirPath()+"/.txt2phorc";
   
   TQStringList::iterator it;
   for (it = files.begin(); it != files.end(); ++it) {

      TQFile file(*it);
      if ( file.open(IO_ReadOnly) ) {
         TQTextStream stream(&file);
         
         while (!stream.atEnd()) {
            TQString s = stream.readLine().stripWhiteSpace();
	    // look for a line "DATAPATH=..."
	    
	    if (s.startsWith("DATAPATH")) {
	       s = s.mid(8, s.length()-8).stripWhiteSpace();
	       if (s.startsWith("=")) {
	          s = s.mid(1, s.length()-1).stripWhiteSpace();
		  if (s.startsWith("/"))
		     return s;
		  else {
		     TQFileInfo info (TQFileInfo(*it).dirPath() + "/" + s);
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
TQString findExecutable (const TQStringList &names, const TQString &possiblePath) {
   // a) Try to find it directly
   TQStringList::ConstIterator it;
   TQStringList::ConstIterator itEnd = names.constEnd();
   for (it = names.constBegin(); it != itEnd; ++it) {
      TQString executable = KStandardDirs::findExe (*it);
      if (!executable.isNull() && !executable.isEmpty())
         return executable;
   }

   // b) Try to find it in the path specified by the second parameter
   for (it = names.constBegin(); it != itEnd; ++it) {
      TQFileInfo info (possiblePath+*it);
      if (info.exists() && info.isExecutable() && info.isFile()) {
         return info.absFilePath();
      }
   }

   // Both tries failed, so the user has to locate the executable.
   return TQString();
}

/** Tries to find the voice files by looking onto the hard disk. */
TQStringList findVoices(TQString mbrolaExec, const TQString &hadifixDataPath) {
   
   // First of all:
   // dereference links to the mbrola executable (if mbrolaExec is a link).
   for (int i = 0; i < 10; ++i) {
      // If we have a chain of more than ten links something is surely wrong.
      TQFileInfo info (mbrolaExec);
      if (info.exists() && info.isSymLink())
            mbrolaExec = info.readLink();
   }

   // Second:
   // create a list of directories that possibly contain voice files
   TQStringList list;
   
   // 2a) search near the mbrola executable
   TQFileInfo info (mbrolaExec);
   if (info.exists() && info.isFile() && info.isExecutable()) {
      TQString mbrolaPath = info.dirPath (true);
      list += mbrolaPath;
   }

   // 2b) search near the hadifix data path
   info.setFile(hadifixDataPath + "../../mbrola");
   TQString mbrolaPath = info.dirPath (true) + "/mbrola";
   if (!list.contains(mbrolaPath))
      list += mbrolaPath;

   // 2c) broaden the search by adding subdirs (with a depth of 2)
   TQStringList subDirs = findSubdirs (list);
   TQStringList subSubDirs = findSubdirs (subDirs);
   list += subDirs;
   list += subSubDirs;

   // Third:
   // look into each of these directories and search for voice files.
   TQStringList result;
   TQStringList::iterator it;
   for (it = list.begin(); it != list.end(); ++it) {
      TQDir baseDir (*it, TQString(),
                    TQDir::Name|TQDir::IgnoreCase, TQDir::Files);
      TQStringList files = baseDir.entryList();

      TQStringList::iterator iter;
      for (iter = files.begin(); iter != files.end(); ++iter) {
         // Voice files start with "MBROLA", but are afterwards binary files
	 TQString filename = *it + "/" + *iter;
         TQFile file (filename);
         if (file.open(IO_ReadOnly)) {
            TQTextStream stream(&file);
            if (!stream.atEnd()) {
               TQString s = stream.readLine();
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
TQStringList findSubdirs (const TQStringList &baseDirs) {
   TQStringList result;

   TQStringList::ConstIterator it;
   TQStringList::ConstIterator itEnd = baseDirs.constEnd();
   for (it = baseDirs.constBegin(); it != itEnd; ++it) {
      // a) get a list of directory names
      TQDir baseDir (*it, TQString(),
                    TQDir::Name|TQDir::IgnoreCase, TQDir::Dirs);
      TQStringList list = baseDir.entryList();

      // b) produce absolute paths
      TQStringList::ConstIterator iter;
      TQStringList::ConstIterator iterEnd = list.constEnd();
      for (iter = list.constBegin(); iter != iterEnd; ++iter) {
         if ((*iter != ".") && (*iter != ".."))
            result += *it + "/" + *iter;
      }
   }
   return result;
}
