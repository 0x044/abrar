#ifndef UTILITYFUNCTIONS_H
#define UTILITYFUNCTIONS_H
#include <string>
using namespace std;
enum UserType { user, admin, invalid } ;

enum UserType authenticate(string dataFile);

#endif