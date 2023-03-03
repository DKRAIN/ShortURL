#include "ShortUrl.h"
#include "Base62.h"
#include "LUrlParser.h"
#include <fstream>

void ShortUrl::generate(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback) {
  // write your application logic here
  std::string longurl;
  if ((longurl = req->getParameter("url")).empty()) {
    auto rjson = req->getJsonObject();
    if (rjson && (*rjson)["url"].isString()) {
      longurl = (*rjson)["url"].asString();
    } else {

      callback(root_resp);

      return;
    }
  }
  if (longurl.size() > 1024) {
    Json::Value json;
    json["code"] = 400;
    json["message"] = "URL too long";
    callback(HttpResponse::newHttpJsonResponse(json));
    return;
  }
  auto url = LUrlParser::ParseURL::parseURL(longurl);
  if (!url.isValid() || (url.scheme_ != "http" && url.scheme_ != "https")) {
    Json::Value json;
    json["code"] = 100;
    json["message"] = "Unsupported scheme";
    callback(HttpResponse::newHttpJsonResponse(json));
    return;
  }
  auto pos = url.host_.find_last_of('.');
  if (pos == -1 || pos + 1 >= url.host_.size() ||
      !TLDs.count(url.host_.substr(pos + 1, url.host_.size()))) {
    Json::Value json;
    json["code"] = 300;
    json["message"] = "Unsupported TLD";
    callback(HttpResponse::newHttpJsonResponse(json));
    return;
  }
  if (url.host_.ends_with(domain)) {
    Json::Value json;
    json["code"] = 300;
    json["message"] = "Unsupported Domain";
    callback(HttpResponse::newHttpJsonResponse(json));
    return;
  }

  mp.findBy(
      orm::Criteria(Url::Cols::_url, orm::CompareOperator::EQ, longurl),
      [callback, this, longurl](std::vector<Url> cb) {
        if (cb.size() == 0) {
          Url sql;
          sql.setUrl(longurl);
          mp.insert(
              sql,
              [callback](const Url dat) {
                Json::Value json;
                json["code"] = 0;
                json["short"] = Base62::encode(*dat.getId());
                callback(HttpResponse::newHttpJsonResponse(json));
              },
              [callback](const orm::DrogonDbException &e) {
                Json::Value json;
                json["code"] = -1;
                json["message"] = "Try it later";
                callback(HttpResponse::newHttpJsonResponse(json));
              });
        } else if (cb.size() == 1) {

          Json::Value json;
          json["code"] = 0;
          json["short"] = Base62::encode(*(cb[0].getId()));
          callback(HttpResponse::newHttpJsonResponse(json));
        } else {
          Json::Value json;
          json["code"] = 500;
          json["message"] = "Server Error";
          callback(HttpResponse::newHttpJsonResponse(json));
        }
      },
      [callback](const orm::DrogonDbException &e) {
        Json::Value json;
        json["code"] = 500;
        json["message"] = "Server Error";
        callback(HttpResponse::newHttpJsonResponse(json));
      });
}
void ShortUrl::redirect(const HttpRequestPtr &req,
                        std::function<void(const HttpResponsePtr &)> &&callback,
                        std::string s) {
  uint64_t id;
  try {
    id = Base62::decode(s);
  } catch (int e) {
    if (e == -1) {
      callback(HttpResponse::newNotFoundResponse());
    } else {
      Json::Value json;
      json["code"] = 500;
      json["message"] = "Server Error";
      callback(HttpResponse::newHttpJsonResponse(json));
      return;
    }
  } catch (...) {

    Json::Value json;
    json["code"] = 500;
    json["message"] = "Server Error";
    callback(HttpResponse::newHttpJsonResponse(json));
    return;
  }

  mp.findByPrimaryKey(
      id,
      [callback](const Url &dat) {
        callback(HttpResponse::newRedirectionResponse(*dat.getUrl()));
      },
      [callback](const orm::DrogonDbException &e) {
        callback(HttpResponse::newNotFoundResponse());
      });
}

ShortUrl::ShortUrl() {
  domain=app().getCustomConfig()["domain"].asString();
  root_resp = HttpResponse::newRedirectionResponse(
      app().getCustomConfig()["root_to"].asString());
  // CREATE TABLE "url" ("id" INTEGER PRIMARY KEY NOT NULL,"url" VARCHAR(1024)
  // UNIQUE NOT NULL);
  std::ifstream ifs("tld.list");
  std::string tmp;
  while (ifs >> tmp) {
    TLDs.insert(tmp);
  }
}