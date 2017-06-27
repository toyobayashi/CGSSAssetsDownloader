#pragma once

#include "sqlite3.h"
#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>

using namespace std;

class Downloader {
private:
	string res_ver;
	string type;
public:
	Downloader(string, string);
	void check_manifest();
	void download_manifest();
	void download_asset();
};
