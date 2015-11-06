#include "Utility.h"

json googleSearch(std::string search, bool image)
{
    cpr::Response r = cpr::Post(cpr::Url("https://ajax.googleapis.com/ajax/services/search/" + std::string(image ? "images" : "web") + "?v=1.0&q=" + urlEncode(search)));
    return json::parse(r.text)["responseData"]["results"][0];
}

json wikipediaSearch(std::string search)
{
    return googleSearch("site:wikipedia.org " + search);
}

json youtubeSearch(std::string search)
{
    return googleSearch("site:youtube.com " + search);
}

json translate(std::string toTranslate, std::string targetLanguage)
{
    cpr::Response r = cpr::Post(cpr::Url("translate.google.ca/translate_a/single?client=t&ie=UTF-8&oe=UTF-8&source=btn&srcrom=1&ssel=3&tsel=0&kc=0&tk=437104|35577&sl=auto&tl=" + targetLanguage + "&hl=fr&dt=t&q=" + urlEncode(toTranslate)),
    cpr::Header {{"User-Agent", "Mozilla/5.0 (Wihttps://www.facebook.com/ajax/mercury/threadlist_info.phpndows NT 5.1; rv:31.0) Gecko/20100101 Firefox/31.0"}});

    json j;
    j["originalWord"] = toTranslate;
    j["translatedWord"] = r.text.substr(4, r.text.find("\"", 4) - 4);

    size_t pos = r.text.find("\"", 4 + j["translatedWord"].get<std::string>().size() + 3 + toTranslate.size() + 1) + 1;
    j["originalLanguage"] = r.text.substr(pos, r.text.find("\"", pos) - pos);
    j["translateLanguage"] = targetLanguage;

//    std::cout << toTranslate << std::endl;
//    std::cout << r.text << std::endl;
//    std::cout << r.text << std::endl;
//    std::cout << j.dump(4) << std::endl;

    return j;
}

std::string defineWord(std::string word)
{
    cpr::Response r = cpr::Post(cpr::Url("http://services.aonaware.com/DictService/DictService.asmx/Define?word=" + urlEncode(word)));

    int pos;
    int defCount = 0;
    std::string result;

    r.text = replaceAll(r.text, "\n", "");
    r.text = replaceAll(r.text, "http://", "");
    r.text = replaceAll(r.text, "https://", "");
    r.text = replaceAll(r.text, "www", "");
    r.text = replaceAll(r.text, ".com", "");
    r.text = replaceAll(r.text, ".org", "");
    r.text = replaceAll(r.text, ".edu", "");
    r.text = replaceAll(r.text, ".unm", "");
    r.text = replaceAll(r.text, ".htm", "");
    r.text = replaceAll(r.text, ".html", "");

    while((pos = r.text.find("  ")) != std::string::npos)
        r.text = replaceAll(r.text, "  ", " ");

    while((pos = r.text.find("<WordDefinition>")) != std::string::npos)
    {
        pos += 16;
        result += "Definition #" + std::to_string(++defCount) + "\n" + r.text.substr(pos, r.text.find("</WordDefinition>") - pos) + "\n\n";
        r.text.erase(pos - 16, r.text.find("</WordDefinition>") - pos + 33);
    }

    if(defCount == 0)
        result = "Word \'" + word + "\' not found";

    return result;
}

std::string randomGenerator(RandomType type)
{
    std::string typeString;
    cpr::Payload payloads {{}};

    if(type == WORD)
    {
        typeString = "RandomWord";
        payloads = cpr::Payload {{"LastWord", ""}};
    }
    else if(type == SENTENCE)
    {
        typeString = "NewRandomSentence";
    }
    else if(type == PARAGRAPH)
    {
        typeString = "RandomParagraph";
        payloads = cpr::Payload {{"Subject1", ""},{"Subject2", ""}};
    }

    cpr::Response r = cpr::Post(cpr::Url("http://watchout4snakes.com/wo4snakes/Random/" + typeString),
                                payloads);

    return r.text;
}

bool isCommand(Facechat::MessagingEvent& event, std::initializer_list<std::string> commands, bool requireSpace)
{
    for(const std::string& command : commands)
    {
        if((event.message.body.size() >= (command.size() + (requireSpace ? 1 : 0)))
                && (event.message.body.substr(0, command.size()) == command)
                && (!requireSpace || (event.message.body.substr(command.size(), 1) == " "))
          )
            return true;
    }
    return false;
}

void removeCommandPart(Facechat::MessagingEvent& event)
{
    event.message.body.erase(0, event.message.body.find(" ") + 1);
}

UserID toUserID(const std::string& nameOrID, Facechat& f)
{
    UserID id = atoll(nameOrID.c_str());
    if(id == 0)
    {
        auto users = f.findUser(nameOrID);
        if(users.size())
            id = users[0].id;
    }

    return id;
}

std::vector<std::string> split(const std::string &s, const std::string& delim)
{
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

std::vector<std::string> &split(const std::string &s, const std::string& delim, std::vector<std::string> &elems)
{
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim[0]))
    {
        elems.push_back(item);
    }
    return elems;
}

std::string urlEncode(std::string str)
{
    std::string new_str = "";
    char c;
    int ic;
    const char* chars = str.c_str();
    char bufHex[10];
    int len = strlen(chars);

    for(int i = 0; i < len; i++)
    {
        c = chars[i];
        ic = c;

        if(isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
            new_str += c;
        else
        {
            sprintf(bufHex, "%X", c);
            if(ic < 16)
                new_str += "%0";
            else
                new_str += "%";
            new_str += bufHex;
        }
    }
    return new_str;
}

std::string urlDecode(std::string str)
{
    std::string ret;
    char ch;
    int i, ii, len = str.length();

    for(i = 0; i < len; i++)
    {
        if(str[i] != '%')
        {
            if(str[i] == '+')
                ret += ' ';
            else
                ret += str[i];
        }
        else
        {
            sscanf(str.substr(i + 1, 2).c_str(), "%x", &ii);
            ch = static_cast<char>(ii);
            ret += ch;
            i = i + 2;
        }
    }
    return ret;
}
