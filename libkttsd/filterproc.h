/***************************************************** vim:set ts=4 sw=4 sts=4:
  Filter Processing class.
  This is the interface definition for text filters.
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
#ifndef _FILTERPROC_H_
#define _FILTERPROC_H_

#include <qobject.h>
#include <qstringlist.h>
#include <qtextstream.h>

typedef QTextStream KttsFilterStream;

class TalkerCode;

class KttsFilterChain : public QObject
{
public:
    /**
     * Constructor.
     */
    KttsFilterChain( QObject *parent = 0, const char *name = 0);
    /**
     * Destructor.
     */
    ~KttsFilterChain();

    KttsFilterStream* inputStream();
    KttsFilterStream* outputStream();

private:
    KttsFilterStream* m_inStream;
    KttsFilterStream* m_outStream;
};

class KttsFilterProc : virtual public QObject
{
    Q_OBJECT

public:
    /**
     * This enum is used to signal the return state of your filter.
     * Return OK in @ref convert() in case everything worked as expected.
     * Feel free to add some more error conditions @em before the last item
     * if it's needed.
     */
    enum ConversionStatus { OK, StupidError, UsageError, CreationError, FileNotFound,
        StorageCreationError, BadMimeType, BadConversionGraph,
        EmbeddedDocError, WrongFormat, NotImplemented,
        ParsingError, InternalError, UnexpectedEOF,
        UnexpectedOpcode, UserCancelled, OutOfMemory,
        JustInCaseSomeBrokenCompilerUsesLessThanAByte = 255 };

    /**
     * Constructor.
     */
    KttsFilterProc( QObject *parent, const char *name, const QStringList &args = QStringList() );

    /**
     * Destructor.
     */
    virtual ~KttsFilterProc();

    /**
     * The filter chain calls this method to perform the actual conversion.
     * The passed mimetypes should be a pair of those you specified in your
     * .desktop file.
     * You @em have to implement this method to make the filter work.
     * Your filter should decide whether to apply itself to the input stream.
     * If not, simply connect the input stream to the output stream.
     * The default implementation does just that.
     *
     * @param from The mimetype of the source file/document
     * @param to The mimetype of the destination file/document
     * @return The error status, see the @ref #ConversionStatus enum.
     *         KttsFilterProc::OK means that everything is alright.
     */
    virtual KttsFilterProc::ConversionStatus convert(
        const QCString& from,
        const QCString& to );

signals:
    /**
     * Emit this signal with a value in the range of 1...100 to have some
     * progress feedback for the user in the statusbar of the application.
     *
     * @param value The actual progress state. Should always remain in
     * the range 1..100.
     */
     void sigProgress( int value );

protected:
    /**
     * Use this pointer to access all information about input/output
     * during the conversion. @em Don't use it in the constructor -
     * it's invalid while constructing the object!
     */
    KttsFilterChain* m_chain;

    /**
     * A Talker Code structure, which indicates the Talker that KTTSD
     * will use to speak the text.  This can be used to determine, for example,
     * the language of the text, which may be a useful hint to the filter.
     * @em Not valid in constructor, but valid when @ref convert is called.
     */
    TalkerCode* m_talkerCode;
};

#endif      // _FILTERPROC_H_
