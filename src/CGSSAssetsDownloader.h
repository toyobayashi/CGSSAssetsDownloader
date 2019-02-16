#ifndef __CGSSASSETSDOWNLOADER_H__
#define __CGSSASSETSDOWNLOADER_H__

#include "../lib/sqlite3/sqlite3.h"

class Downloader {
private:
  std::string res_ver;
  std::string type;
public:
  static int current;
  static int max;
  static int exists;
  static int copy;
  static int mp3;
  static int auto_update;
  static std::string update_list;

  Downloader(std::string, std::string);
  void check_manifest();
  void download_manifest();
  void download_asset();
  void download_single(std::string);
  void set_type(std::string);
};

int Downloader::current = 0;
int Downloader::max = 0;
int Downloader::exists = 0;
int Downloader::copy = 0;
int Downloader::mp3 = 0;
int Downloader::auto_update = 0;
std::string Downloader::update_list = "";

bool extract_acb (std::string acbFile);

int string_index_of (char* arr[], const char* str, int length);

void exec_sync (std::string);

void is_db_ok (int &r, char *&errmsg);

std::string dir_name ();

void hcadec (std::string hcafile);

int exist(void *data, int argc, char **argv, char **azColName);

int sum_number(void *data, int argc, char **argv, char **azColName);

int get_asset(void *data, int argc, char **argv, char **azColName);

int get_single(void *data, int argc, char **argv, char **azColName);

void read_database(sqlite3 *db, const char *sql, const char* data, char *zErrMsg, int rc, std::string type);

#endif // !__CGSSASSETSDOWNLOADER_H__
