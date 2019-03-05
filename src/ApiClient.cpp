#include "./ApiClient.h"
#include <iostream>
#include <sstream>
#include <vector>
#include "../lib/aes/aes.hpp"
#include "../lib/jstype/JSMath.h"
#include "../lib/md5/md5.h"
#include "../lib/sha1/sha1.h"
#include <curl.h>

String ApiClient::VIEWER_ID_KEY = "cyU1Vk5RKEgkJkJxYjYjMys3OGgyOSFGdDR3U2cpZXg=";
String ApiClient::SID_KEY = "ciFJQG50OGU1aT0=";

ApiClient::ApiClient(const String& account, const String& resVer) {
  int first = account.indexOf(":");
  int second = account.lastIndexOf(":");
  _user = account.substring(0, first);
  _viewer = account.substring(first + 1, second);
  _udid = account.substring(second + 1);

  _sid = "";
  _resVer = resVer;
}

Buffer ApiClient::msgpackEncode(const nlohmann::json& data) {
  std::vector<byte> buf = nlohmann::json::to_msgpack(data);
  return Buffer::from(buf.data(), buf.size());
}

nlohmann::json ApiClient::post(const String& path, nlohmann::json& args) {
  String viewerIV = createRandomNumberString(16);
  args["timezone"] = "09:00:00";
  args["viewer_id"] = (viewerIV + b64encode(encryptRJ256(this->_viewer, Buffer::from(viewerIV), b64decode(ApiClient::VIEWER_ID_KEY)))).toCppString();

  String plain = b64encode(msgpackEncode(args));
  String key = b64encode(_xFFFF32()).substring(0, 32);
  Buffer bodyIV = Buffer::from(_udid.replace(std::regex("-"), ""), "hex");
  Buffer plainBuffer = encryptRJ256(plain, bodyIV, Buffer::from(key));
  Buffer bodyBuffer = plainBuffer.concat(Buffer::from(key));
  String body = b64encode(bodyBuffer);
  String sid = _sid == "" ? (_viewer + _udid) : _sid;

  CURL* curl = curl_easy_init();
  struct curl_slist* headers = NULL;

  headers = curl_slist_append(headers, (String("USER-ID: ") + encode(_user)).toCString());
  headers = curl_slist_append(headers, "DEVICE-NAME: Nexus 42");
  headers = curl_slist_append(headers, "APP-VER: 9.9.9");
  headers = curl_slist_append(headers, (String("DEVICE-ID: ") + md5Encrypt("Totally a real Android")).toCString());
  headers = curl_slist_append(headers, "GRAPHICS-DEVICE-NAME: 3dfx Voodoo2 (TM)");
  headers = curl_slist_append(headers, "IDFA: ");
  headers = curl_slist_append(headers, (String("SID: ") + md5Encrypt(sid + b64decode(ApiClient::SID_KEY).toString())).toCString());
  headers = curl_slist_append(headers, "DEVICE: 2");
  headers = curl_slist_append(headers, "KEYCHAIN: ");
  headers = curl_slist_append(headers, "PLATFORM-OS-VERSION: Android OS 13.3.7 / API-42 (XYZZ1Y/74726f6c6c)");
  headers = curl_slist_append(headers, (String("PARAM: ") + sha1Encrypt(_udid + _viewer + path + plain)).toCString());
  headers = curl_slist_append(headers, "X-Unity-Version': '2017.4.2f2");
  headers = curl_slist_append(headers, "CARRIER: google");
  headers = curl_slist_append(headers, (String("RES-VER: ") + _resVer).toCString());
  headers = curl_slist_append(headers, (String("UDID: ") + encode(_udid)).toCString());
  headers = curl_slist_append(headers, "IP-ADDRESS: 127.0.0.1");
  headers = curl_slist_append(headers, "User-Agent: Dalvik/2.1.0 (Linux; U; Android 13.3.7; Nexus 42 Build/XYZZ1Y)");
  headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");

  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

  curl_easy_setopt(curl, CURLOPT_URL, (String("https://apis.game.starlight-stage.jp") + path).toCString());

  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.toCString());
  curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body.byteLength());
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5);  
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);
  curl_easy_setopt(curl, CURLOPT_ENCODING, "gzip");

  // curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, &write_data);
  // curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header);
  std::string resString = "";

  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &onData);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resString);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

  CURLcode code = curl_easy_perform(curl);
  if (code != CURLE_OK) {
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    nlohmann::json errorRes;
    errorRes["data"] = nullptr;
    errorRes["error"] = curl_easy_strerror(code);
    return errorRes;
  }

  nlohmann::json res;

  nlohmann::json resJson = decryptBody(resString, bodyIV);
  if (resJson.find("data_headers") != resJson.end()) {
    _sid = resJson["data_headers"]["sid"].get<std::string>();
    res["data"] = resJson;
    res["error"] = nullptr;
  } else {
    res["data"] = nullptr;
    res["error"] = resJson.dump();
  }

  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);
  
  return res;
}

nlohmann::json ApiClient::check() {
  nlohmann::json args;
  args["campaign_data"] = "";
  args["campaign_user"] = 1337;
  args["campaign_sign"] = md5Encrypt("All your APIs are belong to us").toCString();
  args["app_type"] = 0;

  nlohmann::json res = post("/load/check", args);

  if (!res["error"].is_null()) {
    return res;
  }

  int resultCode = res["data"]["data_headers"]["result_code"].get<int>();

  if (resultCode == 214) {
    nlohmann::json result;
    result["data"] = res["data"]["data_headers"]["required_res_ver"].get<std::string>();
    result["error"] = nullptr;
    return result;
  } else if (resultCode == 1) {
    nlohmann::json result;
    result["data"] = _resVer.toCppString();
    result["error"] = nullptr;
    return result;
  } else {
    nlohmann::json result;
    result["data"] = nullptr;
    result["error"] = resultCode;
    return result;
  }
}

size_t ApiClient::onData(void* buffer, size_t size, size_t nmemb, void* userp) {
  char* d = (char*)buffer;
  std::string* b = (std::string*)(userp);
  int result = 0;
  if (b != NULL) {
    b->append(d, size * nmemb);
    result = size * nmemb;
  }
  return result;
}

nlohmann::json ApiClient::msgpackDecode(const Buffer& data) {
  return nlohmann::json::from_msgpack(data.toVector());
}

nlohmann::json ApiClient::decryptBody(const String& body, const Buffer& iv) {
  Buffer bin = Buffer::from(body, "base64");
  Buffer data = bin.slice(0, bin.length() - 32);
  Buffer key = bin.slice(bin.length() - 32);
  String plain = decryptRJ256(data, iv, key);
  const char* plainBuf = plain.toCString();
  int byteLength = plain.byteLength();
  if (plainBuf[byteLength - 1] > 0 && plainBuf[byteLength - 1] <= 16) {
    char* substr = new char[byteLength - plainBuf[byteLength - 1] + 1]{0};
    for (int i = 0; i < byteLength - plainBuf[byteLength - 1]; i++) {
      substr[i] = plainBuf[i];
    }
    plain = substr;
    delete[] substr;
  }
  
  // printf("%s\n", plain.toCString());

  return msgpackDecode(Buffer::from(plain, "base64"));
}

Buffer ApiClient::encryptRJ256(const String& data,
                               const Buffer& iv,
                               const Buffer& key) {
  int dataLength = data.byteLength();
  const byte* strBuf = (const byte*) data.toCString();

  uint8_t* dataBuf = nullptr;

  int padding = dataLength % 16;
  uint32_t encryptLength = 0;
  if (padding != 0) {
      padding = 16 - padding;
      encryptLength = (uint32_t) (dataLength + padding);
      dataBuf = new uint8_t[encryptLength];
      for (int i = 0; i < dataLength; i++) {
          dataBuf[i] = strBuf[i];
      }
      for (int i = 0; i < padding; i++) {
          dataBuf[i + dataLength] = (uint8_t) padding;
      }
  } else {
      encryptLength = (uint32_t) (dataLength);
      dataBuf = new uint8_t[dataLength];
      for (int i = 0; i < dataLength; i++) {
          dataBuf[i] = strBuf[i];
      }
  }

  struct AES_ctx ctx;
  AES_init_ctx_iv(&ctx, key.buffer(), iv.buffer());
  AES_CBC_encrypt_buffer(&ctx, dataBuf, encryptLength);

  Buffer out = Buffer::alloc(encryptLength);
  for (int i = 0; i < (int)encryptLength; i++) {
    out[i] = dataBuf[i];
  }

  delete[] dataBuf;
  dataBuf = nullptr;

  return out;
}

String ApiClient::decryptRJ256(const Buffer& data,
                               const Buffer& iv,
                               const Buffer& key) {
  int l = data.length();
  byte* encrypt = new byte[l];
  for (int i = 0; i < l; i++) {
    encrypt[i] = data[i];
  }

  struct AES_ctx ctx;
  AES_init_ctx_iv(&ctx, key.buffer(), iv.buffer());
  AES_CBC_decrypt_buffer(&ctx, encrypt, l);

  Buffer out = Buffer::alloc(l);
  for (int i = 0; i < l; i++) {
    out[i] = encrypt[i];
  }

  delete[] encrypt;
  encrypt = nullptr;

  return out.toString();
}

String ApiClient::chr(int c) {
  return String::fromCharCode(c);
};

int ApiClient::ord(const String& str) {
  return str.charCodeAt(0);
}

String ApiClient::createRandomNumberString(int n) {
  String res = "";
  for (int i = 0; i < n; i++) {
    res += std::to_string(Math::floor(10 * Math::random()));
  }
  return res;
}

String ApiClient::md5Encrypt(const String& data) {
  uint8_t res[16] = {0};
  md5((const byte*)data.toCString(), data.byteLength(), res);
  Buffer buf = Buffer::from(res, 16);
  return buf.toString("hex");
}

String ApiClient::sha1Encrypt(const String& data) {
  uint8_t res[21] = {0};
  SHA1((char*)res, data.toCString(), data.byteLength());
  Buffer buf = Buffer::from(res, 20);
  return buf.toString("hex");
}

String ApiClient::_04x(int n) {
  char res[5] = {0};
  sprintf(res, "%04x", n);
  return res;
}

String ApiClient::_xFFFF32() {
  std::ostringstream os;
  std::ostream &out = os;
  out << std::hex;

  for (int i = 0; i < 32; i++) {
    out << Math::floor(65536 * Math::random());
  }
  return os.str();
}

String ApiClient::b64encode(const String& s) {
  return Buffer::from(s).toString("base64");
}

String ApiClient::b64encode(const Buffer& buf) {
  return buf.toString("base64");
}

Buffer ApiClient::b64decode(const String& s) {
  return Buffer::from(s, "base64");
}

String ApiClient::encode(const String& s) {
  int len = s.length();
  String res = _04x(len);
  for (int i = 0; i < len; i++) {
    String c = s[i];
    res += (createRandomNumberString(2) + chr(ord(c) + 10) + createRandomNumberString(1));
  }
  res += createRandomNumberString(32);

  return res;
}
