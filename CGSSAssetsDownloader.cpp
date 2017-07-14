#include "CGSSAssetsDownloader.h"

string update_list = "";

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
		fileName = type + "\\" + name + ".wav";
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
		fileName = "bgm\\" + name + ".wav";
	}
	else if (strcmp((char*)data, "live") == 0) {
		fileName = "live\\" + name + ".wav";
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
		update_list += name;
		update_list += "\n";
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
			if (Downloader::copy != 0) {
				exec_sync("copy bgm\\" + name + ".wav dl\\");
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
			if (Downloader::copy != 0) {
				exec_sync("copy live\\" + name + ".wav dl\\");
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
		fileName = "dl\\" + name + ".acb";
	}
	else if (strcmp((char*)data, "unity3d") == 0) {
		fileName = "live\\" + name + ".unity3d";
	}
	else if (strcmp((char*)data, "bdb") == 0) {
		fileName = "card\\" + name + ".bdb";
	}
	else if (strcmp((char*)data, "mdb") == 0) {
		fileName = "icon\\" + name + ".mdb";
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
			exec_sync("ren dl\\" + name + " " + name + ".acb");
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
	rc = sqlite3_exec(db, sql, sum_number, (void*)data, &zErrMsg);
	rc = sqlite3_exec(db, sql, get_asset, (void*)data, &zErrMsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		exit(0);
	}
	else {
		ofstream bgm_txt;
		exec_sync("if not exist \"log\" md log");
		bgm_txt.open("log\\" + type + ".txt");
		bgm_txt << update_list;
		bgm_txt.close();
		exec_sync("cls");
		printf("%d/%d Completed.\n\n", Downloader::current, Downloader::max);
		string head = "===========  Update " + type + "  ===========\n\n";
		printf(head.c_str());
		printf(update_list.c_str());
		printf("\n====================================");
	}
}

Downloader::Downloader(string v, string t) {
	res_ver = v;
	type = t;
}

void Downloader::check_manifest() {
	fstream _file;
	string fileName = "data\\manifest_" + res_ver + ".db";
	_file.open(fileName.c_str(), ios::in);

	if (!_file) {
		printf("Start download manifest.\n");
		download_manifest();
	}
	else {
		printf("Manifest file exists.\n");
	}
}

void Downloader::download_manifest() {
	exec_sync("if not exist \"data\" md data");
	//exec_sync("tool\\wget\\wget -c -O data\\manifest_" + res_ver + " http://storage.game.starlight-stage.jp/dl/" + res_ver + "/manifests/Android_AHigh_SHigh");
	download("http://storage.game.starlight-stage.jp/dl/" + res_ver + "/manifests/Android_AHigh_SHigh", "./data/manifest_" + res_ver + ".");

	string lz4file = "data\\manifest_" + res_ver;
	long size = get_file_size(lz4file.c_str());
	if (size < 1) {
		printf("Failed.\n");
		exec_sync("del data\\manifest_" + res_ver + " /f /s /q");
		exit(0);
	}
	lz4dec(lz4file, "db");
	exec_sync("del data\\manifest_" + res_ver);
	printf("Successfully download manifest.\n");
}

void Downloader::download_asset() {
	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	char *sql;
	const char* data = type.c_str();

	string sqlfile = "data\\manifest_" + res_ver + ".db";
	rc = sqlite3_open(sqlfile.c_str(), &db);
	if (rc) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		exit(0);
	}
	else {
		fprintf(stderr, "Successfully open database.\n");
	}

	exec_sync("if not exist \"" + type + "\" md " + type);

	if (type == "bgm") {
		sql = "SELECT 'http://storage.game.starlight-stage.jp/dl/resources/High/Sound/Common/b/'||hash AS url, REPLACE(REPLACE(name,'b/',''),'.acb','') AS filename FROM manifests WHERE name LIKE 'b/%acb'";
	}
	else if (type == "live") {
		sql = "SELECT 'http://storage.game.starlight-stage.jp/dl/resources/High/Sound/Common/l/'||hash AS url, REPLACE(REPLACE(name,'l/',''),'.acb','') AS filename FROM manifests WHERE name LIKE 'l/%acb'";
	}
	else if (type == "card") {
		sql = "SELECT 'http://storage.game.starlight-stage.jp/dl/resources/High/AssetBundles/Android/'||hash AS url, REPLACE(name,'.unity3d','') AS filename FROM manifests WHERE name LIKE 'card_bg_______.unity3d'";
	}
	else if (type == "icon") {
		sql = "SELECT 'http://storage.game.starlight-stage.jp/dl/resources/High/AssetBundles/Android/'||hash AS url, REPLACE(name,'.unity3d','') AS filename FROM manifests WHERE name LIKE 'card________m.unity3d'";
	}
	else if (type == "score") {
		sql = "SELECT 'http://storage.game.starlight-stage.jp/dl/resources/Generic/'||hash AS url, REPLACE(name,'.bdb','') AS filename FROM manifests WHERE name LIKE 'musicscores%bdb'";
	}
	read_database(db, sql, data, zErrMsg, rc, type);

	sqlite3_close(db);
}

void Downloader::download_single(string file) {
	sqlite3 *db;
	string sql;
	char *zErrMsg = 0;
	int rc;
	const char* data = file.c_str();

	string sqlfile = "data\\manifest_" + res_ver + ".db";
	rc = sqlite3_open(sqlfile.c_str(), &db);
	if (rc) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		exit(0);
	}
	else {
		fprintf(stderr, "Successfully open database.\n");
	}

	string suffixStr = file.substr(file.find_last_of(".") + 1);
	exec_sync("if not exist \"dl\" md dl");
	if (suffixStr == "acb") {
		string acb_type = file.substr(0,1);
		sql = "SELECT 'http://storage.game.starlight-stage.jp/dl/resources/High/Sound/Common/" + acb_type + "/'||hash AS url, REPLACE(REPLACE(name,'" + acb_type + "/',''),'.acb','') AS filename FROM manifests WHERE name='" + file + "'";
	}
	else if (suffixStr == "unity3d") {
		sql = "SELECT 'http://storage.game.starlight-stage.jp/dl/resources/High/AssetBundles/Android/'||hash AS url, REPLACE(name,'.unity3d','') AS filename FROM manifests WHERE name='" + file + "'";
	}
	else if (suffixStr == "bdb") {
		sql = "SELECT 'http://storage.game.starlight-stage.jp/dl/resources/Generic/'||hash AS url, REPLACE(name,'.bdb','') AS filename FROM manifests WHERE name='" + file + "'";
	}
	else if (suffixStr == "mdb") {
		sql = "SELECT 'http://storage.game.starlight-stage.jp/dl/resources/Generic/'||hash AS url, REPLACE(name,'.mdb','') AS filename FROM manifests WHERE name='" + file + "'";
	}
	else {
		exec_sync("cls");
		printf("File name error.\n\n");
		return;
	}

	rc = sqlite3_exec(db, sql.c_str(), exist, (void*)data, &zErrMsg);
	if (Downloader::exists != 0) {
		rc = sqlite3_exec(db, sql.c_str(), get_single, (void*)suffixStr.c_str(), &zErrMsg);
	}
	else {
		exec_sync("cls");
		printf("%s not found.\n\n", file.c_str());
	}
	is_db_ok(rc, zErrMsg);
}

int main(int argc, char* argv[]) {
	exec_sync("echo off");
	exec_sync("cls");

	int v = string_index_of(argv, "-v", argc);
	int o = string_index_of(argv, "-o", argc);
	int u = string_index_of(argv, "-u", argc);

	string version;
	string option;

	if (v != -1 && v + 1 < argc) {

		if (atoi(argv[v + 1]) > 10012760) {
			version = argv[v + 1];
		}
		else {
			printf("[ERROR] Please try resource version later than 10012760");
			return 0;
		}

		if (u != -1) {
			Downloader::copy = 1;
		}

		if (o == -1) {
			Downloader downloader(version, "");
			downloader.check_manifest();
		}
		else if (o != -1 && o + 1 < argc) {
			option = argv[o + 1];
			if (option == "bgm" || option == "live" || option == "card" || option == "icon" || option == "score") {
				Downloader downloader(version, option);
				downloader.check_manifest();
				downloader.download_asset();
			}
			else {
				Downloader downloader(version, "");
				downloader.check_manifest();
				downloader.download_single(option);
			}
		}

	}
	else {
		printf("CGSSAssetsDownloader ver 1.5\n\n");

		printf("Usage: CGSSAssetsDownloader <-v resource_version> [-o option or filename] [-u]\n\n");

		printf("If you don't know the <resource_version>, try to visit\nhttps://starlight.kirara.ca/api/v1/info\n\n");

		printf("option could be: \n");
		printf("  bgm \t all background music will be downloaded.\n");
		printf("  live \t all live music will be downloaded.\n");
		printf("  card \t all unity3d files that contain card background will be downloaded.\n");
		printf("  icon \t all unity3d files that contain 124x124 card icon will be downloaded.\n");
		printf("  score \t all bdb files that contain music score will be downloaded.\n\n");

		printf("You can use \"DB Browser for SQLite\" open the manifest database file in data\\ to browse file names\n\n");
		printf("Example:\nCGSSAssetsDownloader -v 10027700 -o bgm -u\n");
		printf("CGSSAssetsDownloader -v 10028005 -o gachaselect_30145.unity3d\n\n");

		printf("Developed by github@toyobayashi, tieba@ÆßÞy_Nyanko, weibo@TTPTs\n\n");

		printf("Powered by:\n");
		printf("hcadec\n");
		printf("OpenCGSS/Deretore\n");
		printf("SQLite\n");
		printf("UnityLz4\n\n");

		printf("The copyright of CGSS and its related content is held by BANDAI NAMCO Entertainment Inc.\n\n");

		system("pause");
	}
	return 0;
}
