#include "CGSSAssetsDownloader.h"

Downloader::Downloader(string v, string t) {
	res_ver = v;
	type = t;
}

void Downloader::set_type(string t) {
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
		if (Downloader::mp3 != 0) {
			exec_sync("if not exist bgm\\mp3 md bgm\\mp3");
		}
		else {
			exec_sync("if not exist bgm\\wav md bgm\\wav");
		}
	}
	else if (type == "live") {
		sql = "SELECT 'http://storage.game.starlight-stage.jp/dl/resources/High/Sound/Common/l/'||hash AS url, REPLACE(REPLACE(name,'l/',''),'.acb','') AS filename FROM manifests WHERE name LIKE 'l/%acb'";
		if (Downloader::mp3 != 0) {
			exec_sync("if not exist live\\mp3 md live\\mp3");
		}
		else {
			exec_sync("if not exist live\\wav md live\\wav");
		}
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
	exec_sync("if not exist dl md dl");
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
	int mp3 = string_index_of(argv, "-mp3", argc);
	int a = string_index_of(argv, "-a", argc);

	string version;
	string option;

	if (v != -1 && v + 1 < argc) {

		if (atoi(argv[v + 1]) > 10012760) {
			version = argv[v + 1];
		}
		else {
			printf("[ERROR] Please try resource version later than 10012760");
			system("pause>nul");
			return 0;
		}

		if (u != -1) {
			Downloader::copy = 1;
			exec_sync("if not exist dl md dl");
		}

		if (mp3 != -1) {
			Downloader::mp3 = 1;
		}

		if (a != -1) {
			Downloader::auto_update = 1;
			Downloader downloader(version, "");
			downloader.check_manifest();
			downloader.set_type("bgm");
			downloader.download_asset();
			downloader.set_type("live");
			downloader.download_asset();
			downloader.set_type("card");
			downloader.download_asset();
			downloader.set_type("icon");
			downloader.download_asset();
			downloader.set_type("score");
			downloader.download_asset();
			return 0;
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
		printf("CGSSAssetsDownloader VERSION 1.6\n\n");

		printf("Usage: \n");
		printf("CGSSAssetsDownloader <-v resource_version> [-a] [-u] [-mp3]\n");
		printf("CGSSAssetsDownloader <-v resource_version> [-o option or filename] [-u] [-mp3]\n\n");

		printf("Example: \n");
		printf("CGSSAssetsDownloader -v 10027700 -o -bgm -u\n");
		printf("CGSSAssetsDownloader -v 10028005 -o gachaselect_30145.unity3d\n");
		printf("CGSSAssetsDownloader -v 10031250 -a -u -mp3\n\n");

		printf("Arguments: \n");
		printf("<-v resource_version> [NECESSARY] Set the resource version of game and download database.\n");
		printf("[-a] [OPTIONAL] Auto update bgm, live, card, icon, score assets.\n");
		printf("[-o bgm|live|card|icon|score|(filename)] [OPTIONAL] Read the detail below.\n");
		printf("[-u] [OPTIONAL] Copy files to \"dl\\\" folder.\n");
		printf("[-mp3] [OPTIONAL] WAV to MP3.Default: WAV.\n\n");

		printf("If you don't know the <resource_version>, try to visit\nhttps://starlight.kirara.ca/api/v1/info\n\n");

		printf("-o detail: \n");
		printf("  bgm \t all background music will be downloaded.\n");
		printf("  live \t all live music will be downloaded.\n");
		printf("  card \t all unity3d files that contain card background will be downloaded.\n");
		printf("  icon \t all unity3d files that contain 124x124 card icon will be downloaded.\n");
		printf("  score \t all bdb files that contain music score will be downloaded.\n\n");

		printf("You can use \"DB Browser for SQLite\" open the manifest database file in data\\ to browse file names\n\n");
		
		printf("Developed by github@toyobayashi, tieba@ÆßÞy_Nyanko, weibo@TTPTs\n\n");

		printf("Powered by:\n");
		printf("hcadec\n");
		printf("OpenCGSS/Deretore\n");
		printf("ffmpeg\n");
		printf("SQLite\n");
		printf("UnityLz4\n\n");

		printf("The copyright of CGSS and its related content is held by BANDAI NAMCO Entertainment Inc.\n\n");

		system("pause>nul");
	}
	return 0;
}
