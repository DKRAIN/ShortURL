#pragma once

#include <Url.h>
#include <drogon/HttpController.h>

using namespace drogon;
using Url = drogon_model::sqlite3::Url;

class ShortUrl : public drogon::HttpController<ShortUrl> {

  std::set<std::string> TLDs;
  orm::Mapper<Url> mp{app().getDbClient()};
  HttpResponsePtr root_resp;
  std::set<std::string> blocked_domains;
public:
  METHOD_LIST_BEGIN
  ADD_METHOD_TO(ShortUrl::generate, "/", Get, Post);
  ADD_METHOD_TO(ShortUrl::redirect, "/{}", Get);
  METHOD_LIST_END
  void generate(const HttpRequestPtr &req,
                std::function<void(const HttpResponsePtr &)> &&callback);
  void redirect(const HttpRequestPtr &req,
                std::function<void(const HttpResponsePtr &)> &&callback,
                std::string s);
  ShortUrl();
};
