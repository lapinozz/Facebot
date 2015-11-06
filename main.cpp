#include <stdio.h>
#include "Facebot.hpp"

#include "Utility.h"

int main(int argc, char** argv)
{
    srand(time(NULL));
    
    Facebot f("YOUR_FB_EMAIL_HERE", "YOUR_FB_PASWORD_HERE", "YOUR_IRC_USERNAME_HERE");
    f.launch();

    return 0;
}
