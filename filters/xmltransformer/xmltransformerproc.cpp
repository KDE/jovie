/***************************************************** vim:set ts=4 sw=4 sts=4:
  Generic XML Transformation Filter Processing class.
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

// KDE includes.
#include <kdeversion.h>
#include <kconfig.h>
#include <ktempfile.h>
#include <kstandarddirs.h>
#include <kprocess.h>
#include <kdebug.h>

// KTTS includes.
#include "filterproc.h"

// XmlTransformer includes.
#include "xmltransformerproc.h"
#include "xmltransformerproc.moc"

/**
 * Constructor.
 */
XmlTransformerProc::XmlTransformerProc( QObject *parent, const char *name, const QStringList& ) :
    KttsFilterProc(parent, name)
{
}

/**
 * Destructor.
 */
/*virtual*/ XmlTransformerProc::~XmlTransformerProc()
{
}

/**
 * Initialize the filter.
 * @param config          Settings object.
 * @param configGroup     Settings Group.
 * @return                False if filter is not ready to filter.
 *
 * Note: The parameters are for reading from kttsdrc file.  Plugins may wish to maintain
 * separate configuration files of their own.
 */
bool XmlTransformerProc::init(KConfig* config, const QString& configGroup)
{
    config->setGroup( configGroup );
    m_UserFilterName = config->readEntry( "UserFilterName" );
    m_xsltFilePath = config->readEntry( "XsltFilePath" );
    m_xsltprocPath = config->readEntry( "XsltprocPath" );
    return ( m_xsltFilePath.isEmpty() || m_xsltprocPath.isEmpty() );
}

/**
 * Convert input, returning output.
 * @param inputText         Input text.
 * @param talkerCode        TalkerCode structure for the talker that KTTSD intends to
 *                          use for synthing the text.  Useful for extracting hints about
 *                          how to filter the text.  For example, languageCode.
 */
/*virtual*/ QString XmlTransformerProc::convert(QString& inputText, TalkerCode* /*talkerCode*/)
{
    // If not propertly configured, do nothing.
    if ( m_xsltFilePath.isEmpty() || m_xsltprocPath.isEmpty() ) return inputText;

    /// Write @param text to a temporary file.
    KTempFile inFile(locateLocal("tmp", "kttsd-"), ".xml");
    QString inFilename = inFile.file()->name();
    QTextStream* wstream = inFile.textStream();
    if (wstream == 0) {
        /// wtf...
        kdDebug() << "XmlTransformerProc::convert: Can't write to " << inFilename << endl;;
        return inputText;
    }
    // TODO: Is encoding an issue here?
    // TODO: It would be nice if we detected whether the XML is properly formed
    // with the required xml processing instruction and encoding attribute.  If
    // not wrap it in such.  But maybe this should be handled by SpeechData::setText()?
    *wstream << inputText;
    inFile.close();
#if KDE_VERSION >= KDE_MAKE_VERSION (3,3,0)
    inFile.sync();
#endif

    // Get a temporary output file name.
    KTempFile outFile(locateLocal("tmp", "kttsd-"), ".output");
    QString outFilename = outFile.file()->name();
    outFile.close();
    // outFile.unlink();    // only activate this if necessary.

    /// Spawn an xsltproc process to apply our stylesheet to input file.
    KProcess* xsltProc = new KProcess;
    *xsltProc << m_xsltprocPath;
    *xsltProc << "-o" << outFilename  << "--novalid"
            << m_xsltFilePath << inFilename;
    // Warning: This won't compile under KDE 3.2.  See FreeTTS::argsToStringList().
    // kdDebug() << "SSMLConvert::transform: executing command: " <<
    //     m_xsltProc->args() << endl;

    // connect(m_xsltProc, SIGNAL(processExited(KProcess*)),
    //         this, SLOT(slotProcessExited(KProcess*)));
    if (!xsltProc->start(KProcess::Block, KProcess::NoCommunication))
    {
        kdDebug() << "XmlTransformerProc::convert: Error starting xsltproc" << endl;
        return inputText;
    }

    /// Read back the data that was written to /tmp/fileName.output.
    QFile readfile(outFilename);
    if(!readfile.open(IO_ReadOnly)) {
        /// uhh yeah... Issues writing to the output file.
        kdDebug() << "XmlTransformerProc::convert: Could not read file " << outFilename << endl;
        return QString::null;
    }
    QTextStream rstream(&readfile);
    QString convertedData = rstream.read();
    readfile.close();

    kdDebug() << "XmlTransformerProc::convert: Read file at " + inFilename + " and created " + outFilename + " based on the stylesheet at " << m_xsltFilePath << endl;

    // Clean up.
    QFile::remove(inFilename);
    QFile::remove(outFilename);

    return convertedData;
}
