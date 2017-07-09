#pragma once

#include "sqlite3.h"
#include "lz4.h"
#include <string>
#include <fstream>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdio.h>
#include "clHCA.h"

using namespace std;

class Downloader {
private:
	string res_ver;
	string type;
public:
	static int current;
	static int max;
	static int exists;

	Downloader(string, string);
	void check_manifest();
	void download_manifest();
	void download_asset();
	void download_single(string);
};

int Downloader::current = 0;
int Downloader::max = 0;
int Downloader::exists = 0;

