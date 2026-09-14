#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <optional>
#include <sys/wait.h>
#include <cstdlib>
#include <filesystem>


#ifdef _WIN32
constexpr char PATH_LIST_SEPARATOR = ';';
#else
constexpr char PATH_LIST_SEPARATOR = ':';
#endif

std::vector<std::string> builtins = {"exit", "type", "echo", "pwd", "cd"};

std::vector<std::string> tokenize(const std::string& input) {
    std::vector<std::string> tokens;
    std::string current;
    bool inToken = false;
    bool inSingleQuotes = false;
    bool inDoubleQuotes = false;

    for (size_t i = 0; i < input.size(); ++i) {
        char c = input[i];

        if (inSingleQuotes) {
            if (c == '\'') {
                inSingleQuotes = false;
            } else {
                current += c;
            }
            continue;
        }

        if (inDoubleQuotes) {
            if (c == '"') {
                inDoubleQuotes = false;
            } else if (c == '\\' && i + 1 < input.size() &&
                       (input[i + 1] == '"' || input[i + 1] == '\\' ||
                        input[i + 1] == '$' || input[i + 1] == '`')) {
                current += input[i + 1];
                ++i;
            } else {
                current += c;
            }
            continue;
        }

        if (c == '\'') {
            inSingleQuotes = true;
            inToken = true;
            continue;
        }

        if (c == '"') {
            inDoubleQuotes = true;
            inToken = true;
            continue;
        }

        if (c == '\\' && i + 1 < input.size()) {
            current += input[i + 1];
            inToken = true;
            ++i;
            continue;
        }

        if (std::isspace(static_cast<unsigned char>(c))) {
            if (inToken) {
                tokens.push_back(current);
                current.clear();
                inToken = false;
            }
            continue;
        }

        current += c;
        inToken = true;
    }

    if (inToken) {
        tokens.push_back(current);
    }

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

std::optional<std::string> getExecutablePath(std::string arg) {
    const char* env_path = std::getenv("PATH");
    if(!env_path) {
        std::cout << "PATH not found" << "\n";
        return std::nullopt;
    }

    std::vector<std::string> allDirectories = splitPathList(env_path, PATH_LIST_SEPARATOR);
    for(std::string directory : allDirectories) {
        std::string fullPath = directory + "/" + arg;
        if(isExecutable(fullPath)) {
            return fullPath;
        }
    }
    return std::nullopt;
}

std::vector<char*> buildArgv(const std::vector<std::string>& args) {
    std::vector<char*> argv;
    argv.reserve(args.size() + 1);
    for (const auto& arg : args) {
        argv.push_back(const_cast<char*>(arg.c_str()));
    }
    argv.push_back(nullptr);
    return argv;
}

void executeExternalCommand(const std::string& fullPath, std::vector<char*> argv) {
    pid_t pid = fork();

    if (pid == 0) {
        execv(fullPath.c_str(), argv.data());
        perror("execv");
        std::exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    } else {
        perror("fork");
    }
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

        std::vector<char*> argv = buildArgv(args);

        std::filesystem::path currentWorkingDirectory = std::filesystem::current_path();

        const std::string& cmd = args[0];

        if (cmd == "exit") break;

        if (cmd == "echo") {
            for (size_t i = 1; i < args.size(); ++i) {
                std::cout << args[i] << (i + 1 < args.size() ? " " : "");
            }
            std::cout << "\n";
            continue;
        }

        if(cmd == "pwd") {
            std::cout << currentWorkingDirectory.string() << "\n";
            continue;
        }

        if(cmd == "cd") {
            if(args.size() < 2) {
                const char* home = std::getenv("HOME");
                if(home) std::filesystem::current_path(home);
                continue;
            }

            std::string newPath = args[1];
            if(!newPath.empty() && newPath[0] == '~') {
                const char* home = std::getenv("HOME");
                if(home) newPath = std::string(home) + newPath.substr(1);
            }

            try {
                std::filesystem::current_path(newPath);
            }
            catch(const std::filesystem::filesystem_error& err) {
                std::cout << "cd: " << newPath << ": No such file or directory\n";
            }
            continue;
        }

        if (cmd == "type") {
            if (args.size() < 2) continue;
            
            for(size_t i=1; i<args.size(); i++) {
                if (isBuiltin(args[i])) {
                    std::cout << args[i] << " is a shell builtin\n";
                    continue;
                }
            
                auto fullPath = getExecutablePath(args[i]);
                if(fullPath) {
                    std::cout << args[i] << " is " << *fullPath << "\n";
                }
                else {
                    std::cout << args[i] << ": not found\n";
                }
            }
            continue;
        }

        auto fullPath = getExecutablePath(cmd);
        if (fullPath) {
            executeExternalCommand(*fullPath, argv);
        } else {
            std::cout << cmd << ": command not found\n";
        }
    }
}