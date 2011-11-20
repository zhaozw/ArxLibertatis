/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ARX_CRASHREPORTER_ERRORREPORTDIALOG_H
#define ARX_CRASHREPORTER_ERRORREPORTDIALOG_H

#include <QDialog>
#include <QAbstractButton>
#include <QThread>
#include <QSemaphore>

#include "errorreport.h"
#include "qhexedit/qhexedit.h"
#include "xmlhighlighter/xmlhighlighter.h"


namespace Ui {
	class ErrorReportDialog;
}

/**
 * Base task for tasks
 */
class CrashReportTask : public QThread
{
	Q_OBJECT

public:
	CrashReportTask(ErrorReport& errorReport, const QString& strDescription, int numSteps, QObject* parent = 0)
		: QThread(parent)
		, m_strDescription(strDescription)
		, m_numSteps(numSteps)
		, m_errorReport(errorReport)
	{
	}

	/**
	 * Get the description of this task.
	 * @return The description of this task.
	 */
	const QString& getDescription() const { return m_strDescription; }

	/**
	 * Get the number of steps executed  by this task.
	 * @return The number of steps executed by this task.
	 */
	int getNumSteps() const { return m_numSteps; }

	/**
	 * Get the status of this task.
	 * @return True if this task completed successfully, false otherwise.
	 * @sa getErrorString()
	 */
	bool succeeded() const { return isFinished() && m_strErrorDescription.isEmpty(); }

	/**
	 * Get the error string (available in case of a failure)
	 * @return A string detailling the error that occured in case of a failure.
	 * @sa succeeded()
	 */
	const QString& getErrorString() const { return m_strErrorDescription; }

signals:
	void taskStepStarted(const QString& taskStepDescription);
	void taskStepEnded();

protected:
	void setError(const QString& strError)
	{
		if(m_strErrorDescription.isEmpty() && !strError.isEmpty())
			m_strErrorDescription = strError;
	}

protected:
	ErrorReport& m_errorReport;

private:
	const QString m_strDescription;
	const int m_numSteps;
	QString m_strErrorDescription;
};

//!
class GatherInfoTask : public CrashReportTask
{
	Q_OBJECT

public:
	GatherInfoTask(ErrorReport& errorReport);

private:
	void run();
};

class SendReportTask : public CrashReportTask
{
	Q_OBJECT

public:
	SendReportTask(ErrorReport& errorReport);

private:
	void run();
};

class ScreenshotWidget : public QWidget 
{
	Q_OBJECT

public:
	ScreenshotWidget(QWidget *parent = 0);

	bool load(const QString& fileName);
	void setPixmap(const QPixmap& pixmap);

protected:
	void paintEvent(QPaintEvent *event);

private:
	QPixmap m_pixmap;
};

class ErrorReportFileListModel : public QAbstractListModel
{
	Q_OBJECT

public:
	ErrorReportFileListModel(ErrorReport& errorReport, QObject* parent = 0) 
		: QAbstractListModel(parent)
		, m_errorReport(errorReport)
	{
	}
	
	int rowCount(const QModelIndex & parent = QModelIndex()) const
	{
		return m_errorReport.GetAttachedFiles().count();
	}
	
	QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const
	{
		if (index.isValid() && role == Qt::DisplayRole)
		{
			QString strFilePath = m_errorReport.GetAttachedFiles().at(index.row());
			
			int lastSeparator = strFilePath.lastIndexOf(QDir::separator());
			strFilePath = strFilePath.mid(lastSeparator+1);

			return strFilePath;
		}
		else
			return QVariant();
	}

	
	QVariant headerData(int section, Qt::Orientation orientation, int role) const
	{
		if (role != Qt::DisplayRole)
			return QVariant();

		if (orientation == Qt::Horizontal)
			return QString("Column %1").arg(section);
		else
			return QString("Row %1").arg(section);
	}
	
private:
	ErrorReport& m_errorReport;
};

class ErrorReportDialog : public QDialog
{
	Q_OBJECT

public:
	enum DialogPane
	{
		Pane_Progress,
		Pane_FillInfo,
		Pane_ExitSuccess,
		Pane_ExitError
	};

public:
	explicit ErrorReportDialog(ErrorReport& errorReport, QWidget *parent = 0);
	~ErrorReportDialog();

	void SetCurrentPane(DialogPane paneId);

	// Progress pane
	void ResetProgressBar(int maxNumber);
	void IncrementProgress();
	void SetProgressText(const QString& strProgress);

	// Exit pane
	void SetExitText(const QString& strExit);

public slots:
	void onTaskStepStarted(const QString& taskStepDescription);
	void onTaskStepEnded();
	void onTaskCompleted();
	void onTabChanged(int index);

private slots:
	void onSendReport();
	void onShowFileContent(const QItemSelection& current, const QItemSelection& previous);

private:
	void startTask(CrashReportTask* pTask, int nextPane);

	Ui::ErrorReportDialog *ui;
	QHexEdit m_fileViewHex;
	ScreenshotWidget m_fileViewImage;
	XmlHighlighter* m_pXmlHighlighter;

	CrashReportTask* m_pCurrentTask;
	int m_nextPane;

	ErrorReport& m_errorReport;
};

#endif // ARX_CRASHREPORTER_ERRORREPORTDIALOG_H