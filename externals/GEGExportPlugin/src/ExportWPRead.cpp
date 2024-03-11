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

static int collectionId(const std::map<int,std::vector<int>>& collection, int roomId) {
	for(const auto& it : collection) {
		for(const auto& id : it.second) {
			if(roomId == id)
				return it.first;
		}
	}
	return -1;
}

bool ExportWPRead::update() {
	bool res = false;

	std::vector<GEGRoom>	rooms;
	int	nonValidUsageId = -1;
	int id = 0;
	int surfaceId = 1;
	m_zones.clear();

	for( auto& storey : m_project->m_buildings.front().m_buildingLevels) {
		for( auto& room : storey.m_rooms) {
			rooms.push_back(GEGRoom(id++));
			GEGRoom& currRoom = rooms.back();
			bool res = currRoom.set(room, *m_project, nonValidUsageId);
			if(!res) {
				m_warnings << currRoom.m_warnings;
			}

			// get surfaces
			for(const auto& surf : room.m_surfaces) {
				GEGSurface gegsurface(surfaceId++);
				gegsurface.m_zoneId = currRoom.m_id;
				GEGConstruction constr = gegsurface.set(surf, *m_project);
				if(constr.valid() && constr.m_constructionId > -1) {
					currRoom.m_surfaces.push_back(gegsurface);
					if(m_constructions.find(constr.m_constructionId) == m_constructions.end())
						m_constructions[constr.m_constructionId] = constr;
				}
				else {
					if(!gegsurface.m_errors.isEmpty())
						m_errors << gegsurface.m_errors;
				}
				for(const auto& subsurf : surf.subSurfaces()) {
					GEGSurface gegsurface(surfaceId++);
					gegsurface.m_zoneId = currRoom.m_id;
					GEGConstruction constr = gegsurface.set(surf, subsurf, *m_project);
					if(constr.valid() && constr.m_constructionId > -1) {
						currRoom.m_surfaces.push_back(gegsurface);
						if(m_constructions.find(constr.m_constructionId) == m_constructions.end())
							m_constructions[constr.m_constructionId] = constr;
					}
					else {
						if(!gegsurface.m_errors.isEmpty())
							m_errors << gegsurface.m_errors;
					}
				}
			}
		}
	}

	std::map<int,std::vector<int>> collectedRoomIds;
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
			if(!zone.updateHeating()) {
				m_errors << zone.m_errors;
			}
			for(const auto& room : zone.m_rooms) {
				collectedRoomIds[zone.m_id].push_back(room.m_id);
			}
			m_zones.push_back(zone);
		}
	}
	else {
		for(auto& room : rooms) {
			GEGZone zone(room, room.m_id);
			if(!zone.updateHeating()) {
				m_errors << zone.m_errors;
			}
			m_zones.push_back(zone);
		}
	}
	res = !m_zones.empty() && m_errors.isEmpty();

	// collect surfaces
	std::vector<GEGSurface> connectedSurfaces;
	for(const auto& zone : m_zones) {
		for(const auto& surf : zone.m_surfaces) {
			m_surfaces[surf.m_GEGType].push_back(surf);
			if(surf.m_GEGType == GEGSurface::GST_InnerSurface) {
				connectedSurfaces.push_back(surf);
			}
			else {
				if(surf.m_orgOtherSurfaceId != VICUS::INVALID_ID) {
					m_warnings << QString("Surface '%1' which is not a inner one, with counter surface '%2' found.").arg(surf.m_orgSurfaceId).arg(surf.m_orgOtherSurfaceId);
				}
			}
		}
	}

	// look for inner surface pairs
	std::vector<std::pair<int,int>> surfacePairs;
	std::map<int,GEGSurface> innerSurfacesById;
	for(auto surfIt = connectedSurfaces.begin(); surfIt!=connectedSurfaces.end(); surfIt++) {
		innerSurfacesById[surfIt->m_id] = *surfIt;
		for(auto surfIt2 = surfIt; surfIt2!=connectedSurfaces.end(); surfIt2++) {
			if(surfIt->m_orgSurfaceId == surfIt2->m_orgOtherSurfaceId)
				surfacePairs.push_back(std::make_pair(surfIt->m_id,surfIt2->m_id));
		}
	}

	// update connection list
	for(auto& surfIt : m_surfaces) {
		for( auto& surf : surfIt.second) {
			surf.m_otherZoneId = 0;
			auto fit = std::find_if(surfacePairs.begin(), surfacePairs.end(), [surf](const std::pair<int,int>& sp) { return surf.m_id == sp.first;} );
			if(fit != surfacePairs.end()) {
				auto fit2 = std::find_if(surfIt.second.begin(), surfIt.second.end(), [fit](GEGSurface& cs) { return fit->second == cs.m_id; });
				if(fit2 != surfIt.second.end()) {
					surf.m_otherZoneId = fit2->m_zoneId;
					fit2->m_otherZoneId = surf.m_zoneId;
				}
			}
			if(!collectedRoomIds.empty()) {
				int zoneId = collectionId(collectedRoomIds, surf.m_zoneId);
				if(zoneId == -1) {
					m_errors << QString("Room id not valid for surface %1").arg(surf.m_id);
				}
				else {
					surf.m_zoneId = zoneId;
				}
				if(surf.m_otherZoneId > 0) {
					int otherZoneId = collectionId(collectedRoomIds, surf.m_otherZoneId);
					if(otherZoneId == -1) {
						m_errors << QString("Room id not valid for surface %1").arg(surf.m_id);
					}
					else {
						surf.m_otherZoneId = otherZoneId;
					}
				}
			}
		}
	}

	// remove none of them paired surface
	for(auto& surfIt : m_surfaces) {
		for( auto it = surfIt.second.begin(); it != surfIt.second.end(); ++it) {
			const GEGSurface& surf = *it;
			auto fit = std::find_if(surfacePairs.begin(), surfacePairs.end(), [surf](const std::pair<int,int>& sp) { return surf.m_id == sp.second;} );
			if(fit != surfacePairs.end()) {
				it = surfIt.second.erase(it);
			}
		}
	}

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

void ExportWPRead::exportCsv(QString filename) {

	if(filename.indexOf(".csv") == -1)
			filename += ".csv";
	QString zoneFilename = filename;
	zoneFilename.replace(".csv", "_Import.csv");
	QString constructionFilename = filename;
	constructionFilename.replace(".csv", "_BtlImport.csv");

	QFile zoneFile(zoneFilename);
	zoneFile.open(QFile::WriteOnly);
	if(zoneFile.isOpen()) {
		QTextStream str(&zoneFile);
		str << QString::fromUtf8("*DÄMMWERK Zonen- und Hüllflächenexport, Datenfelder sind durch Semikola getrennt, Stern am Zeilenanfang = Kommentar;;;;;;;;;;\n");
		str << QString::fromUtf8("\"* Abschnitts-Marker; ZONEN, HUELLEN, BODENPLATTENMAße\";;;;;;;;;;\n");
		str << QString::fromUtf8("\"*Orientierungen; -;S;SSW;SW;SWW;W;NWW;NW;NNW;N;NNO;NO;NOO;O;SOO;SO;SSO;\";;;;;;;;;;\n");
		str << QString::fromUtf8("\"*Bauteiltypen; FAW;FF zur Außenluft;FAW Außentür;FD Dach;FD Decke nach unten;zum unbeheizten Raum;Trennwand;FF zur unbeheizten Zone;\n"
			   "Innentür gegen unbeheizte Zone;Fd Decke nach oben;Fz Decke nach unten;Kellerwand;Fußboden auf Erdreich;Kellerdecke;\";;;;;;;;;;\n");
		str << QString::fromUtf8("\"* Bodenplattenmaße; Maximal 5 Stück möglich\";;;;;;;;;;\n");
		str << QString::fromUtf8(";;;;;;;;;;\n");
		str << QString::fromUtf8("ZONEN;;;;;;;;;;\n");
		str << QString::fromUtf8("*Bezeichnung;Nummer;Ti,Soll;Ti,Soll,WE;ANGF;Ve;Vi;;;;\n");
		for(const auto& zone : m_zones) {
			str << zone.string();
		}
		str << QString::fromUtf8("*;;;;;;;;;;\n");
		str << QString::fromUtf8("HUELLEN;;;;;;;;;;\n");
		str << QString::fromUtf8("*Bezeichnung;Fläche;Orientierung;Neigung;Zone;Grenzt an Zone;Flächen-ID;U-Wert;Typ;Bauteil-ID;zu B' Nr.\n");
		str << QString::fromUtf8("oberer Abschluss;;;;;;;;;;\n");
		for(const auto& surf : m_surfaces[GEGSurface::GST_UpperBoundary]) {
			str << surf.string();
		}
		str << QString::fromUtf8("Außenbauteile;;;;;;;;;;\n");
		for(const auto& surf : m_surfaces[GEGSurface::GST_OuterSurface]) {
			str << surf.string();
		}
		str << QString::fromUtf8("Innenbauteile;;;;;;;;;;\n");
		for(const auto& surf : m_surfaces[GEGSurface::GST_InnerSurface]) {
			str << surf.string();
		}
		str << QString::fromUtf8("unterer Abschluss;;;;;;;;;;\n");
		for(const auto& surf : m_surfaces[GEGSurface::GST_LowerBoundary]) {
			str << surf.string();
		}
		str << QString::fromUtf8("Bodenplatte;;;;;;;;;;\n");
		double groundSlabArea = 0;
		for(const auto& surf : m_surfaces[GEGSurface::GST_GroundSlab]) {
			str << surf.string();
			groundSlabArea += surf.m_area;
		}
		str << QString::fromUtf8("*;;;;;;;;;;\n");
		str << QString::fromUtf8("BODENPLATTENMAße;*Marker für Bodenplattenmaße;;;;;;;;;\n");
		str << QString::fromUtf8("*Nr.;A;U;;;;;;;;\n");
		str << "1;" << QString("%1;").arg(groundSlabArea,0,'f', 0) << QString("%1").arg(std::sqrt(groundSlabArea),0,'f', 0) << ";;;;;;;;\n";
	}
	zoneFile.close();

	QFile constructionFile(constructionFilename);
	constructionFile.open(QFile::WriteOnly);
	if(constructionFile.isOpen()) {
		QTextStream str(&constructionFile);
		str << QString::fromUtf8("*** Beispieldatei für DÄMMWERK Bauteilimport\n");
		str << QString::fromUtf8("*** Kennwort \"Bauteil\" für neues Bauteil\n");
		str << QString::fromUtf8("*** BtTyp: 1-Decke gegen außen, Dachdecke, 2-Decke zum Dachraum, hinterlüftet, 3-Außenwand / Außentür, 4-AW hinterlüftet, 5-AW gegen Erdreich, 6/12-Trennwand,\n");
		str << QString::fromUtf8("***        8-Kellerdecke, 9-Fußboden zum Erdreich, 7/13/14/15-Whg-Trenndecken, 16-Decke nach unten hinterlüftet, 17-Decke aufgeständert, 20-Fenster \n");
		str << QString::fromUtf8("*** oder k.A. -> später einstellen  \n");
		str << QString::fromUtf8("*** folgende Zeilen = Baustoffparameter, Schichten von innen nach außen\n");
		str << QString::fromUtf8("*** Typ (DW) = DÄMMWERK Baustoffgruppe oder k.A. \n");
		str << QString::fromUtf8("**************************************** Beginn Struktur\n");
		str << QString::fromUtf8("* Bauteil;Beschreibung 1;BtTyp;\n");
		str << QString::fromUtf8("* Baustoffname 1;Rohdichte kg/m³;Schichtdicke m;Flächengewicht kg/m²;lambda-Wert W/(mK);R-Wert m²K/W;mü min;mü max;cspez J/(kgK);Typ (DW); \n");
		str << QString::fromUtf8("* Baustoffname 2;Rohdichte kg/m³;Schichtdicke m;Flächengewicht kg/m²;lambda-Wert W/(mK);R-Wert m²K/W;mü min;mü max;cspez J/(kgK);Typ (DW); \n");
		str << QString::fromUtf8("* Baustoffname 3;...\n");
		str << QString::fromUtf8("* Bauteil;Beschreibung 2;Bttyp;\n");
		str << QString::fromUtf8("* Baustoffname 1;Rohdichte kg/m³;Schichtdicke m;Flächengewicht kg/m²;lambda-Wert W/(mK);R-Wert m²K/W;mü min;mü max;cspez J/(kgK);Typ (DW); \n");
		str << QString::fromUtf8("* Baustoffname 2;...\n");
		str << QString::fromUtf8("* Baustoffname 3;...\n");
		str << QString::fromUtf8("*************************************** Ende Struktur\n");
		str << QString::fromUtf8("*************************************** Beginn Beispieldaten\n");
		for(const auto& constr : m_constructions) {
			str << constr.second.string() + "\n";
		}
		str << QString::fromUtf8("************************************** Ende Beispieldaten\n");
		constructionFile.close();
	}
}

bool ExportWPRead::isComplete() const {
	return true;
}


void ExportWPRead::on_checkBoxCollectZones_clicked() {
	bool res = update();
}


void ExportWPRead::on_pushButtonExport_clicked() {
	QString filename = QFileDialog::getSaveFileName(this, tr("Export filename"), QString(), tr("Dämmwerk export (*.csv)"));
	if(filename.isEmpty()) {
		return;
	}
	exportCsv(filename);
}

