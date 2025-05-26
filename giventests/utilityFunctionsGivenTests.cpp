#include "../utilityFunctions.h"
#include <gtest/gtest.h>
#include <vector>

TEST(TestAuthentication, RegularUser)
{
    vector<string> creds = {"kara", "shiva", "arya", "kanchen"};
    for (auto cred: creds)
    {
        string str = cred + "\nTrajenta500!\n";
        istringstream input(str);
        streambuf* origCinBuf = cin.rdbuf(input.rdbuf());
        testing::internal::CaptureStdout();
        UserType actualUserType = authenticate("UserData.json");
        cin.rdbuf(origCinBuf);
        string output = testing::internal::GetCapturedStdout();
        ASSERT_EQ(user, actualUserType);
    }
}

TEST(TestAuthentication, AdminUser)
{
    vector<string> creds = {"koram", "lik", "bhatta", "junga"};
    for (auto cred: creds)
    {
        string str = cred + "\nVolix250!\n";
        istringstream input(str);
        streambuf* origCinBuf = cin.rdbuf(input.rdbuf());
        testing::internal::CaptureStdout();
        UserType actualUserType = authenticate("UserData.json");
        cin.rdbuf(origCinBuf);
        string output = testing::internal::GetCapturedStdout();
        ASSERT_EQ(admin, actualUserType);
    }
}

TEST(TestAuthentication, InvalidUser)
{
    istringstream input("david\nEdwin100!\n");
    streambuf* origCinBuf = cin.rdbuf(input.rdbuf());
    testing::internal::CaptureStdout();
    UserType actualUserType = authenticate("UserData.json");
    cin.rdbuf(origCinBuf);
    string output = testing::internal::GetCapturedStdout();
    ASSERT_EQ(invalid, actualUserType);
}