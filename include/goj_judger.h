//
// Created by geray on 2021/1/8.
//

#ifndef CJUDGER_GOJ_JUDGER_H
#define CJUDGER_GOJ_JUDGER_H

#endif //CJUDGER_GOJ_JUDGER_H
#include "goj_env.h"
#include<cstring>
#include <string>
#include <math.h>
// esult for judger
struct RunResult{
    int type;
    int ust,usm;
    int exit_code;
    // If system error return error
    static RunResult failed_result(){
        RunResult res;
        res.type = RS_JGF;
        res.ust = -1;
        res.usm = -1;
        return res;
    }
    static RunResult from_file(const std::string &file_name) {
        RunResult res;
        FILE *fres = fopen(file_name.c_str(), "r");
        if (fres == NULL || fscanf(fres, "%d %d %d %d", &res.type, &res.ust, &res.usm, &res.exit_code) != 4) {
            return RunResult::failed_result();
        }
        fclose(fres);
        return res;
    }
};
struct RunLimit {
    int time;
    int real_time;
    int memory;
    int output;

    RunLimit() {
    }
    RunLimit(const int &_time, const int &_memory, const int &_output)
            : time(_time), memory(_memory), output(_output), real_time(-1) {
    }
};
const RunLimit RL_DEFAULT = RunLimit(1, 256, 64);
const RunLimit RL_JUDGER_DEFAULT = RunLimit(600, 1024, 128);
const RunLimit RL_CHECKER_DEFAULT = RunLimit(5, 256, 64);
const RunLimit RL_INTERACTOR_DEFAULT = RunLimit(1, 256, 64);
const RunLimit RL_VALIDATOR_DEFAULT = RunLimit(5, 256, 64);
const RunLimit RL_MARKER_DEFAULT = RunLimit(5, 256, 64);
const RunLimit RL_COMPILER_DEFAULT = RunLimit(15, 512, 64);
struct CustomTestInfo  {
    int ust, usm;
    std::string info, exp, out;

    CustomTestInfo(const int &_ust, const int &_usm, const std::string &_info,
                   const std::string &_exp, const std::string &_out)
            : ust(_ust), usm(_usm), info(_info),
              exp(_exp), out(_out) {
    }
};
std::string file_preview(const std::string &name, const size_t &len = 100) {
    FILE *f = fopen(name.c_str(), "r");
    if (f == NULL) {
        return "";
    }
    std::string res = "";
    int c;
    while (c = fgetc(f), c != EOF && res.size() < len + 4) {
        res += c;
    }
    if (res.size() > len + 3) {
        res.resize(len);
        res += "...";
    }
    fclose(f);
    return res;
}
struct RunCheckerResult {
    int type;
    int ust, usm;
    int scr;
    std::string info;

    static RunCheckerResult from_file(const std::string &file_name, const RunResult &rres) {
        RunCheckerResult res;
        res.type = rres.type;
        res.ust = rres.ust;
        res.usm = rres.usm;

        if (rres.type != RS_AC) {
            res.scr = 0;
        } else {
            FILE *fres = fopen(file_name.c_str(), "r");
            char type[21];
            if (fres == NULL || fscanf(fres, "%20s", type) != 1) {
                return RunCheckerResult::failed_result();
            }
            if (strcmp(type, "ok") == 0) {
                res.scr = 100;
            } else if (strcmp(type, "points") == 0) {
                double d;
                if (fscanf(fres, "%lf", &d) != 1) {
                    return RunCheckerResult::failed_result();
                } else {
                    res.scr = (int)floor(100 * d + 0.5);
                }
            } else {
                res.scr = 0;
            }
            fclose(fres);
        }
        res.info = file_preview(file_name);
        return res;
    }

    static RunCheckerResult failed_result() {
        RunCheckerResult res;
        res.type = RS_JGF;
        res.ust = -1;
        res.usm = -1;
        res.scr = 0;
        res.info = "Checker Judgment Failed";
        return res;
    }
};
// 编译结果
struct RunCompilerResult{
    int type;
    int ust,usm;
    bool succeeded;
    std::string info;
    static RunCompilerResult failed_result(){
        RunCompilerResult res;
        res.type = RS_JGF;
        res.ust = -1;
        res.usm = -1;
        res.succeeded = false;
        res.info  = "Compile Failed";
        return res;
    }
};
int problen_id;
std::string main_path,work_path,data_path,result_path;
int tot_time = 0, max_memory = 0, tot_score = 0;
inline std::string info_str(int id)  {
    switch (id) {
        case RS_MLE: return "Memory Limit Exceeded";
        case RS_TLE: return "Time Limit Exceeded";
        case RS_OLE: return "Output Limit Exceeded";
        case RS_RE : return "Runtime Error";
        case RS_DGS: return "Dangerous Syscalls";
        case RS_JGF: return "Judgment Failed";
        default    : return "Unknown Result";
    }
}