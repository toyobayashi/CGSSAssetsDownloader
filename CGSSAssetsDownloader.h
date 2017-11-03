#pragma once

#include "stdlib.h"
#include "download.h"
#include "sqlite3.h"
#include "lz4.h"
#include "clHCA.h"
#include <string>
#include <fstream>
#include <Windows.h>

// #define WIN32_LEAN_AND_MEAN
using namespace std;

class Downloader {
private:
	string res_ver;
	string type;
public:
	static int current;
	static int max;
	static int exists;
	static int copy;
	static int mp3;
	static int auto_update;
	static string update_list;

	Downloader(string, string);
	void check_manifest();
	void download_manifest();
	void download_asset();
	void download_single(string);
	void set_type(string);
};

int Downloader::current = 0;
int Downloader::max = 0;
int Downloader::exists = 0;
int Downloader::copy = 0;
int Downloader::mp3 = 0;
int Downloader::auto_update = 0;
string Downloader::update_list = "";

int string_index_of(char* arr[], char* str, int length) {
	for (int i = 0; i < length; i++) {
		if (strcmp(arr[i], str) == 0) {
			return i;
		}
	}
	return -1;
}

void exec_sync(string cmd) {
	system(cmd.c_str());
}

void is_db_ok(int &r, char *&errmsg) {
	if (r != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", errmsg);
		sqlite3_free(errmsg);
		exit(0);
	}
}

string dir_name(){
	char exeFullPath[MAX_PATH];
	string strPath = "";

	GetModuleFileNameA(NULL, exeFullPath, MAX_PATH);
	strPath = (string)exeFullPath;
	int pos = strPath.find_last_of('\\', strPath.length());
	return strPath.substr(0, pos);
}

void hcadec(string hcafile) {
	unsigned int count = 0;
	char *filenameOut = NULL;
	float volume = 1;
	unsigned int ciphKey1 = 0xF27E3B22;
	unsigned int ciphKey2 = 0x00003657;
	int mode = 16;
	int loop = 0;

	char path[MAX_PATH];
	if (!(filenameOut&&filenameOut[0])) {
		strcpy_s(path, sizeof(path), hcafile.c_str());
		char *d1 = strrchr(path, '\\');
		char *d2 = strrchr(path, '/');
		char *e = strrchr(path, '.');
		if (e&&d1<e&&d2<e)*e = '\0';
		strcat_s(path, sizeof(path), ".wav");
		filenameOut = path;
	}

	printf("Decoding %s...\n", hcafile.c_str());
	clHCA hca(ciphKey1, ciphKey2);
	if (!hca.DecodeToWavefile(hcafile.c_str(), filenameOut, volume, mode, loop)) {
		printf("Error: Decoding failed.\n");
	}
}

static int exist(void *data, int argc, char **argv, char **azColName) {
	Downloader::exists++;
	return 0;
}

static int sum_number(void *data, int argc, char **argv, char **azColName) {
	string name(argv[1]);
	string type = (char*)data;

	fstream _file;
	string fileName;
	if (type == "bgm" || type == "live") {
		if (Downloader::mp3 != 0) {
			fileName = type + "\\mp3\\" + name + ".mp3";
		}
		else {
			fileName = type + "\\wav\\" + name + ".wav";
		}
	}
	else if (type == "card" || type == "icon") {
		fileName = type + "\\" + name + ".unity3d";
	}
	else if (type == "score") {
		fileName = type + "\\" + name + ".bdb";
	}

	_file.open(fileName.c_str(), ios::in);

	if (!_file) {
		Downloader::max++;
	}
	_file.close();
	return 0;
}

static int get_asset(void *data, int argc, char **argv, char **azColName) {
	string url(argv[0]);
	string name(argv[1]);

	fstream _file;
	string fileName;

	if (strcmp((char*)data, "bgm") == 0) {
		if (Downloader::mp3 != 0) {
			fileName = "bgm\\mp3\\" + name + ".mp3";
		}
		else {
			fileName = "bgm\\wav\\" + name + ".wav";
		}
	}
	else if (strcmp((char*)data, "live") == 0) {
		if (Downloader::mp3 != 0) {
			fileName = "live\\mp3\\" + name + ".mp3";
		}
		else {
			fileName = "live\\wav\\" + name + ".wav";
		}
	}
	else if (strcmp((char*)data, "card") == 0) {
		fileName = "card\\" + name + ".unity3d";
	}
	else if (strcmp((char*)data, "icon") == 0) {
		fileName = "icon\\" + name + ".unity3d";
	}
	else if (strcmp((char*)data, "score") == 0) {
		fileName = "score\\" + name + ".bdb";
	}
	else {
		fileName = "";
	}

	_file.open(fileName.c_str(), ios::in);

	if (!_file) {
		Downloader::update_list += name;
		Downloader::update_list += "\n";
		exec_sync("cls");
		printf("Downloading: %s\n\n", name.c_str());
		progress(Downloader::current, Downloader::max);
		printf("\n\n");

		if (strcmp((char*)data, "bgm") == 0) {
			//exec_sync("tool\\wget\\wget -c " + url + " -O " + "./bgm/" + name + ".acb");
			download(url, "./bgm/" + name + ".acb");
			exec_sync("tool\\AcbUnzip\\AcbUnzip.exe bgm\\" + name + ".acb");
			hcadec("bgm\\_acb_" + name + ".acb\\" + name + ".hca");
			exec_sync("move bgm\\_acb_" + name + ".acb\\" + name + ".wav bgm\\");
			exec_sync("rd bgm\\_acb_" + name + ".acb /s /q");
			exec_sync("del bgm\\" + name + ".acb");
			if (Downloader::mp3 != 0) {
				exec_sync("tool\\ffmpeg\\ffmpeg.exe -i bgm\\" + name + ".wav bgm\\mp3\\" + name + ".mp3 -v quiet");
				exec_sync("del bgm\\" + name + ".wav");
			}
			else {
				exec_sync("move bgm\\" + name + ".wav bgm\\wav\\");
			}
			if (Downloader::copy != 0) {
				if (Downloader::mp3 != 0) {
					exec_sync("copy bgm\\mp3\\" + name + ".mp3 dl\\");
				}
				else {
					exec_sync("copy bgm\\wav\\" + name + ".wav dl\\");
				}
			}
		}
		else if (strcmp((char*)data, "live") == 0) {
			//exec_sync("tool\\wget\\wget -c " + url + " -O " + "./live/" + name + ".acb");
			download(url, "./live/" + name + ".acb");
			exec_sync("tool\\AcbUnzip\\AcbUnzip.exe live\\" + name + ".acb");
			hcadec("live\\_acb_" + name + ".acb\\" + name + ".hca");
			exec_sync("move live\\_acb_" + name + ".acb\\" + name + ".wav live\\");
			exec_sync("rd live\\_acb_" + name + ".acb /s /q");
			exec_sync("del live\\" + name + ".acb");
			if (Downloader::mp3 != 0) {
				exec_sync("tool\\ffmpeg\\ffmpeg.exe -i live\\" + name + ".wav live\\mp3\\" + name + ".mp3 -v quiet");
				exec_sync("del live\\" + name + ".wav");
			}
			else {
				exec_sync("move live\\" + name + ".wav live\\wav\\");
			}
			if (Downloader::copy != 0) {
				if (Downloader::mp3 != 0) {
					exec_sync("copy live\\mp3\\" + name + ".mp3 dl\\");
				}
				else {
					exec_sync("copy live\\wav\\" + name + ".wav dl\\");
				}
			}
		}
		else if (strcmp((char*)data, "card") == 0) {
			//exec_sync("tool\\wget\\wget -c " + url + " -O " + "./card/" + name);
			download(url, "./card/" + name + ".");
			lz4dec("card\\" + name, "unity3d");
			exec_sync("del card\\" + name);
			if (Downloader::copy != 0) {
				exec_sync("copy card\\" + name + ".unity3d dl\\");
			}
		}
		else if (strcmp((char*)data, "icon") == 0) {
			//exec_sync("tool\\wget\\wget -c " + url + " -O " + "./icon/" + name);
			download(url, "./icon/" + name + ".");
			lz4dec("icon\\" + name, "unity3d");
			exec_sync("del icon\\" + name);
			if (Downloader::copy != 0) {
				exec_sync("copy icon\\" + name + ".unity3d dl\\");
			}
		}
		else if (strcmp((char*)data, "score") == 0) {
			//exec_sync("tool\\wget\\wget -c " + url + " -O " + "./score/" + name);
			download(url, "./score/" + name + ".");
			lz4dec("score\\" + name, "bdb");
			exec_sync("del score\\" + name);
			if (Downloader::copy != 0) {
				exec_sync("copy score\\" + name + ".bdb dl\\");
			}

		}
		Downloader::current++;
	}
	_file.close();
	return 0;
}

static int get_single(void *data, int argc, char **argv, char **azColName) {
	string url(argv[0]);
	string name(argv[1]);

	fstream _file;
	string fileName;

	if (strcmp((char*)data, "acb") == 0) {
		// fileName = "dl\\" + name + ".acb";
		if (Downloader::mp3 != 0) {
			fileName = "dl\\" + name + ".mp3";
		}
		else {
			fileName = "dl\\" + name + ".wav";
		}
	}
	else if (strcmp((char*)data, "unity3d") == 0) {
		fileName = "dl\\" + name + ".unity3d";
	}
	else if (strcmp((char*)data, "bdb") == 0) {
		fileName = "dl\\" + name + ".bdb";
	}
	else if (strcmp((char*)data, "mdb") == 0) {
		fileName = "dl\\" + name + ".mdb";
	}
	else {
		fileName = "";
	}

	_file.open(fileName.c_str(), ios::in);

	if (!_file) {
		printf(name.c_str());
		printf("\n\n");
		//exec_sync("tool\\wget\\wget -c " + url + " -O " + "./dl/" + name);
		download(url, "./dl/" + name + ".");

		if (strcmp((char*)data, "acb") == 0) {
			exec_sync("ren dl\\" + name + ". " + name + ".acb");
			exec_sync("tool\\AcbUnzip\\AcbUnzip.exe dl\\" + name + ".acb");

			exec_sync("dir /a-d /b dl\\_acb_" + name + ".acb\\*.hca>dl\\_acb_" + name + ".acb\\hcafiles.txt");

			string hcaFile[300];
			int i = 0;
			ifstream infile;
			infile.open("dl\\_acb_" + name + ".acb\\hcafiles.txt", ios::in);
			while (!infile.eof()) {
				getline(infile, hcaFile[i], '\n');
				i++;
			}
			infile.close();

			for (int x = 0; x < i - 1; x++) {
				hcadec("dl\\_acb_" + name + ".acb\\" + hcaFile[x]);
				string fn = hcaFile[x].substr(0, hcaFile[x].find_last_of("."));
				if (Downloader::mp3 != 0) {
					exec_sync("tool\\ffmpeg\\ffmpeg.exe -i dl\\_acb_" + name + ".acb\\" + fn + ".wav dl\\" + fn + ".mp3 -v quiet");
				}
				else {
					exec_sync("move dl\\_acb_" + name + ".acb\\" + fn + ".wav dl\\");
				}
			}
			exec_sync("rd dl\\_acb_" + name + ".acb /s /q");
			exec_sync("del dl\\" + name + ".acb");
			/* hcadec("dl\\_acb_" + name + ".acb\\" + name + ".hca");
			exec_sync("move dl\\_acb_" + name + ".acb\\" + name + ".wav dl\\");
			exec_sync("rd dl\\_acb_" + name + ".acb /s /q"); 
			exec_sync("del dl\\" + name + ".acb");
			if (Downloader::mp3 != 0) {
				exec_sync("tool\\ffmpeg\\ffmpeg.exe -i dl\\" + name + ".wav dl\\" + name + ".mp3 -v quiet");
				exec_sync("del dl\\" + name + ".wav");
			}*/
		}
		else if (strcmp((char*)data, "unity3d") == 0) {
			lz4dec("dl\\" + name, "unity3d");
			exec_sync("del dl\\" + name);
		}
		else if (strcmp((char*)data, "bdb") == 0) {
			lz4dec("dl\\" + name, "bdb");
			exec_sync("del dl\\" + name);
		}
		else if (strcmp((char*)data, "mdb") == 0) {
			lz4dec("dl\\" + name, "mdb");
			exec_sync("del dl\\" + name);
		}
		exec_sync("cls");
		printf("%s Completed.\n\n", name.c_str());
	}
	else {
		exec_sync("cls");
		printf("File exists.\n\n");
	}
	_file.close();
	return 0;
}

void read_database(sqlite3 *db, char *sql, const char* data, char *zErrMsg, int rc, string type) {
	printf("Reading database...\n");
	// Downloader::update_list = "";
	rc = sqlite3_exec(db, sql, sum_number, (void*)data, &zErrMsg);
	rc = sqlite3_exec(db, sql, get_asset, (void*)data, &zErrMsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		exit(0);
	}
	else {
		ofstream log_txt;
		exec_sync("if not exist \"log\" md log");
		exec_sync("cls");
		printf("%d/%d Completed.\n\n", Downloader::current, Downloader::max);
		string head = "";
		if (Downloader::auto_update == 0) {
			log_txt.open("log\\" + type + ".txt");
			log_txt << Downloader::update_list;
			log_txt.close();
			head = "===========  Update " + type + "  ===========\n\n";
		}
		else {
			log_txt.open("log\\log.txt");
			log_txt << Downloader::update_list;
			log_txt.close();
			head = "=============  Update  =============\n\n";
		}
		printf(head.c_str());
		printf(Downloader::update_list.c_str());
		printf("\n====================================\n");
	}
}