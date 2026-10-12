/*
 * 	Defindes the user-interaction via shell prompt and script file.
 * 	
 * 	Would be nice to extend with readline capabilities.
 *
 */
#include "interaction.h"
#include "config.h"

#include "cmdline.h"

#include "libgi/timer.h"
#include "libgi/scene.h"
#include "libgi/algorithm.h"
#include "libgi/framebuffer.h"
#include "libgi/context.h"


#include "rt/cpu/seq.h"
#include "gi/primary-hit.h"

#ifdef HAVE_GL
#include "preview.h"
#endif


#ifdef HAVE_LIBEMBREE3
#include "rt/cpu/embree.h"
#endif

#include "libgi/timer.h"

#include "libgi/global-context.h"

#include <iostream>
#include <sstream>
#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <glm/glm.hpp>
#if GLM_VERSION < 997
#define GLM_ENABLE_EXPERIMENTAL
#endif
#include <glm/gtx/string_cast.hpp>

inline std::istream& operator>>(std::istream &in, vec3 &x) {
	in >> x.x >> x.y >> x.z;
	return in;
}

using namespace glm;
using namespace std;

const char *prompt = "rtgi > ";

#define ifcmd(c) if (command==c)
#define error_no_return(x) { cerr << "command " << uc.cmdid << " (" << command << "): " << x << endl; }
#define error(x) { error_no_return(x);  return; }
#define check_in(x) { if (in.bad() || in.fail()) error(x); }
#define check_in_complete(x) { if (in.bad() || in.fail() || !in.eof()) error(x); }

void run(gi_algorithm *algo);
#ifdef HAVE_GL
void run_sample(gi_algorithm *algo);
#endif

/* 
 *  The Read Eval Print Loop is run in a thread and queues all commands in order
 *  (potentially blocking on cin) in the command_queue for evaluation in the main thread.
 *
 *  Reading from cin would be much improved by readline(3) support.
 *
 */

void repl(istream &infile) {
	while (!infile.eof()) {
		if (&infile == &cin)
			cout << prompt << flush;
		string line, command;
		getline(infile, line);
		queue_command(line);
		if (line == "exit" || line == "quit") // this is a hack
			return;
	}
	cout << endl;
}

void run_repls() {
	if (cmdline.script != "") {
		ifstream script(cmdline.script);
		if(script.fail()){
			cerr << "Script file not found" << endl;
			return;
		}
		repl(script);
	}
	if (cmdline.interact)
		repl(cin);

#ifdef HAVE_GL
	if (!preview_window) queue_command("exit");
#else 
	queue_command("exit");
#endif
}

/* 
 *  Commands can be queued in from anywhere via queue_command().
 *  The commands are taken out of the queue by RTGI's main thread (it is woken up
 *  when new commands are entered into the queue) and then evaluated via eval().
 *
 */

static mutex command_queue_mutex;
static condition_variable queue_ready;
static list<string> command_queue;
static bool expecting_commands = true; // only set via eval, called only via process_command_queue

void eval(const std::string &command);

void queue_command(const std::string &command, enqueue_mode mode) {
	unique_lock lock(command_queue_mutex);
	if (mode == remove_prev_same_commands)
		command_queue.remove(command);
	command_queue.push_back(command);
	lock.unlock();
	queue_ready.notify_one();
}

void process_command_queue() {
	while (expecting_commands) {
		string command = "";
		unique_lock lock(command_queue_mutex);
		if (command_queue.empty())
			queue_ready.wait(lock); // may wake up spuriously
		if (!command_queue.empty()) {
			command = command_queue.front();
			command_queue.pop_front();
		}
		lock.unlock();
		eval(command);
	}
}

/*  
 *  Command evaluation is just a big switch/case
 *
 */

//! Keep track of when the user changed important values we have to know about in other places.
struct repl_update_checks {
	unsigned cmdid = 0,
			 scene_touched_at = 0,
			 tracer_touched_at = 0,
			 accel_touched_at = 0;
	bool valid_platform = true; // even true for "no platform", but false when a platform was requested but is not available.
};
static repl_update_checks uc;
static list<string> command_history;

bool platform_and_algo_aligned() {
	// This is used in a version of this code that supports different computing platfomrs
	return true;
}

void eval(const std::string &line) {
	if (command_history.back() != line)
		command_history.push_back(line);

	istringstream in(line);
	uc.cmdid++;
	
	scene &scene = rc->scene;
	static material *selected_mat = nullptr;
	static bool cam_has_pos = false,
				cam_has_dir = false,
				cam_has_up = false,
				scene_up_set = false;

	string command;
	in >> command;
	vec3 tmp;
	static bool skip = false;
	if (skip) {
		if (line == "}")
			skip = false;
		else if (cmdline.verbose)
			cerr << "Warning: Skipping command '" << line << "'" << endl;
		return;
	}
	
	ifcmd("history") {
		command_history.pop_back();
		for (auto &x : command_history)
			cout << x << endl;
	}
	else ifcmd("quit")
		expecting_commands = false;
	else ifcmd("exit")
		expecting_commands = false;
	else ifcmd("with") {
		bool newval = false;
		string what, rest;
		in >> what;
		if (what == "gl") {
#ifndef HAVE_GL
			newval = true;
#endif
		}
		else if (what == "cuda") {
#ifndef HAVE_CUDA
			newval = true;
#endif
		}
		else if (what == "optix") {
#ifndef HAVE_OPTIX
			newval = true;
#endif
		}
		else
			error("Syntax error: expected one of gl, cuda, optix");
		in >> rest;
		check_in_complete("Syntax error, expecting only {");
		if (rest != "{")
			error("Syntax error, expecting {");
		skip = newval;
		if (skip)
			cout << "Skipping a block due to missing " << what << endl;
	}
	else ifcmd("}") {
		check_in_complete("Syntax error: } must be on an otherwise empty line");
	}
	else ifcmd("at") {
		in.get();
		if (in.eof()) {
			cout << scene.camera.pos << endl;
			return;
		}
		in >> tmp;
		check_in_complete("Syntax error, requires 3 numerical components");
		scene.camera.pos = tmp;
		cam_has_pos = true;
	}
	else ifcmd("look") {
		in.get();
		if (in.eof()) {
			cout << scene.camera.dir << endl;
			return;
		}
		in >> tmp;
		check_in_complete("Syntax error, requires 3 numerical components");
		scene.camera.dir = tmp;
		cam_has_dir = true;
	}
	else ifcmd("up") {
		in.get();
		if (in.eof() && scene_up_set) {
			cout << scene.up << endl;
			return;
		}
		if (scene_up_set)
			error("Cannot set scene up vector twice, did you mean camup?");
		in >> tmp;
		check_in_complete("Syntax error, requires 3 numerical components");
		scene_up_set = true;
		scene.up = tmp;
		if (!cam_has_up) {
			scene.camera.up = scene.up;
			cam_has_up = true;
		}
	}
	else ifcmd("camup") {
		in.get();
		if (in.eof()) {
			cout << scene.camera.up << endl;
			return;
		}
		in >> tmp;
		check_in_complete("Syntax error, requires 3 numerical components");
		scene.camera.up = tmp;
		cam_has_up = true;
	}
	else ifcmd("load") {
		string file, name;
		in >> file;
		if (!in.eof())
			in >> name;
		check_in_complete("Syntax error, requires a file name (no spaces, sorry) and (optionally) a name");
		scene.add(file, name);
		uc.scene_touched_at = uc.cmdid;
	}
	else ifcmd("resolution") {
		int w, h;
		in >> w >> h;
		check_in_complete("Syntax error, requires 2 integral values");
		rc->change_resolution(w, h);
	}
	else ifcmd("algo") {
		string name;
		in >> name;
		gi_algorithm *a = nullptr;
		if (name == "primary")      a = new primary_hit_display;
		else error("There is no gi algorithm called '" << name << "'");
		if (a) {
			delete rc->algo;
			rc->algo = a;
		}
	}
	else ifcmd("outfile") {
		string name;
		in >> name;
		check_in_complete("Syntax error, only accepts a single file name (no spaces, sorry)");
		cmdline.outfile = name;
	}
// 	else ifcmd("bookmark") {
// 		
// 	}
	else ifcmd("raytracer") {
		string name;
		in >> name;
		if (name == "seq") scene.use(new seq_tri_is);
		else error("There is no ray tracer called '" << name << "'");
		uc.tracer_touched_at = uc.cmdid;
	}
	else ifcmd("commit") {
		if (!uc.valid_platform)
			error("Invalid platform");
		if (scene.vertices.empty())
			error("There is no scene data to work with");
		if (rc->algo)
			rc->algo->data_reset_required = true;
			if (!scene.rt)
				error("There is no ray traversal scheme to commit the scene data to");
			scene.rt->build(&scene);
		uc.accel_touched_at = uc.cmdid;
	}
	else ifcmd("sppx") {
		int sppx;
		in >> sppx;
		check_in_complete("Syntax error, requires exactly one positive integral value");
		rc->sppx = sppx;
	}
	else ifcmd("preview-offset") {
		int offset;
		in >> offset;
		check_in_complete("Syntax error, requires exactly one positive integral value");
		if (offset <= 0)
			error("Thge preview offset needs to be larger than zero");
		rc->preview_offset = offset;
	}
	else ifcmd("run") {
		if (!uc.valid_platform)
			error("Invalid platform");
		if (!platform_and_algo_aligned())
			error("Incompatible algorithm form platform");
		if (uc.scene_touched_at == 0 || uc.tracer_touched_at == 0 || uc.accel_touched_at == 0 || rc->algo == nullptr)
			error("We have to have a scene loaded, a ray tracer set, an acceleration structure built and an algorithm set prior to running");
		if (uc.accel_touched_at < uc.tracer_touched_at)
			error("The current tracer does (might?) not have an up-to-date acceleration structure");
		if (uc.accel_touched_at < uc.scene_touched_at)
			error("The current acceleration structure is out-dated");
#ifdef HAVE_GL
		if(preview_window)
			run_sample(rc->algo);
		else
#endif
			run(rc->algo);
	}
	else ifcmd("mesh") {
		string name, cmd;
		in >> name;
		if (in.eof() && name == "list") {
			for (auto &obj : scene.objects) cout << obj.name << endl;
			return;
		}
		error("Meshes can only be listed in this version");
	}
	else ifcmd("material") {
		string cmd;
		in >> cmd;
		if (in.eof() && cmd == "list") {
			for (auto &mtl : scene.materials) cout << mtl.name << endl;
			return;
		}
		check_in("Syntax error, requires material name, command and subsequent arguments");
		command = cmd;
		ifcmd("select") {
			string name;
			in >> name;
			check_in_complete("Only a single string (no whitespace) accepted");
			material *m = nullptr; for (auto &mtl : scene.materials) if (mtl.name == name) { m = &mtl; break; }
			if (!m) error("No material called '" << name << "'");
			selected_mat = m;
			return;
		}
		if (!selected_mat)
			error("No material selected");
		ifcmd("albedo") {
			in >> tmp;
			check_in_complete("Expects a color triplet");
			selected_mat->albedo = tmp;
		}
		else ifcmd("emissive") {
			in >> tmp;
			check_in_complete("Expects a color triplet");
			selected_mat->emissive = tmp;
		}
		else ifcmd("roughness") {
			in >> tmp.x;
			check_in_complete("Expects a floating point value");
			selected_mat->roughness = tmp.x;
		}
		else ifcmd("ior") {
			in >> tmp.x;
			check_in_complete("Expects a floating point value");
			selected_mat->ior = tmp.x;
		}
		else ifcmd("texture") {
			in >> cmd;
			check_in_complete("Expected a single (no whitespace) string value");
			// we keep the textures around as other materials might still use them.
			// they will be cleaned up by ~scene
			if (cmd == "drop")
				selected_mat->albedo_tex = nullptr;
			else {
				texture2d<vec4> *tex = load_image4f(cmd);
				if (tex)
					selected_mat->albedo_tex = tex;
			}
		}
		else ifcmd("show") {
			check_in_complete("Does not take further arguments");
			cout << "albedo     " << selected_mat->albedo << endl;
			cout << "albedo-tex " << (selected_mat->albedo_tex ? selected_mat->albedo_tex->path.string() : string("none")) << endl;
			cout << "emissive   " << selected_mat->emissive << endl;
			cout << "roughness  " << selected_mat->roughness << endl;
			cout << "ior        " << selected_mat->ior << endl;
		}
		else error("Unknown subcommand");
	}
	else ifcmd("omp") {
		string sub;
		in >> sub;
		if (sub == "off")
			omp_set_num_threads(1);
		else if (sub == "on")
			omp_set_num_threads(omp_get_max_threads());
		else
			error("Syntax error: expected 'on' or 'off'");
	}
	else ifcmd("stats") {
		string sub;
		in >> sub;
		if (sub == "clear")
			stats_timer.clear();
		else if (sub == "print")
			stats_timer.print();
		else
			error("No such stats subcommand");
	}
	else ifcmd("echo") {
		string text;
		getline(in, text);
		cout << text << endl;
	}
	else if (command == "") ;
	else if (command[0] == '#') ;
	else if (command.substr(0,2) == "//") ;
	else if (rc->algo && platform_and_algo_aligned() && rc->algo->interprete(command, in)) ;
	else if (scene.rt && scene.rt->interprete(command, in)) ;
	else {
		error("Unknown command");
	}
}
