#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>


#ifdef _WIN32
constexpr char PATH_LIST_SEPARATOR = ';';
#else
constexpr char PATH_LIST_SEPARATOR = ':';
#endif

std::vector<std::string> builtins = {"exit", "type", "echo"};

std::vector<std::string> tokenize(const std::string& input) {
    std::istringstream iss(input);
    std::vector<std::string> tokens;
    std::string tok;
    while (iss >> tok) tokens.push_back(tok);
    return tokens;
}

std::vector<std::string> splitPathList(const std::string& pathList, char delimiter) {
    std::vector<std::string> directories;
    std::stringstream ss(pathList);
    std::string directory;

    while(std::getline(ss, directory, delimiter)) {
        if(!directory.empty()) {
            directories.push_back(directory);
        }
    }
    return directories;
}

bool isBuiltin(const std::string& cmd) {
    for (const auto& b : builtins) {
        if (b == cmd) return true;
    }
    return false;
}

bool isExecutable(const std::string& path) {
    return access(path.c_str(), X_OK) == 0;
}

int main() {
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    std::string input;
    while (true) {

        std::cout << "$ ";
        if (!std::getline(std::cin, input)) break;

        std::vector<std::string> args = tokenize(input);
        if (args.empty()) continue;

        const std::string& cmd = args[0];

        if (cmd == "exit") break;

        if (cmd == "echo") {
            for (size_t i = 1; i < args.size(); ++i) {
                std::cout << args[i] << (i + 1 < args.size() ? " " : "");
            }
            std::cout << "\n";
            continue;
        }

        if (cmd == "type") {
            if (args.size() < 2) continue;
            
            const char* env_path = std::getenv("PATH");
            if(!env_path) {
                std::cout << "PATH not found" << "\n";
                continue;
            }
            
            for(size_t i=1; i<args.size(); i++) {
                if (isBuiltin(args[i])) {
                    std::cout << args[i] << " is a shell builtin\n";
                    continue;
                }
                
                bool foundPATHCmd = false;
                std::vector<std::string> allDirectories = splitPathList(env_path, PATH_LIST_SEPARATOR);
                for(std::string directory : allDirectories) {
                    std::string fullPath = directory + "/" + args[i];
                    if(isExecutable(fullPath)) {
                        std::cout << args[i] << " is " << fullPath << "\n";
                        foundPATHCmd = true;
                        break;
                    }
                }
                if(!foundPATHCmd) std::cout << args[i] << ": not found\n";
        
            }
            continue;
        }

        std::cout << cmd << ": command not found\n";
    }
}