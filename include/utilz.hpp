#ifndef UTILZ
#define UTILZ
#include <string>

std::string getOutputPath();
void deleteContentOfDir(const std::string &path);
void createStyles(const std::string &path);
void createIndex(const std::string &path);

#endif // UTILZ