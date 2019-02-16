#ifndef __API_CLIENT_H__
#define __API_CLIENT_H__

#include "./JSBuffer.h"
#include "../lib/json/json.hpp"

class ApiClient {
 private:
  String _user;
  String _viewer;
  String _udid;
  String _sid;
  String _resVer;
  ApiClient(){};
  static String VIEWER_ID_KEY;
  static String SID_KEY;
  static String chr(int);
  static int ord(const String&);
  static String createRandomNumberString(int n = 1);
  static String md5Encrypt(const String&);
  static String sha1Encrypt(const String&);
  static String _04x(int);
  static String _xFFFF32();
  static String b64encode(const String&);
  static String b64encode(const Buffer&);
  static Buffer b64decode(const String&);
  static String encode(const String&);
  static size_t onData(void* buffer, size_t size, size_t nmemb, void* userp);

 public:
 
  explicit ApiClient(const String&, const String& resVer = "10051310");
  nlohmann::json post(const String& path, nlohmann::json& args);
  nlohmann::json check();
  static nlohmann::json msgpackDecode(const Buffer&);
  static Buffer msgpackEncode(const nlohmann::json&);
  static Buffer encryptRJ256(const String&, const Buffer&, const Buffer&);
  static String decryptRJ256(const Buffer&, const Buffer&, const Buffer&);
  static nlohmann::json decryptBody(const String&, const Buffer&);
};

#endif
