#include "IfcDebug_MainWidget.h"
#include "ui_IfcDebug_MainWidget.h"

#include <IFCImportPlugin.h>

#include <QFileDialog>
#include <QMessageBox>

IfcDebug_MainWidget::IfcDebug_MainWidget(QWidget *parent) :
    QWidget(parent),
	m_ui(new Ui::IfcDebug_MainWidget)
{
	m_ui->setupUi(this);

	m_ui->pushButtonSaveFile->setEnabled(false);
}


IfcDebug_MainWidget::~IfcDebug_MainWidget() {
	delete m_ui;
}


void IfcDebug_MainWidget::on_pushButtonIfcImport_clicked() {
	IFCImportPlugin plugin;
	bool succesful = plugin.import(this, m_vicusText);
	m_ui->pushButtonSaveFile->setEnabled(succesful);
}


void IfcDebug_MainWidget::on_pushButtonSaveFile_clicked() {
	QString filename = QFileDialog::getSaveFileName(
							this,
							tr("Specify SIM-VICUS project file"),
							"",
							tr("SIM-VICUS project file (*.vicus);;All files (*.*)"));

	if(filename.isEmpty()) {
		QMessageBox::information(this, "Saved vicus file", QString("Specify a valid filename").arg(filename));
		return;
	}

	std::ofstream out(filename.toStdString());
	out << m_vicusText.toStdString();
	out.close();

	QMessageBox::information(this, "Saved vicus file", QString("Vicus files hase been saved to:\n%1").arg(filename));
}

