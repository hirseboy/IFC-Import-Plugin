#ifndef IFCDEBUG_MAINWIDGET_H
#define IFCDEBUG_MAINWIDGET_H

#include <QWidget>

namespace Ui {
class IfcDebug_MainWidget;
}

class IfcDebug_MainWidget : public QWidget
{
	Q_OBJECT

public:
	explicit IfcDebug_MainWidget(QWidget *parent = nullptr);
	~IfcDebug_MainWidget();

private slots:
	void on_pushButtonIfcImport_clicked();

	void on_pushButtonSaveFile_clicked();

private:
	Ui::IfcDebug_MainWidget *m_ui;

	QString					 m_vicusText;
};

#endif // IFCDEBUG_MAINWIDGET_H
