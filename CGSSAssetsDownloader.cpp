#include "CGSSAssetsDownloader.h"

string update_list = "";

void exec_sync(string cmd) {
	system(cmd.c_str());
}

long get_file_size(const char* strFileName) {
	struct _stat info;
	_stat(strFileName, &info);
	long size = info.st_size;
	return size;
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

static int get_b(void *data, int argc, char **argv, char **azColName) {
	string url(argv[0]);
	string name(argv[1]);

	fstream _file;
	string fileName = "bgm\\" + name + ".wav";
	_file.open(fileName.c_str(), ios::in);

	if (!_file) {
		update_list += name;
		update_list += "\n";
		exec_sync("cls");
		printf("Completed:\t%d/%d\nDownloading:\t%s\n\n", Downloader::current, Downloader::max, name.c_str());
		exec_sync("tool\\wget\\wget -c " + url + " -O " + "./bgm/" + name + ".acb");
		exec_sync("tool\\AcbUnzip\\AcbUnzip.exe bgm\\" + name + ".acb");
		hcadec("bgm\\_acb_" + name + ".acb\\" + name + ".hca");
		exec_sync("move bgm\\_acb_" + name + ".acb\\" + name + ".wav bgm\\");
		exec_sync("rd bgm\\_acb_" + name + ".acb /s /q");
		exec_sync("del bgm\\" + name + ".acb");
		Downloader::current++;
	}
	_file.close();
	return 0;
}

static int get_l(void *data, int argc, char **argv, char **azColName) {
	string url(argv[0]);
	string name(argv[1]);

	fstream _file;
	string fileName = "live\\" + name + ".wav";
	_file.open(fileName.c_str(), ios::in);

	if (!_file) {
		update_list += name;
		update_list += "\n";
		exec_sync("cls");
		printf("Completed:\t%d/%d\nDownloading:\t%s\n\n", Downloader::current, Downloader::max, name.c_str());
		exec_sync("tool\\wget\\wget -c " + url + " -O " + "./live/" + name + ".acb");
		exec_sync("tool\\AcbUnzip\\AcbUnzip.exe live\\" + name + ".acb");
		hcadec("live\\_acb_" + name + ".acb\\" + name + ".hca");
		exec_sync("move live\\_acb_" + name + ".acb\\" + name + ".wav live\\");
		exec_sync("rd live\\_acb_" + name + ".acb /s /q");
		exec_sync("del live\\" + name + ".acb");
		Downloader::current++;
	}
	_file.close();
	return 0;
}

static int get_c(void *data, int argc, char **argv, char **azColName) {
	string url(argv[0]);
	string name(argv[1]);

	fstream _file;
	string fileName = "card\\" + name + ".unity3d";
	_file.open(fileName.c_str(), ios::in);

	if (!_file) {
		update_list += name;
		update_list += "\n";
		exec_sync("cls");
		printf("Completed:\t%d/%d\nDownloading:\t%s\n\n", Downloader::current, Downloader::max, name.c_str());
		exec_sync("tool\\wget\\wget -c " + url + " -O " + "./card/" + name);
		lz4dec("card\\" + name, "unity3d");
		exec_sync("del card\\" + name);
		Downloader::current++;
	}
	_file.close();
	return 0;
}

static int get_i(void *data, int argc, char **argv, char **azColName) {
	string url(argv[0]);
	string name(argv[1]);

	fstream _file;
	string fileName = "icon\\" + name + ".unity3d";
	_file.open(fileName.c_str(), ios::in);

	if (!_file) {
		update_list += name;
		update_list += "\n";
		exec_sync("cls");
		printf("Completed:\t%d/%d\nDownloading:\t%s\n\n", Downloader::current, Downloader::max, name.c_str());
		exec_sync("tool\\wget\\wget -c " + url + " -O " + "./icon/" + name);
		lz4dec("icon\\" + name, "unity3d");
		exec_sync("del icon\\" + name);
		Downloader::current++;
	}
	_file.close();
	return 0;
}

static int get_s(void *data, int argc, char **argv, char **azColName) {
	string url(argv[0]);
	string name(argv[1]);

	fstream _file;
	string fileName = "score\\" + name + ".bdb";
	_file.open(fileName.c_str(), ios::in);

	if (!_file) {
		update_list += name;
		update_list += "\n";
		exec_sync("cls");
		printf("Completed:\t%d/%d\nDownloading:\t%s\n\n", Downloader::current, Downloader::max, name.c_str());
		exec_sync("tool\\wget\\wget -c " + url + " -O " + "./score/" + name);
		lz4dec("score\\" + name, "bdb");
		exec_sync("del score\\" + name);
		Downloader::current++;
	}
	_file.close();
	return 0;
}

static int get_acb(void *data, int argc, char **argv, char **azColName) {
	string url(argv[0]);
	string name(argv[1]);

	fstream _file;
	string fileName = "dl\\" + name + ".acb";
	_file.open(fileName.c_str(), ios::in);

	if (!_file) {
		printf(name.c_str());
		printf("\n\n");
		exec_sync("tool\\wget\\wget -c " + url + " -O " + "./dl/" + name);
		exec_sync("ren dl\\" + name + " " + name + ".acb");
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

static int get_unity3d(void *data, int argc, char **argv, char **azColName) {
	string url(argv[0]);
	string name(argv[1]);

	fstream _file;
	string fileName = "dl\\" + name + ".unity3d";
	_file.open(fileName.c_str(), ios::in);

	if (!_file) {
		printf(name.c_str());
		printf("\n\n");
		exec_sync("tool\\wget\\wget -c " + url + " -O " + "./dl/" + name);
		lz4dec("dl\\" + name, "unity3d");
		exec_sync("del dl\\" + name);
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

static int get_bdb(void *data, int argc, char **argv, char **azColName) {
	string url(argv[0]);
	string name(argv[1]);

	fstream _file;
	string fileName = "dl\\" + name + ".bdb";
	_file.open(fileName.c_str(), ios::in);

	if (!_file) {
		printf(name.c_str());
		printf("\n\n");
		exec_sync("tool\\wget\\wget -c " + url + " -O " + "./dl/" + name);
		lz4dec("dl\\" + name, "bdb");
		exec_sync("del dl\\" + name);
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

static int get_mdb(void *data, int argc, char **argv, char **azColName) {
	string url(argv[0]);
	string name(argv[1]);

	fstream _file;
	string fileName = "dl\\" + name + ".mdb";
	_file.open(fileName.c_str(), ios::in);

	if (!_file) {
		printf(name.c_str());
		printf("\n\n");
		exec_sync("tool\\wget\\wget -c " + url + " -O " + "./dl/" + name);
		lz4dec("dl\\" + name, "mdb");
		exec_sync("del dl\\" + name);
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
	string url, ssd;
	exec_sync("md data");
	exec_sync("tool\\wget\\wget -c -O data\\manifest_" + res_ver + " http://storage.game.starlight-stage.jp/dl/" + res_ver + "/manifests/Android_AHigh_SHigh");

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

	if (type == "bgm") {
		exec_sync("md bgm");
		sql = "SELECT 'http://storage.game.starlight-stage.jp/dl/resources/High/Sound/Common/b/'||hash AS url, REPLACE(REPLACE(name,'b/',''),'.acb','') AS filename FROM manifests WHERE name LIKE 'b/%acb'";
		rc = sqlite3_exec(db, sql, sum_number, (void*)data, &zErrMsg);
		rc = sqlite3_exec(db, sql, get_b, (void*)data, &zErrMsg);
		if (rc != SQLITE_OK) {
			fprintf(stderr, "SQL error: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
			exit(0);
		}
		else {
			ofstream bgm_txt;
			exec_sync("md log");
			bgm_txt.open("log\\bgm.txt");
			bgm_txt << update_list;
			bgm_txt.close();
			exec_sync("cls");
			printf("%d/%d Completed.\n\n", Downloader::current, Downloader::max);
			printf("===========  Update bgm  ===========\n\n");
			printf(update_list.c_str());
			printf("\n====================================");
		}
	}
	else if (type == "live") {
		exec_sync("md live");
		sql = "SELECT 'http://storage.game.starlight-stage.jp/dl/resources/High/Sound/Common/l/'||hash AS url, REPLACE(REPLACE(name,'l/',''),'.acb','') AS filename FROM manifests WHERE name LIKE 'l/%acb'";
		rc = sqlite3_exec(db, sql, sum_number, (void*)data, &zErrMsg);
		rc = sqlite3_exec(db, sql, get_l, (void*)data, &zErrMsg);
		if (rc != SQLITE_OK) {
			fprintf(stderr, "SQL error: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
			exit(0);
		}
		else {
			ofstream live_txt;
			exec_sync("md log");
			live_txt.open("log\\live.txt");
			live_txt << update_list;
			live_txt.close();
			exec_sync("cls");
			printf("%d/%d Completed.\n\n", Downloader::current, Downloader::max);
			printf("===========  Update live  ===========\n\n");
			printf(update_list.c_str());
			printf("\n=====================================");
		}
	}
	else if (type == "card") {
		exec_sync("md card");
		sql = "SELECT 'http://storage.game.starlight-stage.jp/dl/resources/High/AssetBundles/Android/'||hash AS url, REPLACE(name,'.unity3d','') AS filename FROM manifests WHERE name LIKE 'card_bg_______.unity3d'";
		rc = sqlite3_exec(db, sql, sum_number, (void*)data, &zErrMsg);
		rc = sqlite3_exec(db, sql, get_c, (void*)data, &zErrMsg);
		if (rc != SQLITE_OK) {
			fprintf(stderr, "SQL error: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
			exit(0);
		}
		else {
			ofstream live_txt;
			exec_sync("md log");
			live_txt.open("log\\card.txt");
			live_txt << update_list;
			live_txt.close();
			exec_sync("cls");
			printf("%d/%d Completed.\n\n", Downloader::current, Downloader::max);
			printf("===========  Update live  ===========\n\n");
			printf(update_list.c_str());
			printf("\n=====================================");
		}
	}
	else if (type == "icon") {
		exec_sync("md icon");
		sql = "SELECT 'http://storage.game.starlight-stage.jp/dl/resources/High/AssetBundles/Android/'||hash AS url, REPLACE(name,'.unity3d','') AS filename FROM manifests WHERE name LIKE 'card________m.unity3d'";
		rc = sqlite3_exec(db, sql, sum_number, (void*)data, &zErrMsg);
		rc = sqlite3_exec(db, sql, get_i, (void*)data, &zErrMsg);
		if (rc != SQLITE_OK) {
			fprintf(stderr, "SQL error: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
			exit(0);
		}
		else {
			ofstream icon_txt;
			exec_sync("md log");
			icon_txt.open("log\\icon.txt");
			icon_txt << update_list;
			icon_txt.close();
			exec_sync("cls");
			printf("%d/%d Completed.\n\n", Downloader::current, Downloader::max);
			printf("===========  Update icon  ===========\n\n");
			printf(update_list.c_str());
			printf("\n=====================================");
		}
	}
	else if (type == "score") {
		exec_sync("md score");
		sql = "SELECT 'http://storage.game.starlight-stage.jp/dl/resources/Generic/'||hash AS url, REPLACE(name,'.bdb','') AS filename FROM manifests WHERE name LIKE 'musicscores%bdb'";
		rc = sqlite3_exec(db, sql, sum_number, (void*)data, &zErrMsg);
		rc = sqlite3_exec(db, sql, get_s, (void*)data, &zErrMsg);
		if (rc != SQLITE_OK) {
			fprintf(stderr, "SQL error: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
			exit(0);
		}
		else {
			ofstream score_txt;
			exec_sync("md log");
			score_txt.open("log\\score.txt");
			score_txt << update_list;
			score_txt.close();
			exec_sync("cls");
			printf("%d/%d Completed.\n\n", Downloader::current, Downloader::max);
			printf("===========  Update score  ===========\n\n");
			printf(update_list.c_str());
			printf("\n=====================================");
		}
	}

	sqlite3_close(db);
}

void Downloader::download_single(string file) {
	sqlite3 *db;
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
	exec_sync("md dl");
	if (suffixStr == "acb") {
		string acb_type = file.substr(0,1);
		string sql = "SELECT 'http://storage.game.starlight-stage.jp/dl/resources/High/Sound/Common/" + acb_type + "/'||hash AS url, REPLACE(REPLACE(name,'" + acb_type + "/',''),'.acb','') AS filename FROM manifests WHERE name='" + file + "'";
		rc = sqlite3_exec(db, sql.c_str(), exist, (void*)data, &zErrMsg);
		if (Downloader::exists != 0) {
			rc = sqlite3_exec(db, sql.c_str(), get_acb, (void*)data, &zErrMsg);
		}
		else {
			exec_sync("cls");
			printf("%s not found.\n\n", file.c_str());
		}
		is_db_ok(rc, zErrMsg);
	}
	else if (suffixStr == "unity3d") {
		string sql = "SELECT 'http://storage.game.starlight-stage.jp/dl/resources/High/AssetBundles/Android/'||hash AS url, REPLACE(name,'.unity3d','') AS filename FROM manifests WHERE name='" + file + "'";
		rc = sqlite3_exec(db, sql.c_str(), exist, (void*)data, &zErrMsg);
		if (Downloader::exists != 0) {
			rc = sqlite3_exec(db, sql.c_str(), get_unity3d, (void*)data, &zErrMsg);
		}
		else {
			exec_sync("cls");
			printf("%s not found.\n\n", file.c_str());
		}
		is_db_ok(rc, zErrMsg);
	}
	else if (suffixStr == "bdb") {
		string sql = "SELECT 'http://storage.game.starlight-stage.jp/dl/resources/Generic/'||hash AS url, REPLACE(name,'.bdb','') AS filename FROM manifests WHERE name='" + file + "'";
		rc = sqlite3_exec(db, sql.c_str(), exist, (void*)data, &zErrMsg);
		if (Downloader::exists != 0) {
			rc = sqlite3_exec(db, sql.c_str(), get_bdb, (void*)data, &zErrMsg);
		}
		else {
			exec_sync("cls");
			printf("%s not found.\n\n", file.c_str());
		}
		is_db_ok(rc, zErrMsg);
	}
	else if (suffixStr == "mdb") {
		string sql = "SELECT 'http://storage.game.starlight-stage.jp/dl/resources/Generic/'||hash AS url, REPLACE(name,'.mdb','') AS filename FROM manifests WHERE name='" + file + "'";
		rc = sqlite3_exec(db, sql.c_str(), exist, (void*)data, &zErrMsg);
		if (Downloader::exists != 0) {
			rc = sqlite3_exec(db, sql.c_str(), get_mdb, (void*)data, &zErrMsg);
		}
		else {
			exec_sync("cls");
			printf("%s not found.\n\n", file.c_str());
		}
		is_db_ok(rc, zErrMsg);
	}
	else {
		exec_sync("cls");
		printf("File name error.\n\n");
	}
}

int main(int argc, char* argv[]) {
	exec_sync("echo off");
	exec_sync("cls");
	if (argc == 2) {
		Downloader downloader(argv[1], "");
		downloader.check_manifest();
	}
	else if (argc == 3) {
		exec_sync("echo off");
		string option = argv[2];
		if (option == "bgm" || option == "live" || option == "card" || option == "icon" || option == "score") {
			Downloader downloader(argv[1], argv[2]);
			downloader.check_manifest();
			downloader.download_asset();
		}
		else {
			Downloader downloader(argv[1], "");
			downloader.check_manifest();
			downloader.download_single(argv[2]);
		}
	}
	else {
		printf("CGSSAssetsDownloader ver 1.4\n\n");

		printf("Usage: CGSSAssetsDownloader <resource_version> [option or filename]\n\n");

		printf("If you don't know the <resource_version>, try to visit\nhttps://starlight.kirara.ca/api/v1/info\n\n");

		printf("option could be: \n");
		printf("bgm \t all background music will be downloaded.\n");
		printf("live \t all live music will be downloaded.\n");
		printf("card \t all unity3d files that contain card background will be downloaded.\n");
		printf("icon \t all unity3d files that contain 124x124 card icon will be downloaded.\n");
		printf("score \t all bdb files that contain music score will be downloaded.\n\n");

		printf("You can use \"DB Browser for SQLite\" open the manifest database file in data\\ to browse file names\n\n");
		printf("Example:\nCGSSAssetsDownloader 10027700 bgm\n");
		printf("CGSSAssetsDownloader 10028005 gachaselect_30145.unity3d\n\n");
		
		printf("Developed by github@toyobayashi, tieba@ÆßÞy_Nyanko, weibo@TTPTs\n\n");

		printf("Powered by:\n");
		printf("hcadec\n");
		printf("OpenCGSS/Deretore\n");
		printf("SQLite\n");
		printf("UnityLz4\n");
		printf("wget\n\n");

		printf("The copyright of CGSS and its related content is held by BANDAI NAMCO Entertainment, Inc.\n\n");

		system("pause");
	}
	return 0;
}
