#include "IRClient.h"

#include "Utility.h"

namespace IRC
{

#define TO_COMMAND_MACRO(cmdEnum, cmdCompare) if(command == cmdCompare) return cmdEnum;

Commands toCommand(std::string& command)
{
    TO_COMMAND_MACRO(RPL_WELCOME, "001")
    TO_COMMAND_MACRO(RPL_YOURHOST, "002")
    TO_COMMAND_MACRO(RPL_CREATED, "003")
    TO_COMMAND_MACRO(RPL_MYINFO, "004")
    TO_COMMAND_MACRO(RPL_ISUPPORT, "005")

    TO_COMMAND_MACRO(RPL_LUSERCLIENT, "251")
    TO_COMMAND_MACRO(RPL_LUSEROP, "252")
    TO_COMMAND_MACRO(RPL_LUSERUNKNOWN, "253")
    TO_COMMAND_MACRO(RPL_LUSERCHANNELS, "254")
    TO_COMMAND_MACRO(RPL_LUSERME, "255")

    TO_COMMAND_MACRO(RPL_LOCALUSERS, "265")
    TO_COMMAND_MACRO(RPL_GLOBALUSERS, "266")

    TO_COMMAND_MACRO(RPL_TOPICWHOTIME, "333")
    TO_COMMAND_MACRO(RPL_NAMREPLY, "353")
    TO_COMMAND_MACRO(RPL_ENDOFNAMES, "366")

    TO_COMMAND_MACRO(RPL_MOTD, "372")
    TO_COMMAND_MACRO(RPL_MOTDSTART, "375")
    TO_COMMAND_MACRO(RPL_ENDOFMOTD, "376")

    TO_COMMAND_MACRO(PRIVMSG, "PRIVMSG")
    TO_COMMAND_MACRO(NOTICE, "NOTICE")

    TO_COMMAND_MACRO(JOIN, "JOIN")
    TO_COMMAND_MACRO(PART, "PART")

    return UNDEFINED;
}

#define USER_MODE_MACRO(mode) if(character == mode) return mode;

Usermodes toUsermode(char& character)
{
    USER_MODE_MACRO(MODE_IDONTKNOW_1)
    USER_MODE_MACRO(MODE_IDONTKNOW_2)
    USER_MODE_MACRO(MODE_IDONTKNOW_3)
    USER_MODE_MACRO(MODE_IDONTKNOW_4)

    return UNKNOWN;
}

bool isChannelChar(char& c)
{
    return c == defaultChannelChar;
}
};

using namespace IRC;

IRClient::IRClient() : m_pool(10, 5), inputThread(std::bind(&IRClient::consoleInput, this))
{
//    inputThread.launch();

    mMsgCallback = [](std::string s1, std::string s2, std::string s3, int id){};

}

IRClient::~IRClient()
{
    for(std::pair<const int, Server>& pair : m_servers)
        delete pair.second.socket;
}

unsigned int IRClient::connect(sf::IpAddress server, unsigned short port, std::string nick, std::string pass)
{
    sf::TcpSocket* socket = new sf::TcpSocket;
    if (socket->connect(server, port, sf::seconds(60)) == sf::Socket::Done)
    {
        unsigned int id = m_pool.pullID();
        m_servers[id].socket = socket;
        socket->setBlocking(false);

        send("PASS " + pass == "" ? "blank" : pass, id, 1);
        send("NICK " + nick, id, 1);
        send("USER " + nick + " blank blank iCantRememberWhatThisIs", id, 1);
        return id;
    }
    else
        delete socket;

    return -1;
}

void IRClient::join(std::string channel, int id)
{
    if(!IRC::isChannelChar(channel[0]))
        channel.insert(0, &IRC::defaultChannelChar);

    m_servers[id].channel[channel] = Channel();
    send("JOIN " + channel, id);
}

void IRClient::update()
{
#define MSG_SIZE 500
    char msg[MSG_SIZE];
    for (auto& pair : m_servers)
    {
        sf::TcpSocket& s(*pair.second.socket);
        unsigned int size;
        if (s.receive(msg, MSG_SIZE, size) == sf::Socket::Done)
        {
            std::string& buff(m_servers[pair.first].recieveBuffer);
            buff += std::string(msg, size);
            //            buff += "\n";//that's just really wrong
            while (buff.find("\n") != std::string::npos)
            {
                handleMsg(buff.substr(0, buff.find("\n")), pair.first);
                buff.erase(0, buff.find("\n") + 1);
            }
        }

        if (pair.second.ready)
        {
            for (std::string& s : pair.second.toSend)
                send(s, pair.first);

            pair.second.toSend.clear();
        }
    }
}

void IRClient::handleMsg(std::string s, int id)
{

    std::vector<std::string> tokens;
    split(s, " ", tokens);
    
    if (tokens[0] == "PING")
        send("PONG " + tokens[1], id, 1);
    else
    {
        std::string message;

        std::string user = tokens[0]; //host name or nickname
        user.erase(0, 1); // remove the ":" at the start
        tokens.erase(tokens.begin());

        IRC::Commands command = IRC::toCommand(tokens[0]);
        tokens.erase(tokens.begin());

        if (IRC::RPL_ENDOFMOTD == command)
            m_servers[id].ready = 1;
        else if(IRC::PRIVMSG == command || NOTICE == command)
        {
            std::string from = tokens[0]; //sender

            if(PRIVMSG == command)
            {
                if(IRC::isChannelChar(from[0])) //channel message
                    message += "On " + from + " ";
                else //private message
                    message += "In Private ";
            }
            
            std::string channel = from;
            from = user;

            int delimiter = from.find("!");
            if(delimiter != std::string::npos)
                from.erase(delimiter); //only keep user name

            if(PRIVMSG == command)
                message += "From " + from + ": ";
            else if(NOTICE == command)
                message += from + " Whisper: ";

            message += s.substr(s.find(tokens[0]) + tokens[0].length() + 2); //temporary
            mMsgCallback(from, channel, s.substr(s.find(tokens[0]) + tokens[0].length() + 2), id);
        }
        else if(IRC::RPL_NAMREPLY == command)
        {
            tokens.erase(tokens.begin()); //our username
            tokens.erase(tokens.begin()); //unused token

            Channel& channel = m_servers[id].channel[tokens[0]]; //user from what channel
            std::string channelName = tokens[0];

            for(std::string& nickname : split(s.substr(s.find(channelName) + channelName.length() + 2), " ")) //Iterate through nickname
            {
                IRC::Usermodes mode = IRC::toUsermode(nickname[0]);
                if(mode != IRC::UNKNOWN)
                    nickname.erase(0, 1);

                User& user = channel.users[nickname];
                user.nickname = nickname;
                user.mode = mode;

                message += nickname; //temporary
                if(mode != IRC::UNKNOWN)
                    message += mode;
                message += " ";
            }
        }
        else if(JOIN == command || PART == command)
        {
            if(tokens[0][0] == ':')
                tokens[0].erase(0, 1);

            std::string nickname = user.substr(0, user.find("!"));

            if(JOIN == command)
            {
                User& user = m_servers[id].channel[tokens[0]].users[nickname];
                user.nickname = nickname;
            }
            else if(PART == command)
                m_servers[id].channel.erase(nickname);

            message += nickname + (JOIN == command ? " has arrived on " : "has quite ") + tokens[0];

        }
        else if((command >= RPL_WELCOME && command <= RPL_ISUPPORT) ||
                (command >= RPL_LUSERCLIENT && command <= RPL_LUSERME) ||
                RPL_MOTD == command || RPL_MOTDSTART == command)
        {
            message += "<" + user + "> "; //Server
            message += s.substr(s.find(tokens[0]) + tokens[0].length() + 1);
        }
        else if(IRC::UNDEFINED == command)
            message = s;

//        std::cout << message << std::endl;
//        std::cout << "msg: " << s << std::endl;
    }
}

void IRClient::send(std::string msg, int id, int forceSend)
{
    if (!m_servers[id].ready && !forceSend)
    {
        m_servers[id].toSend.push_back(msg);
        return;
    }

//    std::cout << "Sending: " << msg << std::endl;

    msg += "\r\n";
    m_servers[id].socket->send(msg.c_str(), msg.size());

//    const std::size_t size = msg.size() + 2; //this code look a lot more bad ass :p
//    char* buffer = new char[size];          // plus it's more low-level, you should prefer it, like me :p
//    for (int x = 0; x < size - 2; x++)
//        buffer[x] = msg[x];
//
//    buffer[size - 2] = '\r';
//    buffer[size - 1] = '\n';

//    m_servers[id].socket->send(buffer, size);
//    delete buffer;
}

void IRClient::sendMessage(std::string msg, std::string to, int id)
{
    send("PRIVMSG " + to + " :" + msg, id);
}

void IRClient::consoleInput()
{
    while (true)
    {
        std::string buf("");
        std::getline(std::cin, buf);
        send(buf, 1);
    }
}

void IRClient::setMsgCallback(std::function<void(std::string, std::string, std::string, int)> callback)
{
    mMsgCallback = callback;
}
