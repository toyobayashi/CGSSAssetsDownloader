#include "CGSSAssetsDownloader.h"

string update_list = "";

void exec_sync(string cmd) {
	system(cmd.c_str());
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
		printf(name.c_str());
		printf("\n");
		exec_sync("tool\\wget\\wget -c " + url + " -O " + "./bgm/" + name + ".acb");
		exec_sync("tool\\DereTore.ACB\\DereTore.ACB.Test.exe bgm\\" + name + ".acb");
		exec_sync("tool\\HCADecoder\\hca.exe -v 1.0 -m 16 -l 0 -a F27E3B22 -b 00003657 bgm\\_deretore_acb_extract_" + name + ".acb\\acb\\awb\\" + name + ".hca");
		exec_sync("move bgm\\_deretore_acb_extract_" + name + ".acb\\acb\\awb\\" + name + ".wav bgm\\");
		exec_sync("rd bgm\\_deretore_acb_extract_" + name + ".acb /s /q");
		exec_sync("del bgm\\" + name + ".acb");
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
		printf(name.c_str());
		printf("\n");
		exec_sync("tool\\wget\\wget -c " + url + " -O " + "./live/" + name + ".acb");
		exec_sync("tool\\DereTore.ACB\\DereTore.ACB.Test.exe live\\" + name + ".acb");
		exec_sync("tool\\HCADecoder\\hca.exe -v 1.0 -m 16 -l 0 -a F27E3B22 -b 00003657 live\\_deretore_acb_extract_" + name + ".acb\\acb\\awb\\" + name + ".hca");
		exec_sync("move live\\_deretore_acb_extract_" + name + ".acb\\acb\\awb\\" + name + ".wav live\\");
		exec_sync("rd live\\_deretore_acb_extract_" + name + ".acb /s /q");
		exec_sync("del live\\" + name + ".acb");
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
		printf(name.c_str());
		printf("\n");
		exec_sync("tool\\wget\\wget -c " + url + " -O " + "./card/" + name);
		exec_sync("tool\\SSDecompress\\SSDecompress.exe card\\" + name);
		exec_sync("del card\\" + name);
	}
	_file.close();
	return 0;
}

long get_file_size(const char* strFileName) {
	struct _stat info;
	_stat(strFileName, &info);
	long size = info.st_size;
	return size;
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
	exec_sync("tool\\SSDecompress\\SSDecompress.exe data\\manifest_" + res_ver);
	exec_sync("del data\\*.");
	exec_sync("ren data\\*.unity3d *.db");
	printf("Successfully download manifest.\n");
}

void Downloader::download_asset() {
	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	char *sql;
	const char* data = "Callback function called";

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
			printf("===========  Update bgm  ===========\n\n");
			printf(update_list.c_str());
			printf("\n====================================");
		}
	}
	else if (type == "live") {
		exec_sync("md live");
		sql = "SELECT 'http://storage.game.starlight-stage.jp/dl/resources/High/Sound/Common/l/'||hash AS url, REPLACE(REPLACE(name,'l/',''),'.acb','') AS filename FROM manifests WHERE name LIKE 'l/%acb'";
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
			printf("===========  Update live  ===========\n\n");
			printf(update_list.c_str());
			printf("\n=====================================");
		}
	}
	else if (type == "card") {
		exec_sync("md card");
		sql = "SELECT 'http://storage.game.starlight-stage.jp/dl/resources/High/AssetBundles/Android/'||hash AS url, REPLACE(name,'.unity3d','') AS filename FROM manifests WHERE name LIKE 'card_bg_______.unity3d'";
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
			printf("===========  Update live  ===========\n\n");
			printf(update_list.c_str());
			printf("\n=====================================");
		}
	}

	sqlite3_close(db);
}

int main(int argc, char* argv[]) {
	if (argc == 2) {
		Downloader downloader(argv[1], "");
		downloader.check_manifest();
	}
	else if (argc == 3) {
		Downloader downloader(argv[1], argv[2]);
		downloader.check_manifest();
		downloader.download_asset();
	}
	else {
		printf("CGSSAssetsDownloader ver 1.1\n\n");
		printf("Usage: CGSSAssetsDownloader <resource_version> [bgm | live | card]\n");
		printf("Example: CGSSAssetsDownloader 10027700 bgm\n");
		printf("If you don't know the <resource_version>, try to visit\nhttps://starlight.kirara.ca/api/v1/info\n");
		printf("\n");
		printf("By tieba@ÆßÞy_Nyanko, weibo@TTPTs\n\n");
		system("pause");
		exit(0);
	}
	return 0;
}
