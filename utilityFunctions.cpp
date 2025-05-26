#include "fleetXpress.h"
#include <iostream>
#include <fstream>
#include <json/json.h>
#include <openssl/sha.h>
#include <iomanip>
#include <sstream>

using namespace std;

string sha256(const string& str)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(str.c_str()), str.size(), hash);

    stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
    {
        ss << hex << setw(2) << setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

enum UserType authenticate(string username, string password)
{
    ifstream userFile("userData.json");
    Json::Value userData;
    userFile >> userData;

    string hashedPassword = sha256(password);

    for (const auto& user : userData)
    {
        if (user["loginname"].asString() == username && user["pwSHA256ChkSum"].asString() == hashedPassword)
        {
            if (user["role"].asString() == "admin")
            {
                return UserType::Admin;
            }
            else if (user["role"].asString() == "user")
            {
                return UserType::User;
            }
        }
    }

    return UserType::None;
}
