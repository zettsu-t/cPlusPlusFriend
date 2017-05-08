set MINGWPATH=C:\MinGW\
set PATH=%MINGWPATH%bin;%MINGWPATH%lib;%PATH%
g++ -std=gnu++14 -Wall -I%MINGWPATH%include -DCPPFRIENDS_REGEX_BUILD_STAND_ALONE -o cppFriendsSpace cppFriendsSpace.cpp -lboost_regex
cppFriendsSpace
set /P DUMMY="done"
