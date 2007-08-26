/***************************************************** vim:set ts=4 sw=4 sts=4:
  Model for listing Jobs, typically in a QTreeView.
  -------------------
  Copyright : (C) 2006 by Gary Cramblitt <garycramblitt@comcast.net>
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

#ifndef JOBINFOLISTMODEL_H
#define JOBINFOLISTMODEL_H

// Qt includes.
#include <QAbstractListModel>
#include <QModelIndex>

// KDE includes.
#include <kdemacros.h>

// KTTS includes.

// ----------------------------------------------------------------------------

class JobInfo {
public:
    int         jobNum;
    QString     applicationName;
    int         priority;
    QString     talkerID;
    int         state;
    int         sentenceNum;
    int         sentenceCount;
    QString     appId;
};

typedef QList<JobInfo> JobInfoList;

// ----------------------------------------------------------------------------

/**
 * Model for list of speech jobs.
 * Intended for use with jobList QTreeView.
 * Each row of the displayed view corresponds to a Job in the model.
 */
class KDE_EXPORT JobInfoListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    JobInfoListModel(JobInfoList jobs = JobInfoList(), QObject *parent = 0);

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
    *   The list of Jobs for this model.
    */
    const JobInfoList datastore() const { return m_jobs; }
    void setDatastore(JobInfoList jobs = JobInfoList());
    /**
    *   Returns the JobInfo for a specified row of the model/view.
    */
    JobInfo getRow(int row) const;
    /**
    *   Adds a new row to the model/view containing the specified JobInfo.
    */
    bool appendRow(JobInfo& job);
    /**
    *   Updates a row of the model/view with information from specified JobInfo.
    */
    bool updateRow(int row, JobInfo& job);
    /**
    *   Swaps two rows of the model/view.
    */
    bool swap(int i, int j);
    /**
    *   Clears the model/view.
    */
    void clear();
    /**
    *   Returns a QModelIndex to the row corresponding to a job number.
    */
    QModelIndex jobNumToIndex(int jobNum);
    /**
    *   Convert a KTTSD job state integer into a display string.
    *   @param state          KTTSD job state
    *   @return               Display string for the state.
    */
    QString stateToStr(int state) const;
    /**
    *   Convert a KTTSD job priority into a display string.
    *   @param priority       KTTSD job priority.
    *   @return               Display string for priority.
    */
    QString priorityToStr(int priority) const;

private:
    // Returns the displayable portion of the job corresponding to a column of the view.
    QVariant dataColumn(const JobInfo& job, int column) const;
    // QList of JobInfos.
    JobInfoList m_jobs;
};

#endif // JOBINFOLISTMODEL_H
