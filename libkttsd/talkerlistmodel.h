/***************************************************** vim:set ts=4 sw=4 sts=4:
  Model for listing Talkers, typically in a QTreeView.
  -------------------
  Copyright : (C) 2005 by Gary Cramblitt <garycramblitt@comcast.net>
  -------------------
  Original author: Gary Cramblitt <garycramblitt@comcast.net>
  Current Maintainer: Gary Cramblitt <garycramblitt@comcast.net>

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
*******************************************************************************/

#ifndef TALKERLISTMODEL_H
#define TALKERLISTMODEL_H

// Qt includes.
#include <QAbstractListModel>
#include <QModelIndex>

// KDE includes.
#include <kdemacros.h>

// KTTS includes.
#include "talkercode.h"

class KConfig;

/**
 * Model for list of configured talkers.
 * Intended for use with talkerList QTreeView.
 * Each row of the displayed view corresponds to a TalkerCode in the model.
 */
class KDE_EXPORT TalkerListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    TalkerListModel(TalkerCode::TalkerCodeList talkers = TalkerCode::TalkerCodeList(), QObject *parent = 0);

    // Inherited method overrides.
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex & index ) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation,
        int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool removeRow(int row, const QModelIndex & parent = QModelIndex());

    /**
    *   The list of TalkerCodes for this model.
    */
    const TalkerCode::TalkerCodeList datastore() const { return m_talkerCodes; }
    void setDatastore(TalkerCode::TalkerCodeList talkers = TalkerCode::TalkerCodeList());
    /**
    *   Returns the TalkerCode for a specified row of the model/view.
    */
    TalkerCode getRow(int row) const;
    /**
    *   Adds a new row to the model/view containing the specified TalkerCode.
    */
    bool appendRow(TalkerCode& talker);
    /**
    *   Updates a row of the model/view with information from specified TalkerCode.
    */
    bool updateRow(int row, TalkerCode& talker);
    /**
    *   Swaps two rows of the model/view.
    */
    bool swap(int i, int j);
    /**
    *   Clears the model/view.
    */
    void clear();
    /**
    *   Returns the highest ID of all TalkerCodes.  Useful when generating a new TalkerCode.
    */
    int highestTalkerId() const { return m_highestTalkerId; }
    /**
    *   Loads the TalkerCodes into the model/view from the config file.
    *   @param config                   Pointer to KConfig object holding the config file info.
    */
    void loadTalkerCodesFromConfig(KConfig* config);

private:
    // Returns the displayable portion of the talkerCode corresponding to a column of the view.
    QVariant dataColumn(const TalkerCode& talkerCode, int column) const;
    // QList of TalkerCodes.
    TalkerCode::TalkerCodeList m_talkerCodes;
    // Highest talker ID.  Used to generate a new ID.
    int m_highestTalkerId;
};

#endif          // TALKERLISTMODEL_H
