/***************************************************** vim:set ts=4 sw=4 sts=4:
  Generic XML Transformation Filter Processing class.
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
#include <QFile>
#include <QRegExp>
#include <QTextStream>

// KDE includes.
#include <kdeversion.h>
#include <kconfig.h>
#include <ktempfile.h>
#include <kstandarddirs.h>
#include <kprocess.h>
#include <kdebug.h>

// KTTS includes.
#include "filterproc.h"
#include "utils.h"

// XmlTransformer includes.
#include "xmltransformerproc.h"
#include "xmltransformerproc.moc"

/**
 * Constructor.
 */
XmlTransformerProc::XmlTransformerProc( QObject *parent, const char *name, const QStringList& ) :
    KttsFilterProc(parent, name)
{
    m_xsltProc = 0;
}

/**
 * Destructor.
 */
/*virtual*/ XmlTransformerProc::~XmlTransformerProc()
{
    delete m_xsltProc;
    if (!m_inFilename.isEmpty()) QFile::remove(m_inFilename);
    if (!m_outFilename.isEmpty()) QFile::remove(m_outFilename);
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
    // kdDebug() << "XmlTransformerProc::init: Running." << endl;
    config->setGroup( configGroup );
    m_UserFilterName = config->readEntry( "UserFilterName" );
    m_xsltFilePath = config->readEntry( "XsltFilePath" );
    m_xsltprocPath = config->readEntry( "XsltprocPath" );
    m_rootElementList = config->readEntry( "RootElement", QStringList(), ',' );
    m_doctypeList = config->readEntry( "DocType", QStringList(), ',' );
    m_appIdList = config->readEntry( "AppID", QStringList(), ',' );
    kdDebug() << "XmlTransformerProc::init: m_xsltprocPath = " << m_xsltprocPath << endl;
    kdDebug() << "XmlTransformerProc::init: m_xsltFilePath = " << m_xsltFilePath << endl;
    return ( m_xsltFilePath.isEmpty() || m_xsltprocPath.isEmpty() );
}

/**
 * Returns True if the plugin supports asynchronous processing,
 * i.e., supports asyncConvert method.
 * @return                        True if this plugin supports asynchronous processing.
 *
 * If the plugin returns True, it must also implement @ref getState .
 * It must also emit @ref filteringFinished when filtering is completed.
 * If the plugin returns True, it must also implement @ref stopFiltering .
 * It must also emit @ref filteringStopped when filtering has been stopped.
 */
/*virtual*/ bool XmlTransformerProc::supportsAsync() { return true; }

/**
 * Convert input, returning output.
 * @param inputText         Input text.
 * @param talkerCode        TalkerCode structure for the talker that KTTSD intends to
 *                          use for synthing the text.  Useful for extracting hints about
 *                          how to filter the text.  For example, languageCode.
 * @param appId             The DCOP appId of the application that queued the text.
 *                          Also useful for hints about how to do the filtering.
 */
/*virtual*/ QString XmlTransformerProc::convert(const QString& inputText, TalkerCode* talkerCode,
    const QByteArray& appId)
{
    // kdDebug() << "XmlTransformerProc::convert: Running." << endl;
    // If not properly configured, do nothing.
    if ( m_xsltFilePath.isEmpty() || m_xsltprocPath.isEmpty() )
    {
        kdDebug() << "XmlTransformerProc::convert: not properly configured" << endl;
        return inputText;
    }
    // Asynchronously convert and wait for completion.
    if (asyncConvert(inputText, talkerCode, appId))
    {
        waitForFinished();
        m_state = fsIdle;
        return m_text;
    } else
        return inputText;
}

/**
 * Convert input.  Runs asynchronously.
 * @param inputText         Input text.
 * @param talkerCode        TalkerCode structure for the talker that KTTSD intends to
 *                          use for synthing the text.  Useful for extracting hints about
 *                          how to filter the text.  For example, languageCode.
 * @param appId             The DCOP appId of the application that queued the text.
 *                          Also useful for hints about how to do the filtering.
 * @return                  False if the filter cannot perform the conversion.
 *
 * When conversion is completed, emits signal @ref filteringFinished.  Calling
 * program may then call @ref getOutput to retrieve converted text.  Calling
 * program must call @ref ackFinished to acknowledge the conversion.
 */
/*virtual*/ bool XmlTransformerProc::asyncConvert(const QString& inputText, TalkerCode* /*talkerCode*/,
    const QByteArray& appId)
{
    m_wasModified = false;

    // kdDebug() << "XmlTransformerProc::asyncConvert: Running." << endl;
    m_text = inputText;
    // If not properly configured, do nothing.
    if ( m_xsltFilePath.isEmpty() || m_xsltprocPath.isEmpty() )
    {
        kdDebug() << "XmlTransformerProc::asyncConvert: not properly configured." << endl;
        return false;
    }

    bool found = false;
    // If not correct XML type, or DOCTYPE, do nothing.
    if ( !m_rootElementList.isEmpty() )
    {
        // kdDebug() << "XmlTransformerProc::asyncConvert:: searching for root elements " << m_rootElementList << endl;
        for ( int ndx=0; ndx < m_rootElementList.count(); ++ndx )
        {
            if ( KttsUtils::hasRootElement( inputText, m_rootElementList[ndx] ) )
            {
                found = true;
                break;
            }
        }
        if ( !found && m_doctypeList.isEmpty() )
        {
            kdDebug() << "XmlTransformerProc::asyncConvert: Did not find root element(s)" << m_rootElementList << endl;
            return false;
        }
    }
    if ( !found && !m_doctypeList.isEmpty() )
    {
        for ( int ndx=0; ndx < m_doctypeList.count(); ++ndx )
        {
            if ( KttsUtils::hasDoctype( inputText, m_doctypeList[ndx] ) )
            {
                found = true;
                break;
            }
        }
        if ( !found )
        {
            // kdDebug() << "XmlTransformerProc::asyncConvert: Did not find doctype(s)" << m_doctypeList << endl;
            return false;
        }
    }

    // If appId doesn't match, return input unmolested.
    if ( !m_appIdList.isEmpty() )
    {
        QString appIdStr = appId;
        // kdDebug() << "XmlTransformrProc::convert: converting " << inputText << " if appId "
        //     << appId << " matches " << m_appIdList << endl;
        found = false;
        for ( int ndx=0; ndx < m_appIdList.count(); ++ndx )
        {
            if ( appIdStr.contains(m_appIdList[ndx]) )
            {
                found = true;
                break;
            }
        }
        if ( !found )
        {
            // kdDebug() << "XmlTransformerProc::asyncConvert: Did not find appId(s)" << m_appIdList << endl;
            return false;
        }
    }

    /// Write @param text to a temporary file.
    KTempFile inFile(locateLocal("tmp", "kttsd-"), ".xml");
    m_inFilename = inFile.file()->name();
    QTextStream* wstream = inFile.textStream();
    if (wstream == 0) {
        /// wtf...
        kdDebug() << "XmlTransformerProc::convert: Can't write to " << m_inFilename << endl;;
        return false;
    }
    // TODO: Is encoding an issue here?
    // If input does not have xml processing instruction, add it.
    if (!inputText.startsWith("<?xml")) *wstream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    // FIXME: Temporary Fix until Konqi returns properly formatted xhtml with & coded as &amp;
    // This will change & inside a CDATA section, which is not good, and also within comments and
    // processing instructions, which is OK because we don't speak those anyway.
    QString text = inputText;
    text.replace(QRegExp("&(?!amp;)"),"&amp;");
    *wstream << text;
    inFile.close();
#if KDE_VERSION >= KDE_MAKE_VERSION (3,3,0)
    inFile.sync();
#endif

    // Get a temporary output file name.
    KTempFile outFile(locateLocal("tmp", "kttsd-"), ".output");
    m_outFilename = outFile.file()->name();
    outFile.close();
    // outFile.unlink();    // only activate this if necessary.

    /// Spawn an xsltproc process to apply our stylesheet to input file.
    m_xsltProc = new KProcess;
    *m_xsltProc << m_xsltprocPath;
    *m_xsltProc << "-o" << m_outFilename  << "--novalid"
            << m_xsltFilePath << m_inFilename;
    // Warning: This won't compile under KDE 3.2.  See FreeTTS::argsToStringList().
    // kdDebug() << "SSMLConvert::transform: executing command: " <<
    //     m_xsltProc->args() << endl;

    m_state = fsFiltering;
    connect(m_xsltProc, SIGNAL(processExited(KProcess*)),
            this, SLOT(slotProcessExited(KProcess*)));
    connect(m_xsltProc, SIGNAL(receivedStdout(KProcess*, char*, int)),
            this, SLOT(slotReceivedStdout(KProcess*, char*, int)));
    connect(m_xsltProc, SIGNAL(receivedStderr(KProcess*, char*, int)),
            this, SLOT(slotReceivedStderr(KProcess*, char*, int)));
    if (!m_xsltProc->start(KProcess::NotifyOnExit,
         static_cast<KProcess::Communication>(KProcess::Stdout | KProcess::Stderr)))
    {
        kdDebug() << "XmlTransformerProc::convert: Error starting xsltproc" << endl;
        m_state = fsIdle;
        return false;
    }
    return true;
}

// Process output when xsltproc exits.
void XmlTransformerProc::processOutput()
{
    QFile::remove(m_inFilename);

    int exitStatus = 11;
    if (m_xsltProc->normalExit())
        exitStatus = m_xsltProc->exitStatus();
    else
        kdDebug() << "XmlTransformerProc::processOutput: xsltproc was killed." << endl;

    delete m_xsltProc;
    m_xsltProc = 0;

    if (exitStatus != 0)
    {
        kdDebug() << "XmlTransformerProc::processOutput: xsltproc abnormal exit.  Status = " << exitStatus << endl;
        m_state = fsFinished;
        QFile::remove(m_outFilename);
        emit filteringFinished();
        return;
    }

    /// Read back the data that was written to /tmp/fileName.output.
    QFile readfile(m_outFilename);
    if(!readfile.open(QIODevice::ReadOnly)) {
        /// uhh yeah... Issues writing to the output file.
        kdDebug() << "XmlTransformerProc::processOutput: Could not read file " << m_outFilename << endl;
        m_state = fsFinished;
        emit filteringFinished();
    }
    QTextStream rstream(&readfile);
    m_text = rstream.read();
    readfile.close();

    kdDebug() << "XmlTransformerProc::processOutput: Read file at " + m_inFilename + " and created " + m_outFilename + " based on the stylesheet at " << m_xsltFilePath << endl;

    // Clean up.
    QFile::remove(m_outFilename);

    m_state = fsFinished;
    m_wasModified = true;
    emit filteringFinished();
}

/**
 * Waits for a previous call to asyncConvert to finish.
 */
/*virtual*/ void XmlTransformerProc::waitForFinished()
{
    if (m_xsltProc)
    {
        if (m_xsltProc->isRunning())
        {
            if ( !m_xsltProc->wait( 15 ) )
            {
                m_xsltProc->kill();
                kdDebug() << "XmlTransformerProc::waitForFinished: After waiting 15 seconds, xsltproc process seems to hung.  Killing it." << endl;
                processOutput();
            }
        }
    }
}

/**
 * Returns the state of the Filter.
 */
/*virtual*/ int XmlTransformerProc::getState() { return m_state; }

/**
 * Returns the filtered output.
 */
/*virtual*/ QString XmlTransformerProc::getOutput() { return m_text; }

/**
 * Acknowledges the finished filtering.
 */
/*virtual*/ void XmlTransformerProc::ackFinished()
{
    m_state = fsIdle;
    m_text.clear();
}

/**
 * Stops filtering.  The filteringStopped signal will emit when filtering
 * has in fact stopped and state returns to fsIdle;
 */
/*virtual*/ void XmlTransformerProc::stopFiltering()
{
    m_state = fsStopping;
    m_xsltProc->kill();
}

/**
 * Did this filter do anything?  If the filter returns the input as output
 * unmolested, it should return False when this method is called.
 */
/*virtual*/ bool XmlTransformerProc::wasModified() { return m_wasModified; }

void XmlTransformerProc::slotProcessExited(KProcess*)
{
    // kdDebug() << "XmlTransformerProc::slotProcessExited: xsltproc has exited." << endl;
    processOutput();
}

void XmlTransformerProc::slotReceivedStdout(KProcess*, char* /*buffer*/, int /*buflen*/)
{
    // QString buf = QString::fromLatin1(buffer, buflen);
    // kdDebug() << "XmlTransformerProc::slotReceivedStdout: Received from xsltproc: " << buf << endl;
}

void XmlTransformerProc::slotReceivedStderr(KProcess*, char* buffer, int buflen)
{
    QString buf = QString::fromLatin1(buffer, buflen);
    kdDebug() << "XmlTransformerProc::slotReceivedStderr: Received error from xsltproc: " << buf << endl;
}

