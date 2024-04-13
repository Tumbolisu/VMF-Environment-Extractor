#include <iostream>
#include <typeinfo>
#include <vector>
#include <string>
#include <sstream>
#include <map>

// This stuff is only used for printing error messages.
#ifdef __GNUG__
#include <cstdlib>
#include <memory>
#include <cxxabi.h>
std::string demangle(const char * name)
{
	int status = -4;
	std::unique_ptr<char, void(*)(void*)> res {abi::__cxa_demangle(name, NULL, NULL, &status), std::free};
	return (status == 0) ? res.get() : name;
}
#else
// does nothing if not g++
std::string demangle(const char * name)
{
	return name;
}
#endif

template <class T>
inline std::string type(const T & t)
{
	return demangle(typeid(t).name());
}


#include "vdf.hpp"


int main(int argc, char* argv[])
{
	using namespace std;
	cout << "Program Version Date: " __DATE__ " " __TIME__ << endl;
	cout << "Drag and drop one or multiple VMF files to process them. Folders are not supported." << endl;
	try
	{
		if (argc <= 1)
			throw "No input!";

		vector<string> filepaths;
		filepaths.reserve(argc-1);

		for (int i = 1; i < argc; ++i)
			filepaths.push_back(argv[i]);

		for (string filepath : filepaths)
		{
			cout << "Processing \"" << filepath << "\"" << endl;

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

			// Search for relevant entities and the world properties. Delete everything else.
			for (VDF::KeyValue & kv : vmf)
			{
				if (kv.key == "entity")
				{
					bool keep_entity = false;
					VDF::KeyValue * origin_kv = nullptr;
					string classname;

					VDF & entity = *get<shared_ptr<VDF>>(kv.val);
					for (VDF::KeyValue & kv : entity)
					{
						if (kv.key == "classname")
						{
							classname = get<string>(kv.val);
							if (ent_map.find(classname) != ent_map.end())
							{
								keep_entity = true;
							}
						}
						else if (kv.key == "origin")
						{
							origin_kv = &kv;
						}
					}

					if (keep_entity)
					{
						if (origin_kv != nullptr)
						{
							origin_kv->val = ent_map[classname].to_string();
							ent_map[classname].z += 16.0f;
/*
							if (classname == "color_correction")
								origin_kv->val = "40 0 16";
							else if (classname == "env_fog_controller")
								origin_kv->val = "-8 0 16";
							else if (classname == "env_soundscape")
								origin_kv->val = "32 -16 16";
							else if (classname == "env_tonemap_controller")
								origin_kv->val = "8 0 16";
							else if (classname == "light_environment")
								origin_kv->val = "-40 0 16";
							else if (classname == "logic_auto")
								origin_kv->val = "24 0 16";
							else if (classname == "shadow_control")
								origin_kv->val = "-24 0 16";
							else if (classname == "sky_camera")
								origin_kv->val = "0 -16 16";
							else
								origin_kv->val = "0 0 0";
*/
						}
					}
					else
					{
						kv = VDF::KeyValue(); // delete entry
					}
				}
				else if (kv.key == "world")
				{
					VDF & world = *get<shared_ptr<VDF>>(kv.val);
					for (VDF::KeyValue & kv : world)
					{
						if (kv.key == "solid")
						{
							kv = VDF::KeyValue(); // delete entry
						}
					}
				}
				else
				{
					kv = VDF::KeyValue(); // delete entry
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
