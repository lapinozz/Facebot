#ifndef FACEBOT_HPP
#define FACEBOT_HPP

#include "Utility.h"

#include "IRClient.h"

class Facebot
{

public:
    Facebot(std::string facebookEmail, std::string facebookPass, std::string ircUsername = "");
    ~Facebot();
    
    void launch();
    
    std::string executeProgram(std::string command);

private:
    Facechat f;
    
    std::string mIrcUsername;
    
    IRClient mExecutionClient;
    IRClient mMessaginClient;
    struct Client
    {
        int id;
        Facechat::MessagingEvent event;
    };
    std::map<UniversalID, Client> mIrcClients;
    bool ircActive = true;
    
    std::vector<std::pair<time_t, Facechat::MessagingEvent>> mToSay;
};

#endif // FACEBOT_HPP
    
