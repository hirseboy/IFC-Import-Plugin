/*	IFC2BESTest

	blablabla
*/

#include <iostream>
#include <unordered_set>
#include <locale>
#include <codecvt>

#include <IBK_messages.h>
#include <IBK_WaitOnExit.h>
#include <IBK_StopWatch.h>
#include <IBK_Exception.h>

#include <ReaderSTEP.h>
#include <ReaderXML.h>

#include <ifcpp/IFC4/include/IfcBuildingStorey.h>
#include <ifcpp/IFC4/include/IfcGloballyUniqueId.h>
#include <ifcpp/IFC4/include/IfcLabel.h>
#include <ifcpp/IFC4/include/IfcText.h>
#include <ifcpp/IFC4/include/IfcRelAggregates.h>
#include <ifcpp/IFC4/include/IfcRelContainedInSpatialStructure.h>
#include <ifcpp/IFC4/include/IfcProject.h>
#include <ifcpp/IFC4/include/IfcSpace.h>
#include <ifcpp/model/AttributeObject.h>


class MyIfcTreeItem
{
public:
	MyIfcTreeItem() {}
	std::wstring m_name;
	std::wstring m_description;
	std::wstring m_entity_guid;
	std::string m_ifc_class_name;
	std::vector<shared_ptr<MyIfcTreeItem> > m_children;
};

shared_ptr<MyIfcTreeItem> resolveTreeItems(shared_ptr<BuildingObject> obj, std::unordered_set<int>& set_visited) {
	shared_ptr<MyIfcTreeItem> item;

	shared_ptr<IfcObjectDefinition> obj_def = dynamic_pointer_cast<IfcObjectDefinition>(obj);
	if (obj_def) 	{
		if (set_visited.find(obj_def->m_entity_id) != set_visited.end()) {
			return nullptr;
		}
		set_visited.insert(obj_def->m_entity_id);

		item = std::shared_ptr<MyIfcTreeItem>(new MyIfcTreeItem());
		item->m_ifc_class_name = obj_def->className();

		// access some attributes of IfcObjectDefinition
		if (obj_def->m_GlobalId) {
			item->m_entity_guid = obj_def->m_GlobalId->m_value;
		}

		if (obj_def->m_Name) {
			item->m_name = obj_def->m_Name->m_value;
		}

		if (obj_def->m_Description) {
			item->m_description = obj_def->m_Description->m_value;
		}

		// check if there are child elements of current IfcObjectDefinition
		// child elements can be related to current IfcObjectDefinition either by IfcRelAggregates, or IfcRelContainedInSpatialStructure, see IFC doc
		if (obj_def->m_IsDecomposedBy_inverse.size() > 0) {
			// use inverse attribute to navigate to child elements:
			std::vector<weak_ptr<IfcRelAggregates> >& vec_IsDecomposedBy = obj_def->m_IsDecomposedBy_inverse;
			for ( auto it = vec_IsDecomposedBy.begin(); it != vec_IsDecomposedBy.end(); ++it) {
				shared_ptr<IfcRelAggregates> rel_agg(*it);
				std::vector<shared_ptr<IfcObjectDefinition> >& vec_related_objects = rel_agg->m_RelatedObjects;
				for (shared_ptr<IfcObjectDefinition> child_obj_def : vec_related_objects) {
					shared_ptr<MyIfcTreeItem> child_tree_item = resolveTreeItems(child_obj_def, set_visited);
					if (child_tree_item) 					{
						item->m_children.push_back(child_tree_item);
					}
				}
			}
		}

		shared_ptr<IfcSpatialStructureElement> spatial_ele = dynamic_pointer_cast<IfcSpatialStructureElement>(obj_def);
		if (spatial_ele) {
			// use inverse attribute to navigate to child elements:
			std::vector<weak_ptr<IfcRelContainedInSpatialStructure> >& vec_contained = spatial_ele->m_ContainsElements_inverse;
			if (vec_contained.size() > 0) 			{
				for (auto it_rel_contained = vec_contained.begin(); it_rel_contained != vec_contained.end(); ++it_rel_contained) {
					shared_ptr<IfcRelContainedInSpatialStructure> rel_contained(*it_rel_contained);
					std::vector<shared_ptr<IfcProduct> >& vec_related_elements = rel_contained->m_RelatedElements;

					for (shared_ptr<IfcProduct> related_product : vec_related_elements) {
						shared_ptr<MyIfcTreeItem> child_tree_item = resolveTreeItems(related_product, set_visited);
						if (child_tree_item) {
							item->m_children.push_back(child_tree_item);
						}
					}
				}
			}
		}
	}

	return item;
}

std::string ws2s(const std::wstring& wstr) {
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(wstr);
}

int main(int argc, char * argv[]) {
	const char* FUNC_ID = "main";

#ifdef WIN32
	IBK::WaitOnExit wait;			// windows: default - wait
#endif //WIN32

	try {
		// a stopwatch to measure time needed for solver initialization
		IBK::StopWatch initWatch;

		std::wstring testFileIFC4 = L"..\\..\\data\\AC20-FZK-Haus.ifc";
		std::wstring testFileIFC3 = L"..\\..\\data\\231110AC11-FZK-Haus-IFC.ifc";

		ReaderSTEP readerStep;

		std::string errtxt;
		std::shared_ptr<BuildingModel> model(new BuildingModel);
		try {
//			readerStep.readHeader(testFileIFC3, model);
			readerStep.loadModelFromFile(testFileIFC4, model);
			std::cout << "Step model loaded\n";
			const std::map<int, shared_ptr<BuildingEntity> >& map_entities = model->getMapIfcEntities();
			std::cout << map_entities.size() << " entities in model\n";

			MyIfcTreeItem ifcTree;
			shared_ptr<IfcProject> ifc_project = model->getIfcProject();
			std::unordered_set<int> set_visited;
			shared_ptr<MyIfcTreeItem> root_item = resolveTreeItems(ifc_project, set_visited);

			int cwall = 0;
			int cwindows = 0;
			int cspace = 0;
			int cslab = 0;

			std::vector<shared_ptr<IfcSpace>> spaces;

			for (auto it : map_entities) {
				shared_ptr<BuildingEntity> entity = it.second;
				std::string name = entity->className();
				if (name == "IfcWall" || name == "IfcWallStandardCase") ++cwall;
				if (name == "IfcWindow") ++cwindows;
				if (name == "IfcSpace") {
					++cspace;
					spaces.emplace_back(std::dynamic_pointer_cast<IfcSpace>(entity));
				}
				if (name == "IfcSlab") ++cslab;
			}

			std::cout << "\n";
			for (auto sp : spaces) {
				std::vector<std::pair<std::string, shared_ptr<BuildingObject> > > atts;
				std::vector<std::pair<std::string, shared_ptr<BuildingObject> > > atts_inv;
				sp->getAttributes(atts);
				sp->getAttributesInverse(atts_inv);
				std::cout << "space: " << sp->m_Name.get() << "\n";
				std::cout << "attributes \n";
				for(auto sp_att : atts) {
					std::cout << "   " << sp_att.first << "  :  " << (sp_att.second.operator bool() ? std::string(sp_att.second->className()) : std::string("not valid")) << "\n";
				}
				std::cout << "attributes invers\n";
				for(auto sp_att_inv : atts_inv) {
					std::shared_ptr<AttributeObjectVector> att_objs = dynamic_pointer_cast<AttributeObjectVector>(sp_att_inv.second);
					int count_obj = att_objs.get() == nullptr ? 0 : att_objs->m_vec.size();
					std::cout << "   " << sp_att_inv.first << "  :  " << count_obj << "\n";
					if(count_obj > 0) {
						for(int i=0; i<count_obj; ++i) {
							const shared_ptr<BuildingObject>& sub_obj = att_objs->m_vec[i];
							std::cout << "      " << i << "  " << sub_obj->className() << "\n";
						}
					}
				}
				std::cout << "\n";
			}
			std::cout << "\n";

/*
			std::cout << cwall << " walls\n";
			std::cout << cwindows << " windows\n";
			std::cout << cspace << " spaces\n";
			std::cout << cslab << " slabs\n";

			std::cout << "resolve tree\n\n";
			std::cout << root_item->m_ifc_class_name << "name\n";
			std::cout << "has " << root_item->m_children.size() << " childs\n";

			int c0 = 0;
			for(auto it : root_item->m_children) {
				std::cout << c0++ << "   " << it->m_ifc_class_name << "  " << ws2s(it->m_name) << "  " << it->m_children.size() << " childs\n";
				int c1 = 0;
				for(auto it1 : it->m_children) {
					std::cout << c1++ << "       " << it1->m_ifc_class_name << "  " << ws2s(it1->m_name) << "  " << it1->m_children.size() << " childs\n";
					int c2 = 0;
					for(auto it2 : it1->m_children) {
						std::cout << c2++ << "          " << it2->m_ifc_class_name << "  " << ws2s(it2->m_name) << "  " << it2->m_children.size() << " childs\n";
						int c3 = 0;
						for(auto it3 : it2->m_children) {
							std::cout << c3++ << "             " << it3->m_ifc_class_name << "  " << ws2s(it3->m_name) << "  " << it3->m_children.size() << " childs\n";
							int c4 = 0;
							for(auto it4 : it3->m_children) {
								std::cout << c4++ << "                " << it4->m_ifc_class_name << "  " << ws2s(it4->m_name) << "  " << it4->m_children.size() << " childs\n";
							}
						}
					}
				}
			}
*/
		}
		catch (std::exception& e) {
			errtxt = e.what();
			std::cout << errtxt;
		}

		IBK::IBK_Message(IBK::FormatString("Time elapsed = %1 s.\n").arg(initWatch.difference()/1000.0),
						 IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	}
	catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();
		IBK::IBK_Message("Critical error occurred.", IBK::MSG_ERROR, FUNC_ID);
		return EXIT_FAILURE;
	}
	catch (std::exception& ex) {
		IBK::IBK_Message(ex.what(), IBK::MSG_ERROR, FUNC_ID);
		IBK::IBK_Message("Critical error occurred.", IBK::MSG_ERROR, FUNC_ID);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

