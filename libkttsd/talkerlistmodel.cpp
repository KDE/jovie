/***************************************************** vim:set ts=4 sw=4 sts=4:
  Widget for listing Talkers.  Based on QTreeView.
  -------------------
  Copyright : (C) 2005 by Gary Cramblitt <garycramblitt@comcast.net>
  Copyright : (C) 2009 by Jeremy Whiting <jpwhiting@kde.org>
  -------------------
  Original author: Gary Cramblitt <garycramblitt@comcast.net>
  Current Maintainer: Jeremy Whiting <jpwhiting@kde.org>
 ******************************************************************************/

/******************************************************************************
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

// TalkerListWidget includes.
#include "talkerlistmodel.h"
#include "talkerlistmodel.moc"

// Qt includes.

// KDE includes.
#include "klocale.h"
#include "kconfig.h"
#include "kdebug.h"
#include "kconfiggroup.h"

// ----------------------------------------------------------------------------

enum Columns {
    kNameColumn = 0,
    kLanguageColumn,
    kModuleColumn,
    kVoiceTypeColumn,
    kVolumeColumn,
    kRateColumn,
    kPitchColumn,
    kColumnCount
};

TalkerListModel::TalkerListModel(TalkerCode::TalkerCodeList talkers, QObject *parent) :
    QAbstractListModel(parent),
    m_talkerCodes(talkers)
{
}

int TalkerListModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return m_talkerCodes.count();
    else
        return 0;
}

int TalkerListModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return kColumnCount;
}

QModelIndex TalkerListModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid())
        return createIndex(row, column, 0);
    else
        return QModelIndex();
}

QModelIndex TalkerListModel::parent(const QModelIndex & index ) const 
{
    Q_UNUSED(index);
    return QModelIndex();
}

QVariant TalkerListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() < 0 || index.row() >= m_talkerCodes.count())
        return QVariant();

    if (index.column() < 0 || index.column() >= kColumnCount)
        return QVariant();

    if (role == Qt::DisplayRole)
        return dataColumn(m_talkerCodes.at(index.row()), index.column());
    else
        return QVariant();
}

QVariant TalkerListModel::dataColumn(const TalkerCode& talkerCode, int column) const
{
    switch (column)
    {
        case kNameColumn:      return talkerCode.name(); break;
        case kLanguageColumn:  return TalkerCode::languageCodeToLanguage(talkerCode.language()); break;
        case kModuleColumn:    return talkerCode.outputModule(); break;
        case kVoiceTypeColumn: return TalkerCode::translatedVoiceType(talkerCode.voiceType()); break;
        case kVolumeColumn:    return talkerCode.volume(); break;
        case kRateColumn:      return talkerCode.rate(); break;
        case kPitchColumn:     return talkerCode.pitch(); break;
    }
    return QVariant();
}

Qt::ItemFlags TalkerListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant TalkerListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        switch (section)
        {
            case kNameColumn:      return i18n("Name"); break;
            case kLanguageColumn:  return i18n("Language"); break;
            case kModuleColumn:    return i18n("Synthesizer"); break;
            case kVoiceTypeColumn: return i18n("Voice Type"); break;
            case kVolumeColumn:    return i18nc("Volume of noise", "Volume"); break;
            case kRateColumn:      return i18n("Speed"); break;
            case kPitchColumn:     return i18n("Pitch"); break;
        };

    return QVariant();
}

bool TalkerListModel::removeRow(int row, const QModelIndex & parent)
{
    beginRemoveRows(parent, row, row);
    m_talkerCodes.removeAt(row);
    endRemoveRows();
    return true;
}

void TalkerListModel::setDatastore(TalkerCode::TalkerCodeList talkers)
{
    m_talkerCodes = talkers;
    emit reset();
}

TalkerCode TalkerListModel::getRow(int row) const
{
    TalkerCode code;
    if (row < 0 || row >= rowCount())
        return code;
    return m_talkerCodes.at(row);
}

bool TalkerListModel::appendRow(TalkerCode& talker)
{
    beginInsertRows(QModelIndex(), m_talkerCodes.count(), m_talkerCodes.count());
    m_talkerCodes.append(talker);
    endInsertRows();
    return true;
}

bool TalkerListModel::updateRow(int row, TalkerCode& talker)
{
    m_talkerCodes.replace(row, talker);
    emit dataChanged(index(row, 0, QModelIndex()), index(row, columnCount()-1, QModelIndex()));
    return true;
}

bool TalkerListModel::swap(int i, int j)
{
    m_talkerCodes.swap(i, j);
    emit dataChanged(index(i, 0, QModelIndex()), index(j, columnCount()-1, QModelIndex()));
    return true;
}

void TalkerListModel::clear()
{
    m_talkerCodes.clear();
    emit reset();
}

void TalkerListModel::loadTalkerCodesFromConfig(KConfig* c)
{
    // Clear the model and view.
    clear();
    // Iterate through list of the TalkerCode IDs.
    KConfigGroup config(c, "General");
    QStringList talkerIDsList = config.readEntry("TalkerIDs", QStringList());
    // kDebug() << "TalkerListModel::loadTalkerCodesFromConfig: talkerIDsList = " << talkerIDsList;
    if (!talkerIDsList.isEmpty())
    {
        QStringList::ConstIterator itEnd = talkerIDsList.constEnd();
        for (QStringList::ConstIterator it = talkerIDsList.constBegin(); it != itEnd; ++it)
        {
            QString talkerID = *it;
            kDebug() << "TalkerListWidget::loadTalkerCodes: talkerID = " << talkerID;
            KConfigGroup talkGroup(c, "Talkers");
            QString talkerCode = talkGroup.readEntry(talkerID);
            TalkerCode tc(talkerCode, true);
            kDebug() << "TalkerCodeWidget::loadTalkerCodes: talkerCode = " << talkerCode;
            //tc.setId(talkerID);
            appendRow(tc);
        }
    }
}
