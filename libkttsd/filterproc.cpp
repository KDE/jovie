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

// FilterProc includes.
#include "filterproc.h"

/**
 * Constructor.
 */
KttsFilterChain::KttsFilterChain( QObject *parent, const char *name) :
    QObject(parent, name)
{
    m_inStream = 0;
    m_outStream = 0;
}

/**
 * Destructor.
 */
KttsFilterChain::~KttsFilterChain()
{
    delete m_inStream;
    delete m_outStream;
}

KttsFilterStream* KttsFilterChain::inputStream() { return m_inStream; }

KttsFilterStream* KttsFilterChain::outputStream() { return m_outStream; }

/**
 * Constructor.
 */
KttsFilterProc::KttsFilterProc( QObject *parent, const char *name, const QStringList& /*args*/) :
        QObject(parent, name), m_chain( 0 ) { }

/**
 * Destructor.
 */
KttsFilterProc::~KttsFilterProc() { }

/*virtual*/ KttsFilterProc::ConversionStatus KttsFilterProc::convert(
    const QCString& /*from*/,
    const QCString& /*to*/ )
{
    if (!m_chain) return KttsFilterProc::UsageError;
    // Default implementation connects input to output.
    m_chain->outputStream()->setDevice(m_chain->inputStream()->device());
    return KttsFilterProc::OK;
}
