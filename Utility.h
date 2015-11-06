#pragma once

#include "Facechat/stdinclude.hpp"
#include "Facechat/Facechat.h"

#include <queue>

json googleSearch(std::string search, bool image = false);
json youtubeSearch(std::string search);
json wikipediaSearch(std::string search);

json translate(std::string toTranslate, std::string targetLanguage = "en");
std::string defineWord(std::string word);

enum RandomType {WORD, SENTENCE, PARAGRAPH};
std::string randomGenerator(RandomType type = WORD);

bool isCommand(Facechat::MessagingEvent& event, std::initializer_list<std::string> commands, bool requireSpace = true);
void removeCommandPart(Facechat::MessagingEvent& event);

UserID toUserID(const std::string& nameOrID, Facechat& f);

std::vector<std::string> split(const std::string &s, const std::string& delim);
std::vector<std::string> &split(const std::string &s, const std::string& delim, std::vector<std::string> &elems);

std::string urlEncode(std::string str);
std::string urlDecode(std::string str);
