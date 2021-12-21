#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QPluginLoader>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	ui->actionSave->setEnabled(false);
}

MainWindow::~MainWindow()
{
	delete ui;
}


void MainWindow::on_actionOpen_triggered() {
  QString filename = QFileDialog::getOpenFileName(this, tr("Open ifc file"), QString(), tr("ifc STEP file (*.ifc)"));
  if(!filename.isEmpty()) {
	  m_reader.reset(new IFCC::IFCReader(filename.toStdWString()));
	  ui->label_FileName->setText(filename);
  }
  else {
	  m_reader.reset(nullptr);
	  ui->label_FileName->setText(tr("No file"));
  }
  ui->actionSave->setEnabled(false);
}

QStringList MainWindow::statistic() const {
	QStringList text;
	text << tr("Statistic:");
	text << tr("%1 buildings.").arg(m_reader->m_site.m_buildings.size());
	for(const auto& building : m_reader->m_site.m_buildings) {
		text << tr("Building %1 with %2 storeys.").arg(QString::fromStdString(building.m_name))
				.arg(building.storeys().size());
		for(const auto& storey : building.storeys()) {
			text << tr("\tStorey %1 with %2 spaces.").arg(QString::fromStdString(storey.m_name))
					.arg(storey.spaces().size());
			for(const auto& space : storey.spaces()) {
				text << tr("\tSpace %1 with %2 space boundaries.").arg(QString::fromStdString(space.m_name+" - "+space.m_longName))
						.arg(space.spaceBoundaries().size());
			}
		}
	}
	text << "\nDatabase\n";
	text << tr("%1 materials").arg(m_reader->m_database.m_materials.size());
	for(const auto& mat : m_reader->m_database.m_materials) {
		text << tr("%1 - id %2").arg(QString::fromStdString(mat.second.m_name)).arg(mat.first);
	}
	text << tr("%1 constructions").arg(m_reader->m_database.m_constructions.size());
	for(const auto& con : m_reader->m_database.m_constructions) {
		text << tr("construnction id %1 with %2 layers").arg(con.first).arg(con.second.m_layers.size());
	}
	return text;
}

void MainWindow::on_actionRead_ifc_triggered() {
	QStringList text;
	if(m_reader != nullptr) {
		bool res = m_reader->convert();
		if(!res) {
			text << tr("Read not successful.");
			text << tr("Error:");
			text << QString::fromStdString(m_reader->m_errorText);
			text << tr("Warnings:");
			text << QString::fromStdString(m_reader->m_warningText);
			text << statistic();
		}
		else {
			text << tr("Read successful.");
			text << QString::fromStdString(m_reader->m_progressText);
			text << tr("Warnings:");
			text << QString::fromStdString(m_reader->m_warningText);
			text << statistic();
		}
		ui->textEdit->setText(text.join("\n"));
		ui->actionSave->setEnabled(true);
	}
	else {
		ui->textEdit->setText(tr("Reader not set"));
	}
}


void MainWindow::on_actionSave_triggered() {
	QFileInfo finfo(ui->label_FileName->text());
	QString fnameSuggestion = finfo.absoluteDir().absolutePath() + "/" + finfo.baseName() + ".vicus";
	QString filename = QFileDialog::getSaveFileName(
							this,
							tr("Specify SIM-VICUS project file"),
							fnameSuggestion,
							tr("SIM-VICUS project file (*.vicus);;All files (*.*)"));

	if(!filename.isEmpty()) {
		m_reader->writeXML(IBK::Path(filename.toStdString()));
	}

}
QDir directoryOf(const QString& subdir) {
	QDir dir(QApplication::applicationDirPath());
#if defined(Q_OS_WIN)
	if(dir.dirName().toLower() == "debug" || dir.dirName().toLower() == "release")
		dir.cdUp();
#elif defined(Q_OS_MAC)
	if( dir.dirName() == "MacOS") {
		dir.cdUp();
		dir.cdUp();
		dir.cdUp();
	}
#endif
	dir.cd(subdir);
	return dir;
}

void MainWindow::loadPlugins() {
	QDir pluginsDir = directoryOf("");
	for( QString filename : pluginsDir.entryList(QDir::Files)) {
		QPluginLoader loader(pluginsDir.absoluteFilePath(filename));
		if(IFCImportInterface* import = qobject_cast<IFCImportInterface*>(loader.instance())) {
			m_importer.insert(import->name(), import);
		}
	}
	if(!m_importer.empty()) {
		QMenu* importMenu = ui->menuPlugins;
		for(auto name : m_importer.keys()) {
			QIcon icon(m_importer.value(name)->icon());
			importMenu->addAction(icon, name);
		}
		connect(importMenu, SIGNAL(triggered(QAction*)), this, SLOT(runImport(QAction*)));
	}
}

void MainWindow::runImport(QAction* action)
{
	QString name = action->text();
	IFCImportInterface* exp = m_importer.value(name);
	VICUS::Project* project = new VICUS::Project;
	if( exp )
		exp->importFunct(project);
}
