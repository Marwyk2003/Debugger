#ifndef UTILZ
#define UTILZ
#include <string>

void deleteContentOfDir(const std::string& path);
void createStyles(const std::string& path);
void createIndex(const std::string& path);
std::string getDebugDir();

#endif // UTILZ