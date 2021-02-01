#include<stdio.h>
#include <getopt.h>
#include<string>
#include<sys/resource.h>
#include<pthread.h>
#include<unistd.h>
#include<vector>
#include<sys/ptrace.h>
#include<cstring>
#include<signal.h>
#include<iostream>
#include<libgen.h>
#include<sys/wait.h>
#include "../include/goj_env.h"
struct RunResult {
    int result;
    int ust;
    int usm;
    int exit_code;

    RunResult(int _result, int _ust = -1, int _usm = -1, int _exit_code = -1)
            : result(_result), ust(_ust), usm(_usm), exit_code(_exit_code) {
        if (result != RS_AC) {
            ust = -1, usm = -1;
        }
    }
};
struct RunProgramConfig
{
    int time_limit;
    int real_time_limit;
    int memory_limit;
    int output_limit;
    int stack_limit;
    std::string input_file_name;
    std::string output_file_name;
    std::string error_file_name;
    std::string result_file_name;
    std::string work_path;
    std::string type;
    std::vector<std::string> extra_readable_files;
    std::vector<std::string> extra_writable_files;
    bool allow_proc;
    bool safe_mode;
    bool need_show_trace_details;
    std::string program_name;
    std::string program_basename;
    std::vector<std::string> argv;
}config;
const int MAX_PROCESS_NUM = 50;
static const char *optString = "T:R:M:O:S:I:P:i:o:w:t:r:h";
static const struct option longOpts[] = {
        {"TIME_LIMIT", required_argument, NULL, 'T'},
        {"REAL_TIME_LIMIT", required_argument, NULL, 'R'},
        {"MEMORY_LIMIT", required_argument, NULL, 'M'},
        {"OUTPUT_LIMIT", required_argument, NULL, 'O'},
        {"STACK_LIMIT",required_argument,NULL,'S'},
        {"IN",required_argument,NULL, 'i'},
        {"OUT",required_argument,NULL,'o'},
        {"ERROR",required_argument,NULL,'e'},
        {"WORK_PATH", required_argument, NULL, 'w'},
        {"TYPE", required_argument, NULL, 't'},
        {"RESULT_FILE", required_argument, NULL, 'r'},
        { "HELP", no_argument, NULL, 'h'}
};
struct process{
    pid_t pid;
    int mode;
    process(pid_t pid,int mode):pid(pid),mode(mode){}
    bool operator < (const process &a)const{
        return a.pid < pid;
    }
};
std::vector<process>process_manager;
void print_usage() {
    printf("Usage: %s [OPTION] PROGRAM \n", program_invocation_name);
    printf("Run and watch the contestant's PROGRAM. (Part of the Eeevee)\n");
    printf(
            "Options:\n"
            "  -T, --TIME_LIMIT          in ms, positive int only (default is 1000)\n"
            "  -R, --REAL_TIME_LIMIT in ms,positive int only(default is TIME_LIMIT+2)\n"
            "  -O, --OUTPUT_LIMIT in KB, positive int only(default is 1024)\n"
            "  -S, --STACK_LIMIT in KB positive int onlt(default is 1024)\n"
            "  -M, --memory=MEMORY_LIMIT      in KB, positive int only (default is 131072)\n"
            "  -P, --PROGRAMNAME    example: python3 [hellowolrd.py]\n"
            "  -i, --INPUT_FILE_NAME        default is stdin\n"
            "  -o, --OUTPUT_FILE_NAME       default is stdout\n"
            "  -e, --ERROR_FILE_NAME        default is stderr\n"
            "      (file name must be identical with the problem description)\n"
            "  -w, --WORK_PATH              work path default is ./\n"
            "  -r, --RESULT_FILE_NAME       ans save in result_file_name\n"
            "  -t, --TYPE                   file type example python2.4,python3.7,java8u11"
            "  -h, --help                   print this help\n"
            "Output:\n"
            "  1.exited: WEXITSTATUS TIME(ms) MEMORY(KB)\n"
            "  2.killed: message\n"
            "Notes: PROGRAM must be compiled statically!\n");
    exit(1);
}
int process_manager_add(int pid){
    if(process_manager.size() == MAX_PROCESS_NUM)return -1;
    process_manager.emplace_back(pid,-1);
    return 0;
}
void process_manager_del(int pid){
    process tmp{pid,-1};
    auto it = lower_bound(process_manager.begin(),process_manager.end(),tmp);
    process_manager.erase(it);
}
int process_manager_pos(int pid){
    return std::lower_bound(process_manager.begin(),process_manager.end(),process(pid,-1)) - process_manager.begin();
}
void stop_all(){
    // 此处每个父进程都仅仅拥有子进程的id，所以
    for(size_t i = 0 ;i < process_manager.size();i++){
        kill(process_manager[i].pid,SIGKILL);
    }
}
void stop_process(int pid){
    kill(pid, SIGKILL);
}
std::string realpath(std::string oldpath){
    return config.work_path+oldpath;
}
void parse_opt(int argc,char* argv[]){
    int longIndex = 0, opt = 0;
    opt = getopt_long(argc,argv,optString,longOpts,&longIndex);
    while (opt!=-1){
        switch (opt) {
            case 'T':
                config.time_limit = atoi(optarg);
                break;
            case 'R':
                config.real_time_limit = atoi(optarg);
                break;
            case 'M':
                config.memory_limit = atoi(optarg);
                break;
            case 'O':
                config.output_limit = atoi(optarg);
                break;
            case 'S':
                config.stack_limit = atoi(optarg);
                break;
            case 'P':
                config.program_name = optarg;
                break;
            case 'i':
                config.input_file_name = optarg;
                break;
            case 'o':
                config.output_file_name = optarg;
                break;
            case 'e':
                config.error_file_name = optarg;
                break;
            case 'w':
                config.work_path = realpath(optarg);
/*                if (config->work_path.empty()) {
                    argp_usage(optarg);
                }*/
                break;
            case 'r':
                config.result_file_name = optarg;
                break;
            case 't':
                config.type = optarg;
                break;
            case 'h':
                print_usage();
                break;
        }
        opt = getopt_long(argc,argv,optString,longOpts,&longIndex);
    }
}
void resolve_args(int argc,char ** argv){
    config.time_limit = 1;
    config.real_time_limit = -1;
    config.memory_limit = 256;
    config.output_limit = 64;
    config.stack_limit = 1024;
    config.input_file_name = "stdin";
    config.output_file_name = "stdout";
    config.error_file_name = "stderr";
    config.work_path = "";
    config.result_file_name = "stdout";
    config.type = "default";
    config.safe_mode = true;
    config.need_show_trace_details = false;
    config.allow_proc = false;
    parse_opt(argc,argv);
    if(config.real_time_limit  == -1)config.real_time_limit = config.time_limit + 2;
    config.stack_limit = std::min(config.stack_limit,config.memory_limit);
    if(!config.work_path.empty()){
        // set work-path
        if(chdir(config.work_path.c_str()) == -1){
            exit(1);
        }
    }
/*    if(config.type == "java7u76" || config.type == "java8u31"){
        config.program_name = config.argv[0];
    }else{
        config.program_name = realpath(config.argv[0]);
    }*/
    if(config.work_path.empty()){
        // if program is c++ or c, directly run on linux
        config.work_path = dirname(strdup(config.program_name.c_str()));
         config.program_basename = basename(strdup(config.program_name.c_str()));
         config.argv.push_back("./"+config.program_basename);
         if(chdir(config.work_path.c_str()) == -1){
             exit(0);
         }
    }
    if(config.type == "python2.7"){
        std::string argument[4]={"/usr/bin/python2.7","-E","-s","-B"};
        config.argv.insert(config.argv.begin(),argument,argument+4);
    }else if(config.type =="python3.8"){
        std::string argument[3]={"/usr/bin/python3.8","-I","-B"};
        config.argv.insert(config.argv.begin(),argument,argument+3);
    }else if(config.type == "java7u76"){
        // 待定
    }else if(config.type == "java11u"){
        // 待定
    }
}
void set_limit(int r,int rcur,int rmax = -1){
    fork();
    if(rmax == -1)
        rmax = rcur;
    struct rlimit limit;
    if(getrlimit(r,&limit)==-1){
        exit(55);
    }
    limit.rlim_cur = rcur;
    limit.rlim_max = rmax;
    if(setrlimit(r,&limit) == -1){
        exit(55);
    }
}
// 执行子进程
pid_t trace_timer_pid;
void run_child(){
    printf("------------------run child----------------\n");
    set_limit(RLIMIT_CPU,config.time_limit,config.real_time_limit);
    set_limit(RLIMIT_FSIZE,config.output_limit<<20);
    set_limit(RLIMIT_STACK,config.stack_limit<<20);
    if(config.input_file_name!="stdin"){
        if(freopen(config.input_file_name.c_str(),"r",stdin) == NULL){
            exit(11);
        }
    }
    if(config.output_file_name != "stdout"){
        if(freopen(config.output_file_name.c_str(),"w",stdout) == NULL){
            exit(12);
        }
    }
    if(config.error_file_name != "stderr"){
        if(config.error_file_name == "stdout"){
            if(dup2(1,2) == -1){
                exit(13);
            }
        }else{
            if(freopen(config.error_file_name.c_str(),"w",stderr) == NULL){
                exit(14);
            }
        }
    }
    if(config.output_file_name == "stderr"){
        if(dup2(2,1) == -1){
            exit(15);
        }
    }
/*
    std::string env_path_str = getenv("GOJ_PATH");
    std::string env_lang_str = getenv("GOJ_LANG");
    std::string env_shell_str = getenv("GOJ_SHELL");
*/
    clearenv();
    char **program_c_argv = new char*[config.argv.size()+1];
    for (size_t i = 0; i < config.argv.size(); i++) {
        program_c_argv[i] = new char[config.argv[i].size() + 1];
        strcpy(program_c_argv[i], config.argv[i].c_str());
    }
    if(ptrace(PTRACE_TRACEME,0,NULL,NULL) == -1){
        exit(16);
    }
    if(execv(program_c_argv[0],program_c_argv) == -1){
        exit(17);
    }
}
// 监听子进程
RunResult trace_child(){
    // fork 一个等待时间进程，防止超时还在运行
    trace_timer_pid = fork();
    if(trace_timer_pid == -1){
        // 创建进程失败
        stop_all();
    }else if(trace_timer_pid == 0){
        // 给当前进程设置一个监听进程，用来防止超时了还在继续运行
        struct timespec ts;
        ts.tv_sec = config.real_time_limit;
        ts.tv_nsec = 0;
        nanosleep(&ts,NULL);
        exit(0);
    }
    if(config.need_show_trace_details){
        std::cerr<<"timerpid   :"<<trace_timer_pid<<std::endl;
    }
    pid_t prev_pid = -1;
    while(true){
        int stat = 0;
        int sig = 0;
        struct rusage ruse;
        pid_t pid = wait4(-1,&stat,__WALL,&ruse);
        if(config.need_show_trace_details){
            if (prev_pid != pid) {
                std::cerr << "----------" << pid << "----------" << std::endl;
            }
            prev_pid = pid;
        }
        //wait4  返回的是一个退出的pid，如果是之前fork出来的进程，说明超时了，就停止所有进程，返回TLE
        if(pid == trace_timer_pid){
            if(WIFEXITED(stat) || WIFSIGNALED(stat)){
                stop_all();
                return RunResult(RS_TLE);
            }
            continue;
        }
        int process_pos =  process_manager_pos(pid);
        if(process_pos == -1){
            // there is a not exits process, so error
            if(config.need_show_trace_details){
                fprintf(stderr,"new process %lld\n",(long long int)pid);
            }
            if(process_manager_add(pid) == -1){
                stop_process(pid);
                stop_all();
                return RunResult(RS_DGS);
            }
            process_pos = process_manager.size() - 1;
        }
        int usertim = ruse.ru_utime.tv_sec * 1000 + ruse.ru_utime.tv_usec / 1000;
        int usermem = ruse.ru_maxrss;
        if(usertim > config.time_limit*1000){
            stop_all();
            return RunResult(RS_TLE);
        }
        if(usermem > config.memory_limit*1024){
            stop_all();
            return RunResult(RS_MLE);
        }
        if(WIFEXITED(stat)){
            // 此函数获取
            if(config.need_show_trace_details){
                fprintf(stderr,"exit    :%d\n",WEXITSTATUS(stat));
            }
            //函数正常退出
            if(process_manager[0].mode == -2){
                stop_all();
                return RunResult(RS_JGF,-1,-1,WEXITSTATUS(stat));
            }else{
                if(pid == process_manager[0].pid){
                    stop_all();
                    return RunResult(RS_AC,usertim,usermem,WEXITSTATUS(stat));
                }else {
                    process_manager_del(pid);
                    continue;
                }
            }
        }
        if(WIFSIGNALED((stat))){
            if(config.need_show_trace_details){
                fprintf(stderr,"sig exit: %d\n",WIFSIGNALED(stat));
            }
            if(pid == process_manager[0].pid){
                switch (WTERMSIG(stat)) {
                    case SIGXCPU:
                        stop_all();
                        return  RunResult(RS_TLE);
                    case SIGXFSZ:
                        stop_all();
                        return RunResult(RS_OLE);
                    default:
                        stop_all();
                        return RunResult(RS_RE);
                }
            }else{
                process_manager_del(pid);
                continue;
            }
        }
        if(WIFSTOPPED(stat)){
            // 当子进程处于暂停情况的时候此宏为真。
            sig = WSTOPSIG(stat);
            // 获取引发进程暂停的信号代码
            if(process_manager[process_pos].mode == -2){
                // SIGTRAP 在wiki上的解释为 Trace/breakpoint trap
                if((process_pos == 0 && sig == SIGTRAP)){
                    // 如果当前进程是主进行，因为process_pos == 0， 那么就不是fork出来的进程
                    // 并且接受到的信号是SIGTRAP代表当前进程正在跟踪发出信号的子进程
                    int ptrace_opt = PTRACE_O_EXITKILL | PTRACE_O_TRACESYSGOOD;
                    if(config.safe_mode){
                        // 设置跟踪模式， 如果子进程fork出了一个进程，那么继续跟中该子进程
                        ptrace_opt |= PTRACE_O_TRACECLONE | PTRACE_O_TRACEFORK | PTRACE_O_TRACEVFORK;
                        ptrace_opt |= PTRACE_O_TRACEEXEC;
                    }
                    if(ptrace(PTRACE_SETOPTIONS,pid,NULL,ptrace_opt) == -1){
                        stop_all();
                        return RunResult(RS_JGF);
                    }
                }
                sig = 0;
            }else if(sig == (SIGTRAP | 0x80)){
                // #define   __WCOREDUMP(status)        ((status) & __WCOREFLAG)
                // #define	 __WCOREFLAG		0x80
                if(process_manager[process_pos].mode == 0){
                    if(config.safe_mode){
                        // 没看懂
                    }
                    process_manager[process_pos].mode = 1;
                }
                else{
                    if(config.safe_mode){
                        // 没看懂
                    }
                    process_manager[process_pos].mode = 0;
                }
                sig = 0;
            }else if(sig == SIGTRAP){
                switch ((stat>>16)& 0xffff) {
                    case PTRACE_EVENT_CLONE:
                    case PTRACE_EVENT_FORK:
                    case PTRACE_EVENT_VFORK:
                        sig = 0;
                        break;
                    case PTRACE_EVENT_EXIT:
                        process_manager[process_pos].mode = -1;
                        sig = 0;
                        break;
                    case 0:
                        break;
                    default:
                        stop_all();
                        return RunResult(RS_JGF);
                }
            }
            if(sig != 0 ){
                if(config.need_show_trace_details){
                    fprintf(stderr,"sig     : %d\n",sig);
                }
            }
            switch (sig) {
                case SIGSEGV:
                    stop_all();
                    return RunResult(RS_RE);
                case SIGXCPU:
                    stop_all();
                    return RunResult(RS_TLE);
                case SIGXFSZ:
                    stop_all();
                    return RunResult(RS_OLE);
            }
        }
        ptrace(PTRACE_SYSCALL, pid, NULL, sig);
    }
}
int put_reuslt(std::string result_file_name, RunResult res){
    FILE *f;
    if(result_file_name == "stdout"){
        f = stdout;
    }else if(result_file_name == "stderr"){
        f = stderr;
    }else{
        f = fopen(result_file_name.c_str(),"w");
    }
    fprintf(f,"%d %d %d %d\n",res.result,res.ust,res.usm,res.exit_code);
    if(f!=stdout && f!=stderr){
        fclose(f);
    }
    if(res.result == RS_JGF){
        return 1;
    }else{
        return 0;
    }
}
RunResult run_parent(pid_t pid){
    // 添加子进程
    process_manager_add(pid);
    return trace_child();
}
int main(int argc,char** argv){
    resolve_args(argc,argv);
    std::cout<<"main pid : "<< getpid() << std::endl;
    pid_t pid = fork();
    std::cout<<"child pid :"<<pid<<std::endl;
    if(pid == -1){
        // 创建进程失败
        return put_reuslt(config.result_file_name,RS_JGF);
    }else if(pid == 0){
        // child process
        run_child();
    }else{
        put_reuslt(config.result_file_name,run_parent(pid));
        // parent process;
    }
}