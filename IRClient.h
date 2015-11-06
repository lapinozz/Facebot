#pragma once
#ifndef _IRC_CLIENT_
#define _IRC_CLIENT_

#include "IDPool.h"

#include <SFML/Network.hpp>

namespace IRC
{
enum Commands
{
    RPL_WELCOME       =   1, //:Welcome to the Internet Relay Network <nick>!<user>@<host>
    RPL_YOURHOST      =   2, //:Your host is <servername>, running version <version>
    RPL_CREATED       =   3, //:This server was created <date>
    RPL_MYINFO        =   4, //<server_name> <version> <user_modes> <chan_modes>
    RPL_ISUPPORT      =   5, //http://www.irc.org/tech_docs/005.html

    RPL_LUSERCLIENT   = 251,
    RPL_LUSEROP       = 252,
    RPL_LUSERUNKNOWN  = 253,
    RPL_LUSERCHANNELS = 254,
    RPL_LUSERME       = 255,

    RPL_LOCALUSERS    = 265,
    RPL_GLOBALUSERS   = 266,

    RPL_TOPICWHOTIME  = 333, //<channel> <setter> <timestamp>
    RPL_NAMREPLY      = 353,
    RPL_ENDOFNAMES    = 366,

    RPL_MOTD          = 372,
    RPL_MOTDSTART     = 375,
    RPL_ENDOFMOTD     = 376,

    PRIVMSG,
    NOTICE,

    JOIN,
    PART,

    UNDEFINED
};

enum Usermodes
{
    UNKNOWN,

    MODE_IDONTKNOW_1 = '@',
    MODE_IDONTKNOW_2 = '&',
    MODE_IDONTKNOW_3 = '%',
    MODE_IDONTKNOW_4 = '+'
};

Commands toCommand(std::string& command);

Usermodes toUsermode(char& character);

const char defaultChannelChar = '#';
bool isChannelChar(char& c);

}

class IRClient
{

public:
    struct User
    {
        IRC::Usermodes mode = IRC::UNKNOWN;
        std::string nickname;
    };

    struct Channel
    {
        std::map<std::string, User> users;
    };

    struct Server
    {
        sf::TcpSocket* socket;
        std::string recieveBuffer = "";

        std::map<std::string, Channel> channel;

        bool ready = false;
        std::vector<std::string> toSend;
    };

    IRClient();
    ~IRClient();

    unsigned int connect(sf::IpAddress server, unsigned short port, std::string nick, std::string pass = "");
    void join(std::string channel, int id);

    void update();

    void send(std::string msg, int id, int forceSend = 0);
    void sendMessage(std::string msg, std::string to, int id);
    
    void setMsgCallback(std::function<void(std::string, std::string, std::string, int)> callback);


private:

    sf::Thread inputThread;
    void consoleInput(); // temporary (i hope lel)

    IDPool m_pool;

    void handleMsg(std::string s, int id);
    
    std::function<void(std::string, std::string, std::string, int)> mMsgCallback;

    std::map<int, Server> m_servers;
};


#endif
