#include <iostream>
#include <typeinfo>
#include <vector>
#include <string>
#include <sstream>
#include <map>

#include "vdf.hpp"
#include "utility.hpp"


int main(int argc, char* argv[])
{
	using namespace std;
	cout << "Program Version Date: " __DATE__ " " __TIME__ << endl;
	cout << "Drag and drop one or multiple VMF files to process them. Folders are not supported." << endl;
	try
	{
		if (argc <= 1)
		{ throw "No input!"; }

		vector<string> filepaths;
		filepaths.reserve(argc-1);

		for (int i = 1; i < argc; ++i)
		{ filepaths.push_back(argv[i]); }

		for (string filepath : filepaths)
		{
			cout << "Processing \"" << filepath << "\"" << endl;

			if (!string_ends_with(filepath, ".vmf"))
			{
				cout << "WARNING: File name does not end with \".vmf\"! Skipping." << endl;
				continue;
			}

			cout << "Reading File..." << endl;
			VDF vmf = VDF::parse_from_filepath(filepath);

			cout << "Editing VMF..." << endl;

			struct Pos3D
			{
				float x, y, z;
				string to_string()
				{
					ostringstream s;
					s << x << " " << y << " " << z;
					return s.str();
				}
			};

			// This map stores the classnames of every relevant entity as keys.
			// The associated position is the origin that we want this entity to have.
			map<string, Pos3D> ent_map
			{
				{"color_correction",       { 40,  0, 16} },
				{"env_fog_controller",     { -8,  0, 16} },
				{"env_soundscape",         { 32,-16, 16} },
				{"env_tonemap_controller", {  8,  0, 16} },
				{"light_environment",      {-40,  0, 16} },
				{"logic_auto",             { 24,  0, 16} },
				{"shadow_control",         {-24,  0, 16} },
				{"sky_camera",             {  0,-16, 16} },
			};

			// List of references to all entities that we are keeping in the VMF.
			vector<shared_ptr<VDF>> kept_entities;

			// Search for relevant entities and the world properties. Delete everything else.
			for (VDF::KeyValue & vmf_kv : vmf)
			{
				if (vmf_kv.key == "entity")
				{
					bool keep_entity = false;
					VDF::KeyValue * origin_kv = nullptr;
					string classname;

					auto entity_ptr = get<shared_ptr<VDF>>(vmf_kv.val);
					VDF & entity = *entity_ptr;

					for (VDF::KeyValue & ent_kv : entity)
					{
						if (ent_kv.key == "classname")
						{
							classname = get<string>(ent_kv.val);
							if (contains(ent_map, classname))
							{ keep_entity = true; }
						}
						else if (ent_kv.key == "origin")
						{
							origin_kv = &ent_kv;
						}
					}

					if (keep_entity)
					{
						bool entity_is_not_new = false;
						for (const auto & ent : kept_entities)
						{
							// "classname" is included in the ignore list because it's identical anyway.
							if (VDF::compare(entity, *ent, {"id", "origin", "classname", "editor"}, true))
							{ entity_is_not_new = true; }
						}
						if (entity_is_not_new)
						{
							vmf_kv.clear();
						}
						else
						{
							kept_entities.push_back(entity_ptr);
							if (origin_kv != nullptr)
							{
								origin_kv->val = ent_map[classname].to_string();
								ent_map[classname].z += 16.0f;
							}
						}
					}
					else
					{
						vmf_kv.clear();
					}
				}
				else if (vmf_kv.key == "world")
				{
					VDF & world = *get<shared_ptr<VDF>>(vmf_kv.val);
					for (VDF::KeyValue & world_kv : world)
					{
						if (world_kv.key == "solid")
						{ world_kv.clear(); }
					}
				}
				else
				{
					vmf_kv.clear();
				}
			}

			string output_filepath = filepath + ".env.vmf";
			cout << "Writing to \"" << output_filepath << "\"" << endl;
			vmf.serialize_to_filepath(output_filepath);
		}

		cout << "All Files Done!" << endl;

		return 0;
	}
	catch (const std::exception & e)
	{
		cerr << "OH NO! AN ERROR HAS OCCURED!" << endl;
		cerr << "TYPE: '" << type(e) << "'" << endl;
		cerr << "MESSAGE: " << e.what() << endl;
		return 1;
	}
	catch (const char * text)
	{
		cerr << "OH NO! AN ERROR HAS OCCURED!" << endl;
		cerr << "THE ERROR SAYS: " << text << endl;
		return 1;
	}
	// All other exceptions will be handled by the OS.
}
