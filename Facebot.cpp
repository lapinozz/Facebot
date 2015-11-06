#include "Facebot.hpp"

#include <SFML/System.hpp>

Facebot::Facebot(std::string facebookEmail, std::string facebookPass, std::string ircUsername) : mIrcUsername(ircUsername)
{
    mExecutionClient.connect(sf::IpAddress("irc.boxbox.org"), 6667, ircUsername + "codeExecuter");

    f.login(facebookEmail, facebookPass);

    mMessaginClient.connect(sf::IpAddress("irc.boxbox.org"), 6667, ircUsername + "FbBot");
    mMessaginClient.setMsgCallback([this](std::string from, std::string channel, std::string message, int serverID)
    {
        if(serverID == 1)
            f.sendMessage(channel + " " + from + ": " + message, std::stoll(f.getUserID()), false);
        else if(ircActive && (message.size() < 4 || !(message.size() > 3 && message.substr(0, 3) == "***")))
        {
            for(auto& pair : mIrcClients)
            {
                if(pair.second.id == serverID)
                    f.sendMessage("." + message, pair.second.event.message.from, pair.second.event.message.isGroup);
            }
        }
    });
}

Facebot::~Facebot()
{
}

void Facebot::launch()
{
    while(true)
    {
        mExecutionClient.update();
        mMessaginClient.update();

        for(int x = 0; x < mToSay.size(); x++)
        {
            if(mToSay[x].first < time(NULL))
            {
                f.sendMessage(mToSay[x].second.message.body, mToSay[x].second.message.from, mToSay[x].second.message.isGroup);
                mToSay.erase(mToSay.begin() + x);
                break;
            }

        }

        Facechat::MessagingEvent event;
        while(f.pullMessage(event))
        {
            if(event.type == Facechat::MessagingEvent::MESSAGE)
            {
                if(event.message.from == std::stoll(f.getUserID()) && !event.message.isGroup)
                    event.message.from = event.message.to;
                else if(event.message.isGroup)
                    event.message.from = event.message.conversationID;

                if(isCommand(event, {"."}, false))
                {
                    if(isCommand(event, {".help"}, false))
                    {
                        removeCommandPart(event);

                        if(event.message.body.size())
                        {
                            if(event.message.body[0] != '.')
                                event.message.body.insert(0, ".");
                        }

                        std::string message;

                        if(isCommand(event, {".say"}, false))
                            message = "simple command that echo back. eg: .say text to be echoed";
                        else if(isCommand(event, {".g", ".google"}, false))
                            message = "Google search. eg: .google how to be as cool as lapinozz? \n\nImage search. eg: .google image cool image\n\n Facebook now block most link, it's possible that this command seem to not work";
                        else if(isCommand(event, {".y", ".youtube"}, false))
                            message = "Youtube search. eg: .youtube funny fail video\n\n Facebook now block most link, it's possible that this command seem to not work";
                        else if(isCommand(event, {".w", ".wiki", ".wikipedia"}, false))
                            message = "Wikipedia search. eg: .wiki proton\n\n Facebook now block most link, it's possible that this command seem to not work";
                        else if(isCommand(event, {".tr", ".translate"}, false))
                            message = "Translation command. eg: .tr traduit ce text en englais\n\nThe default target language is English you can change that like this: .tr fr This text will be translated in French";
                        else if(isCommand(event, {".def", ".define"}, false))
                            message = "Get Definition. eg: .define paradox";
                        else if(isCommand(event, {".rnd"}, false))
                            message = "Random, word, sentence or paragraph.\n\neg: .rnd\n\neg: .rnd sentence\n\neg: .rnd paragraph";
                        else if(isCommand(event, {".time"}, false))
                            message = "Give the current time. eg: .time";
                        else if(isCommand(event, {".userID", ".userId", ".userid"}, false))
                            message = "Get the facebook user id of someone. eg: .userId Marie Gagnon";
                        else if(isCommand(event, {".userInfo", ".userinfo"}, false))
                            message = "Get the facebook information of someone. eg: .userInfo Marie Gagnon";
                        else if(isCommand(event, {".tell"}, false))
                            message = "Send a mesage to someone. eg: .tell Marie Gagnon : you'r so exy today\n\n you can also send to multipel personne eg: .tell Marie Gagnon | Stephanie Gagnon : im not a cheater";
                        else if(isCommand(event, {".g++", ".gcc", ".nasm"}, false))
                            message = "execute code, yes this use cee from BoxBox :D";
                        else if(isCommand(event, {".choose"}, false))
                            message = "Randombly choose between multiple choice eg: .choose choice1|choice2|choice3";
                        else if(isCommand(event, {".irc"}, false))
                        {
                            message = "IRC commands\n";
                            message += "eg: .irc join #sfml\n";
                            message += "eg: .irc part #sfml\n";
                            message += "eg: .irc send #sfml im too cool for you\n";
                            message += "eg: .irc send Nyrox im too cool for you\n";
                            message += "Turn on Facebook to IRC eg: .irc on\n";
                            message += "Turn off Facebook to IRC eg: .irc off\n";
                        }
                        else if(isCommand(event, {".in"}, false))
                            message = "Send message after a certain time eg: .in 4m 30s tell me this\n\n can also send to someone else eg: .in 4m 30s Marie Gagnon: tell her this\n\n or even to mutiple personne eg: .in 4m 30s Marie Gagnon | Stephanie Gagnon: i already told you nothing happened between your sister and me";
                        else
                        {
                            message = "Command List: \n";
                            message += ".say\n";
                            message += ".g, .google\n";
                            message += ".y, .youtube\n";
                            message += ".w, .wiki, .wikipedia\n";
                            message += ".tr, .translate\n";
                            message += ".def, .define\n";
                            message += ".rnd\n";
                            message += ".time\n";
                            message += ".userID, .userId, .userid\n";
                            message += ".userInfo, .userinfo\n";
                            message += ".tell\n";
                            message += ".g++, .gcc, .nasm\n";
                            message += ".choose\n";
                            message += ".irc\n";
                            message += ".in\n";
                        }

                        f.sendMessage(message, event.message.from, event.message.isGroup);
                        f.deleteMessage(event.message.messageID);
                    }
                    else if(isCommand(event, {".say"}))
                    {
                        removeCommandPart(event);
                        f.sendMessage(event.message.body, event.message.from, event.message.isGroup);
                        f.deleteMessage(event.message.messageID);
                    }
                    else if(isCommand(event, {".g", ".google"}))
                    {
                        removeCommandPart(event);
                        bool image = isCommand(event, {"image"});
                        if(image)
                            removeCommandPart(event);

                        json j = googleSearch(event.message.body, image);

                        if(f.sendUrl(j["titleNoFormatting"], j["unescapedUrl"], event.message.from, event.message.isGroup) == "")
                            f.sendMessage("Facebook blocked direct URl here's pasted url\n" + j["unescapedUrl"].get<std::string>(), event.message.from, event.message.isGroup);

                        f.deleteMessage(event.message.messageID);
                    }
                    else if(isCommand(event, {".y", ".youtube"}))
                    {
                        removeCommandPart(event);
                        json j = youtubeSearch(event.message.body);

                        if(f.sendUrl(j["titleNoFormatting"], j["unescapedUrl"], event.message.from, event.message.isGroup) == "")
                            f.sendMessage("Facebook blocked direct URl here's pasted url\n" + j["unescapedUrl"].get<std::string>(), event.message.from, event.message.isGroup);

                        f.deleteMessage(event.message.messageID);
                    }
                    else if(isCommand(event, {".w", ".wiki", ".wikipedia"}))
                    {
                        removeCommandPart(event);
                        json j = wikipediaSearch(event.message.body);

                        if(f.sendUrl(j["titleNoFormatting"], j["unescapedUrl"], event.message.from, event.message.isGroup) == "")
                            f.sendMessage("Facebook blocked direct URl here's pasted url\n" + j["unescapedUrl"].get<std::string>(), event.message.from, event.message.isGroup);

                        f.deleteMessage(event.message.messageID);
                    }
                    else if(isCommand(event, {".tr", ".translate"}))
                    {
                        json j;
                        removeCommandPart(event);
                        std::string lang = "en";

                        if(event.message.body.size() >= 3 && (event.message.body[2] == ' '))
                        {
                            lang = event.message.body.substr(0, 2);
                            removeCommandPart(event);
                        }

                        j = translate(event.message.body, lang);

                        std::string message;
                        message = "From " + j["originalLanguage"].get<std::string>() + " to " + j["translateLanguage"].get<std::string>() + "\n" + j["translatedWord"].get<std::string>();
                        f.sendMessage(message, event.message.from, event.message.isGroup);
                        f.deleteMessage(event.message.messageID);
                    }
                    else if(isCommand(event, {".def", ".define"}))
                    {
                        removeCommandPart(event);

                        f.sendMessage(defineWord(event.message.body), event.message.from, event.message.isGroup);
                        f.deleteMessage(event.message.messageID);
                    }
                    else if(isCommand(event, {".rnd"}, false))
                    {
                        removeCommandPart(event);

                        RandomType type = WORD;
                        if(isCommand(event, {"sentence"}, false))
                            type = SENTENCE;
                        else if(isCommand(event, {"paragraph"}, false))
                            type = PARAGRAPH;

                        f.sendMessage(randomGenerator(type), event.message.from, event.message.isGroup);
                        f.deleteMessage(event.message.messageID);
                    }
                    else if(isCommand(event, {".time"}, false))
                    {
                        std::string message = "Current time: " + timestampToString(time(NULL));
                        f.sendMessage(message, event.message.from, event.message.isGroup);
                        f.deleteMessage(event.message.messageID);
                    }
                    else if(isCommand(event, {".userID", ".userId", ".userid"}))
                    {
                        removeCommandPart(event);

                        auto users = f.findUser(event.message.body);
                        std::string message;

                        if(users.size())
                            message = std::to_string(users[0].id);
                        else
                            message = "User not found";

                        f.sendMessage(message, event.message.from, event.message.isGroup);
                        f.deleteMessage(event.message.messageID);
                    }
                    else if(isCommand(event, {".userInfo", ".userinfo"}))
                    {
                        removeCommandPart(event);

                        std::string message;
                        UserID id = toUserID(event.message.body, f);

                        if(id)
                        {
                            Facechat::UserInfo user = f.getUserInfo(id);
                            message += user.completeName + "\n";

                            if(user.vanity.size())
                                message += user.vanity + "\n";

                            message += "Is a " + std::string(user.gender == 2 ? "guy" : "girl") + "\n";
                            message += std::to_string(user.id) + "\n";
                            message += "Is " + std::string(user.isFriend ? "" : "not ") + "your friend\n";

                            f.sendUrl("", user.profilePicture, event.message.from, event.message.isGroup);
                            f.sendUrl("", user.profileUrl, event.message.from, event.message.isGroup);
                        }
                        else
                            message = "User not found";

                        f.sendMessage(message, event.message.from, event.message.isGroup);
                        f.deleteMessage(event.message.messageID);
                    }
                    else if(isCommand(event, {".tell"}))
                    {
                        removeCommandPart(event);

                        std::string message;
                        int pos = event.message.body.find(':');
                        if(pos)
                        {
                            message = event.message.body.substr(pos + 1);
                            event.message.body.erase(pos);
                        }
                        else
                            message = "Enter name/id separeted by \'|\' then a \':\' followed by the message";


                        for(std::string& s : split(event.message.body, "|"))
                        {
                            UserID id = toUserID(s, f);
                            if(id)
                                f.sendMessage(message, id, event.message.isGroup);
                        }

                        f.deleteMessage(event.message.messageID);
                    }
                    else if(isCommand(event, {".g++", ".gcc", ".nasm"}))
                    {
                        for(std::string s : split(replaceAll(executeProgram(event.message.body), "#", "."), "|"))
                            f.sendMessage(s, event.message.from, event.message.isGroup);
                    }
                    else if(isCommand(event, {".choose"}))
                    {
                        removeCommandPart(event);

                        std::vector<std::string> choices = split(event.message.body, "|");

                        std::string message = "My choices: ";
                        for(std::string& s : choices)
                            message += s + ", ";
                        message.erase(message.size() - 2);

                        message += "\nI choose: " + choices[rand() % choices.size()];

                        f.sendMessage(message, event.message.from, event.message.isGroup);
                        f.deleteMessage(event.message.messageID);
                    }
                    else if(isCommand(event, {".irc"}))
                    {
                        removeCommandPart(event);

                        if(isCommand(event, {"join", "JOIN"}))
                            mMessaginClient.send(event.message.body, 1);
                        else if(isCommand(event, {"part", "PART"}))
                            mMessaginClient.send(event.message.body, 1);
                        else if(isCommand(event, {"on", "ON"}, false))
                            ircActive = true;
                        else if(isCommand(event, {"off", "OFF"}, false))
                            ircActive = false;
                        else if(isCommand(event, {"send"}))
                        {
                            removeCommandPart(event);

                            std::string sendTo = event.message.body.substr(0, event.message.body.find(' '));
                            removeCommandPart(event);

                            if(sendTo[0] == '#')
                                mMessaginClient.join(sendTo, 1);

                            mMessaginClient.send("PRIVMSG " + sendTo + " "  + event.message.body, 1);
                        }
                    }
                    else if(isCommand(event, {".in"}))
                    {

                        removeCommandPart(event);

                        time_t in = time(NULL);

                        while(isdigit(event.message.body[0]))
                        {
                            char type = event.message.body[event.message.body.find(' ') - 1];
                            int num = std::atoi(event.message.body.substr(0, event.message.body.find(' ') - 1).c_str());

                            if(isdigit(type))
                                break;

                            if(type == 's' || type == 'S')
                                in += num;
                            else if(type == 'm' || type == 'M')
                                in += num * 60;
                            else if(type == 'h' || type == 'H')
                                in += num * 60 * 60;
                            else if(type == 'd' || type == 'D')
                                in += num * 60 * 60 * 24;

                            removeCommandPart(event);
                        }

                        std::string timeString = timestampToString(in, true);
                        if(timeString.size() > 3 && (timeString.substr(timeString.size() - 3) == "ago"))
                            timeString = "in " + timeString.substr(0, timeString.size() - 4);

                        std::string message;
                        int pos = event.message.body.find(':');
                        if(pos != std::string::npos)
                        {
                            message = event.message.body.substr(pos + 1);
                            event.message.body.erase(pos);

                            for(std::string& s : split(event.message.body, "|"))
                            {
                                UserID id = toUserID(s, f);
                                if(id)
                                {
                                    Facechat::MessagingEvent toSayEvent = event;
                                    toSayEvent.message.from = id;
                                    toSayEvent.message.body = message;
                                    mToSay.push_back(std::pair<time_t, Facechat::MessagingEvent>(in, toSayEvent));
                                }
                            }

                            message = "Ok, i'll tell them ";
                        }
                        else
                        {
                            mToSay.push_back(std::pair<time_t, Facechat::MessagingEvent>(in, event));
                            message = "Ok, i'll tell you ";
                        }

                        f.sendMessage(message + timeString, event.message.from, event.message.isGroup);
                        f.deleteMessage(event.message.messageID);
                    }
                }
                else if(ircActive)
                {
                    if(mIrcClients.find(event.message.from) == mIrcClients.end())
                    {
                        if(event.message.isGroup)
                            event.message.senderName = event.message.group.groupName;
                        else
                            event.message.senderName = f.getUserInfo(event.message.from).completeName;

                        if(event.message.senderName.empty())
                            event.message.senderName = "group_" + std::to_string(event.message.conversationID);

                        event.message.senderName = replaceAll(event.message.senderName, " ", "_");
                        event.message.senderName = replaceAll(event.message.senderName, "é", "e");
                        event.message.senderName = replaceAll(event.message.senderName, "è", "e");
                        event.message.senderName = replaceAll(event.message.senderName, ".", "");
                        event.message.senderName = replaceAll(event.message.senderName, ",", "");

                        mIrcClients[event.message.from].event = event;
                        mIrcClients[event.message.from].id = mMessaginClient.connect(sf::IpAddress("irc.boxbox.org"), 6667, event.message.senderName);
                    }

                    mMessaginClient.send("PRIVMSG " + mIrcUsername + " " + event.message.senderName + ": " + event.message.body, mIrcClients[event.message.from].id);
                }
            }
        }
    }
}

std::string Facebot::executeProgram(std::string command)
{
    int id = 1;
    mExecutionClient.send("PRIVMSG cee " + command, id);

    bool stop = false;
    std::string user;
    std::string message = "Depased execution time limit";

    mExecutionClient.setMsgCallback([&stop, &user, &message](std::string t_user, std::string t_channel, std::string t_message, int t_id)
    {
        if(t_user != "cee")
            return;

        stop = true;
        user = t_user;
        message = t_message;
    });


    sf::Clock clock;
    clock.restart();

    while(!stop && clock.getElapsedTime() < sf::seconds(5))
        mExecutionClient.update();

    return message;
}
