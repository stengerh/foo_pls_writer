// foo_pls.cpp : Defines the exported functions for the DLL application.
//

#define _WIN32_WINNT 0x0501

#include "foobar2000/SDK/foobar2000.h"
#include "foobar2000/helpers/helpers.h"
//#include <sstream>
//#include <time.h>

DECLARE_COMPONENT_VERSION(
	"PLS playlist saver",
	"0.1.1",
	"PLS playlist saver for foobar 1.3.x\n\n"
	"Jan Christoph Terasa and Holger Stenger\n\n"
	"This components allows to save playlists in the .pls format."
	);
	
class pls : public playlist_loader {
public:	
	void open(const char * p_path, const service_ptr_t<file> & p_file,playlist_loader_callback::ptr p_callback, abort_callback & p_abort) {
		//playlist_loader::g_load_playlist_filehint(p_file,p_path,p_callback,p_abort);
		// Throw exception to force playlist_loader::g_open* to try the next instance,
		// i.e. fall back to built-in PLS loader.
		throw exception_io_unsupported_format();
	}

	const char * get_extension() {
		return "pls";
	}

	bool can_write() {
		return true;
	}

	bool is_our_content_type(const char* p_content_type) {
		return !strcmp(p_content_type,"audio/x-scpls");
	}

	bool is_associatable() {
		return true;
	}

	void write(const char * p_path, const service_ptr_t<file> & p_file,metadb_handle_list_cref p_data,abort_callback & p_abort) {
		t_size num_entries = p_data.get_size();
		const char* endline = "\n";

		const char * utf_bom = "\xEF\xBB\xBF";
		pfc::string out(utf_bom);

		out += "[Playlist]";
		out += endline;
		out += "NumberOfEntries=";
		out += pfc::string(num_entries);
		out += endline;

		for (t_size i=0; i < num_entries; i++) {
			pfc::string num = pfc::string(i+1);
			const metadb_handle_ptr item = p_data.get_item(i);

			// PATH
			pfc::string8 path8;
			if (!filesystem::g_relative_path_create(item->get_path(), p_path, path8)) {
				path8 = item->get_path();
			}
			pfc::string str_path(path8.get_ptr(), path8.get_length());
			out += "File";
			out += num;
			out += "=";
			if(str_path.startsWith(pfc::string("file://")))
				out += str_path.subString(7);
			else
				out += str_path;
			out += endline;

			// TITLE
			file_info_impl info;
			item->get_info(info);
				
			out += "Title";
			out += num;
			out += "=";
			if(info.meta_exists("TITLE")) {
				out += info.meta_get("TITLE",0);
			} else {
				out += pfc::string_filename(item->get_path());
			}
			out += endline;


			// LENGTH
			const double length = item->get_length();
			out += "Length";
			out += num;
			out += "=";
			if (length < 0.0) {
				out += "-1";
			} else {
				out += pfc::string(static_cast<int>(length));
			}
			out += endline;
		}

		out += "Version=2";
		out += endline;

		try {
			p_file->write_string_raw(out.get_ptr(), p_abort);
		}
		catch (...) {
			//try {filesystem::g_remove(p_path,p_abort);} catch(...) {}
			throw;
		}
	}
};

static playlist_loader_factory_t<pls> foo_pls;
