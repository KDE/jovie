/***************************************************** vim:set ts=4 sw=4 sts=4:
  Model for listing Jobs, typically in a QTreeView.
  -------------------
  Copyright : (C) 2006 by Gary Cramblitt <garycramblitt@comcast.net>
  -------------------
  Original author: Gary Cramblitt <garycramblitt@comcast.net>
  Current Maintainer: Gary Cramblitt <garycramblitt@comcast.net>
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

// Qt includes.

// KDE includes.
#include "klocale.h"
#include "kdebug.h"

// KTTS includes.
#include "kspeech.h"

// JobInfoListModel includes.
#include "jobinfolistmodel.h"
#include "jobinfolistmodel.moc"


// ----------------------------------------------------------------------------

JobInfoListModel::JobInfoListModel(JobInfoList jobs, QObject *parent) :
    QAbstractListModel(parent),
    m_jobs(jobs)
{
}

int JobInfoListModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return m_jobs.count();
    else
        return 0;
}

int JobInfoListModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 7;
}

QModelIndex JobInfoListModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid())
        return createIndex(row, column, 0);
    else
        return QModelIndex();
}

QModelIndex JobInfoListModel::parent(const QModelIndex & index ) const 
{
    Q_UNUSED(index);
    return QModelIndex();
}

QVariant JobInfoListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() < 0 || index.row() >= m_jobs.count())
        return QVariant();

    if (index.column() < 0 || index.column() >= 8)
        return QVariant();

    if (role == Qt::DisplayRole)
        return dataColumn(m_jobs.at(index.row()), index.column());
    else
        return QVariant();
}

QVariant JobInfoListModel::dataColumn(const JobInfo& job, int column) const
{
    switch (column)
    {
        case 0: return job.jobNum; break;
        case 1: return job.appId; break;
        case 2: return job.talkerID; break;
        case 3: return stateToStr(job.state); break;
        case 4: return job.sentenceNum; break;
        case 5: return job.sentenceCount; break;
        case 6: return job.partNum; break;
        case 7: return job.partCount; break;
    }
    return QVariant();
}

Qt::ItemFlags JobInfoListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant JobInfoListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    switch (section)
    {
        case 0: return i18n("Job Num");
        case 1: return i18n("Owner");
        case 2: return i18n("Talker ID");
        case 3: return i18n("State");
        case 4: return i18n("Position");
        case 5: return i18n("Sentences");
        case 6: return i18n("Part Num");
        case 7: return i18n("Parts");
    };

    return QVariant();
}

bool JobInfoListModel::removeRow(int row, const QModelIndex & parent)
{
    beginRemoveRows(parent, row, row);
    m_jobs.removeAt(row);
    endRemoveRows();
    return true;
}

void JobInfoListModel::setDatastore(JobInfoList jobs)
{
    m_jobs = jobs;
    emit reset();
}

JobInfo JobInfoListModel::getRow(int row) const
{
    if (row < 0 || row >= rowCount()) return JobInfo();
    return m_jobs[row];
}

bool JobInfoListModel::appendRow(JobInfo& job)
{
    beginInsertRows(QModelIndex(), m_jobs.count(), m_jobs.count());
    m_jobs.append(job);
    endInsertRows();
    return true;
}

bool JobInfoListModel::updateRow(int row, JobInfo& job)
{
    m_jobs.replace(row, job);
    emit dataChanged(index(row, 0, QModelIndex()), index(row, columnCount()-1, QModelIndex()));
    return true;
}

bool JobInfoListModel::swap(int i, int j)
{
    m_jobs.swap(i, j);
    emit dataChanged(index(i, 0, QModelIndex()), index(j, columnCount()-1, QModelIndex()));
    return true;
}

void JobInfoListModel::clear()
{
    m_jobs.clear();
    emit reset();
}

QModelIndex JobInfoListModel::jobNumToIndex(uint jobNum)
{
    for (int row = 0; row < m_jobs.count(); ++row)
        if (getRow(row).jobNum == jobNum)
            return createIndex(row, 0);
    return QModelIndex();
}

/**
* Convert a KTTSD job state integer into a display string.
* @param state          KTTSD job state
* @return               Display string for the state.
*/
QString JobInfoListModel::stateToStr(int state) const
{
    switch( state )
    {
        case KSpeech::jsQueued: return        i18n("Queued");
        case KSpeech::jsSpeakable: return     i18n("Waiting");
        case KSpeech::jsSpeaking: return      i18n("Speaking");
        case KSpeech::jsPaused: return        i18n("Paused");
        case KSpeech::jsFinished: return      i18n("Finished");
        default: return                       i18n("Unknown");
    }
}

