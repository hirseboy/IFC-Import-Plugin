#include "ExportWPRead.h"
#include "ui_ExportWPRead.h"

#include <QFileDialog>

#include <IBK_Path.h>

#include <VICUS_Project.h>

ExportWPRead::ExportWPRead(QWidget *parent, VICUS::Project* project) :
	QWizardPage(parent),
	ui(new Ui::ExportWPRead),
	m_project(project)
{
	Q_ASSERT(m_project != nullptr);

	ui->setupUi(this);
	setTitle(tr("GEG Export"));
	setSubTitle(tr("Export the current project for Dämmwerk"));

	ui->pushButtonExport->setEnabled(false);

	if(m_project->m_buildings.size() == 0 || m_project->m_buildings.size() > 1) {
		ui->textEdit->setHtml(tr("The SIM-VICUS project should have exactly one building.<br>Export not possible."));
	}
	else {

		bool res = update();
	}
}


ExportWPRead::~ExportWPRead() {
	delete ui;
}

bool ExportWPRead::update() {
	bool res = false;

	std::vector<GEGRoom>	rooms;
	std::map<int,GEGConstruction>	constructions;
	int	nonValidUsageId = -1;
	int id = 0;
	int surfaceId = 1;
	m_zones.clear();

	for( auto& storey : m_project->m_buildings.front().m_buildingLevels) {
		for( auto& room : storey.m_rooms) {
			rooms.push_back(GEGRoom(id++));
			GEGRoom& currRoom = rooms.back();
			currRoom.set(room, *m_project, nonValidUsageId);

			// get surfaces
			for(const auto& surf : room.m_surfaces) {
				currRoom.m_surfaces.push_back(GEGSurface(surfaceId++));
				GEGSurface& currSurf = currRoom.m_surfaces.back();
				currSurf.m_zoneId = currRoom.m_id;
				GEGConstruction constr = currSurf.set(surf, *m_project);
				if(constr.valid() && constr.m_constructionId > -1) {
					if(constructions.find(constr.m_constructionId) == constructions.end())
						constructions[constr.m_constructionId] = constr;
				}
				else {
					if(!currSurf.m_errors.isEmpty())
						m_errors << currSurf.m_errors;
				}
			}
		}
	}

	if(ui->checkBoxCollectZones->isChecked()) {

		std::map<int,GEGZone> roomsZones;
		for(auto& room : rooms) {
			roomsZones[room.m_zoneTemplateId].m_rooms.push_back(room);
			roomsZones[room.m_zoneTemplateId].m_name = room.m_zoneTemplateName;
		}

		id = 1;
		for(auto& zoneIt : roomsZones) {
			GEGZone& zone = zoneIt.second;
			zone.m_id = id++;
			zone.update();
			m_zones.push_back(zone);
		}
	}
	else {
		id = 1;
		for(auto& room : rooms) {
			GEGZone zone(room, id++);
			m_zones.push_back(zone);
		}
	}
	res = !m_zones.empty() && m_errors.isEmpty();

	ui->pushButtonExport->setEnabled(res);
	if(!m_errors.isEmpty()) {
		QStringList messages;
		messages << tr("Errors in project found");
		messages << m_errors;
		ui->textEdit->setHtml(messages.join("<br>"));
	}
	else {
		ui->textEdit->setHtml(tr("No problems in SIM-VICUS project found."));
	}
	if(ui->checkBoxIgnoreErrors->isChecked())
		res = true;

	if(res) {
		ui->textEdit->append(tr("Export of GEG file ready."));
	}
	else {
		ui->textEdit->append(tr("Export of GEG not possible. Please correct the project."));
	}

	return res;
}

bool ExportWPRead::isComplete() const {
	return true;
}


void ExportWPRead::on_checkBoxCollectZones_clicked() {

}


void ExportWPRead::on_pushButtonExport_clicked() {

}

