/**
@project htmlPorter
@author Tao Zhang
@since 2023/10/14
@version 0.1.0-SNAPSHOT 2023/10/15
*/

#pragma once

#include <iostream>
#include <unordered_map>
#include <functional>
#include <vector>
#include <fstream>
#include <string>
#include <regex>
#include <memory>
#include <iterator>
#include "seeker/common.h"



namespace htmlPorter {



class HtmlTemplate {
 private:
  inline static std::regex getRegex(std::string argName) {
    // <!--@title[-->一些文字内容<!--]title@-->
    // string regStr = "@\\{\\{" + argName + "\\}\\}.*@";
    // string regStr = "<!--@" + argName + "-->.*<!--" + argName + "@-->";
    // string regStr = "<!--@" + argName + "-->(\n|.)*<!--" + argName + "@-->";
    std::string regStr = "<!--@" + argName + "\\[-->.*<!--\\]" + argName + "@-->";
    return std::regex{regStr, std::regex::optimize};
  };

  inline static std::vector<std::regex> patternInitHalper(std::vector<std::string>& argNameList) {
    std::vector<std::regex> ls{};
    std::transform(argNameList.begin(), argNameList.end(), std::back_inserter(ls),
                   [](const std::string& argName) { return getRegex(argName); });
    return ls;
  }


  inline static std::pair<std::string, std::vector<std::string>> parseTemplate(
      const std::string& templateFile) {
    static const std::regex argsLineRe{R"(<!--Args\((.*)\)-->)"};

    std::smatch sm;
    std::vector<std::string> args;

    auto getArgs = [&](std::string ln) -> bool {
      if (std::regex_search(ln, sm, argsLineRe)) {
        auto argsStr = sm.str(1);
        // cout << "argsStr:" << argsStr << endl;
        argsStr = seeker::String::removeBlanks(argsStr);
        // cout << "argsStr processed:" << argsStr << endl;
        args = seeker::String::split(argsStr, ",");
        return true;
      } else {
        return false;
      }
    };

    std::string htmlString{};
    std::string templateFilePath = templateFile;
    {
      bool argsGot = false;
      std::ifstream is{templateFilePath};
      if (is.is_open()) {
        std::string line{};
        while (std::getline(is, line)) {
          if (!argsGot) {
            argsGot = getArgs(line);
            continue;
          }
          htmlString += line;
          htmlString += "\n";
        }
      } else {
        throw std::runtime_error("can not open template file: " + templateFilePath);
      }
    }

    return std::pair<std::string, std::vector<std::string>>{htmlString, args};
  }



 public:
  const std::string templateName;
  const std::string htmlString;
  const std::vector<std::string> argsNameList;
  const std::vector<std::regex> argsPatternList{};

  HtmlTemplate(const std::string& templateName_, const std::string& template_,
               std::vector<std::string>& argsNameList_)
      : templateName(templateName_),
        htmlString(template_),
        argsNameList(argsNameList_),
        argsPatternList(patternInitHalper(argsNameList_)) {
    std::cout << "HtmlTemplate base construct: " << templateName << std::endl;
  }


  void genHtml(const std::vector<std::string>& args, std::string& outHtml) {
    if (args.size() != argsPatternList.size()) {
      throw std::runtime_error("args error for template[" + templateName + "]");
    } else {
      outHtml = htmlString;
      for (size_t i = 0; i < argsPatternList.size(); i++) {
        auto& p = argsPatternList.at(i);
        auto& value = args.at(i);
        outHtml = std::regex_replace(outHtml, p, value);
      }
    }
  }

  std::string genHtml(const std::vector<std::string>& args) {
    std::string outHtml{};
    genHtml(args, outHtml);
    return outHtml;
  }


  inline static std::shared_ptr<HtmlTemplate> loadTemplateToSharePtr(const std::string& templateFile) {
    std::pair<std::string, std::vector<std::string>> results = parseTemplate(templateFile);
    return std::make_shared<HtmlTemplate>(templateFile, results.first, results.second);
  }

  inline static HtmlTemplate loadTemplate(const std::string& templateFile) {
    std::pair<std::string, std::vector<std::string>> results = parseTemplate(templateFile);
    return HtmlTemplate(templateFile, results.first, results.second);
  }
};

// class Engine {
//  private:
//   Engine(){};
//
//   std::unordered_map<std::string, std::shared_ptr<HtmlTemplate>> templateMap{};
//
//   static Engine& getInstance() {
//     static Engine instance{};
//     return instance;
//   }
//
//   // std::shared_ptr<HtmlTemplate> getTemplateImp(const std::string& templateName) {
//   //   auto r = templateMap.find(templateName);
//   //   if (r != templateMap.end()) {
//   //     return r->second;
//   //   } else {
//   //     return std::shared_ptr<HtmlTemplate>{};
//   //   }
//   // };
//
//   std::shared_ptr<HtmlTemplate> loadTemplateImp(const std::string& templateName) {
//     //<!--Args(title)-->
//
//     static const std::regex argsLineRe{R"(<!--Args\((.*)\)-->)"};
//     static const std::regex winPathSplitorRe{R"(\\)"};
//
//     std::smatch sm;
//     std::vector<std::string> args;
//
//     auto getArgs = [&](std::string ln) -> bool {
//       if (std::regex_search(ln, sm, argsLineRe)) {
//         auto argsStr = sm.str(1);
//         // cout << "argsStr:" << argsStr << endl;
//         argsStr = seeker::String::removeBlanks(argsStr);
//         // cout << "argsStr processed:" << argsStr << endl;
//         args = seeker::String::split(argsStr, ",");
//         return true;
//       } else {
//         return false;
//       }
//     };
//
//     std::string htmlString{};
//     std::string templateFilePath = templateName;
//     {
//       bool argsGot = false;
//       std::ifstream is{templateFilePath};
//       if (is.is_open()) {
//         std::string line{};
//         while (std::getline(is, line)) {
//           if (!argsGot) {
//             argsGot = getArgs(line);
//             continue;
//           }
//           htmlString += line;
//           htmlString += "\n";
//         }
//       } else {
//         throw std::runtime_error("can not open template file: " + templateFilePath);
//       }
//     }
//
//     auto temp = std::make_shared<HtmlTemplate>(templateName, htmlString, args);
//     templateMap.insert_or_assign(templateName, temp);
//     return templateMap.at(templateName);
//   }
//
//  public:
//   Engine(const Engine&) = delete;
//   Engine& operator=(const Engine&) = delete;
//   ~Engine() { std::cout << "Engine destructed." << std::endl; }
//
//   static std::shared_ptr<HtmlTemplate> loadTemplate(const std::string& templateFile) {
//     auto& ins = getInstance();
//     return ins.loadTemplateImp(templateFile);
//   };
//
//   // static std::shared_ptr<HtmlTemplate> getTemplate(const std::string& templateName) {
//   //   auto& ins = getInstance();
//   //   return ins.getTemplateImp(templateName);
//   // }
// };

}  // namespace htmlPorter
