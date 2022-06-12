#include "task.hpp"

std::map<std::string, command_type> str_com_map = {
    {"-o", FILE_OUT},
    {"-c", NOT_LINK},
    {"-I", INC_DIRS},
    {"-L", LIB_DIRS},
};

task::task(int argc, const char *argv[])
{
        char ex_msg[EX_MSG_SIZE];
        std::string carg;
        std::string ccommand;

        for (int i = 1; i < argc; i++) {

            // read next command or arg
            if (argv[i][0] == '-') {
                // previos command not consumed
                if (ccommand.size() != 0)
                    throw std::invalid_argument("no arg provided");

                ccommand.insert(0, argv[i]);

            } else
                carg.insert(0, argv[i]);

            // insert new command
            if (ccommand.size() != 0) {
                command_type ccode;
                try {
                    ccode = str_com_map.at(ccommand);
                } catch (std::out_of_range &ex) {

                }

                switch (ccode) {
                    case FILE_OUT:
                    {
                        if (carg.size() != 0) {
                            fout = carg;
                            commands.insert(FILE_OUT);
                            ccommand.erase(ccommand.begin(), ccommand.end());
                            carg.erase(carg.begin(), carg.end());
                        }
                        continue;
                    }
                    case NOT_LINK:
                    {
                        commands.insert(NOT_LINK);
                        ccommand.erase(ccommand.begin(), ccommand.end());
                        break;
                    }
                    case INC_DIRS:
                    {
                        if (carg.size() != 0) {
                            inc_dirs.push_back(carg);
                            commands.insert(INC_DIRS);
                            ccommand.erase(ccommand.begin(), ccommand.end());
                            carg.erase(carg.begin(), carg.end());
                        }
                        continue;
                    }
                    case LIB_DIRS:
                    {
                        if (carg.size() != 0) {
                            lib_dirs.push_back(carg);
                            commands.insert(LIB_DIRS);
                            ccommand.erase(ccommand.begin(), ccommand.end());
                            carg.erase(carg.begin(), carg.end());
                        }
                        continue;
                    }

                    default:
                        throw std::invalid_argument("wrong command");
                }
            }

            /* prev commands do not consume carg, so
             * so we treat it as file name */
            if (carg.size() != 0) {
                files.push_back(carg);
                carg.erase(carg.begin(), carg.end());
            }

        }

        if (ccommand.size() != 0)
            throw std::invalid_argument("no arg provided");

    }