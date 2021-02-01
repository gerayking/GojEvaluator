#include<cstring>
#include<iostream>
#include <getopt.h>
#include<unistd.h>
#include<vector>
static const char *optString = "l:w:o:i:e:h:r";
static const struct option longOpts[] = {
        {"LANGUAGE", required_argument, NULL, 'l'},
        {"WORKPATH", required_argument, NULL, 'w'},
        {"OUTPUT_FILE_NAME", required_argument, NULL, 'o'},
        {"INPUT_FILE_NAME", required_argument, NULL, 'i'},
        {"ERROR_FILE_NAME", required_argument, NULL, 'e'},
        {"RESULT_FILE_NAME", required_argument, NULL, 'r'},
        {"LANGUAGE", no_argument, NULL, 'h'},
};
void print_usage() {
    printf("Usage: %s [OPTION] PROGRAM \n", program_invocation_name);
    printf("Run and watch the contestant's PROGRAM. (Part of the Eeevee)\n");
    printf(
            "Options:\n"
            "  -l, --LANGUAGE          COMPILE LANGUAGE\n"
            "  -w, --WORK_PATH         relative path\n"
            "  -i, --FILENAME OF NEED COMPILE\n"
            "  -e  --ERROR FILE\n"
            "  -o, --COMPILE OUTPUT FILE's PATH\n"
            "  -h, --COMMAND HELP\n"
            "Output:\n"
            "  1.STATUS\n"
            "  2.COMPILE INFO\n"
            "Notes : NO!\n");
    exit(1);
}
struct compile_config{
    std::string language;
    std::string work_math;
    std::string error_file_name;
    std::string result_file_name;
    std::string out_file_name;
    std::string input_file_name;
    std::string args;
}com_cfg;
struct compile_result{
    int status;
    std::string info;
    compile_result(int status):status(status){}
    compile_result(int status,std::string info):status(status),info(info){}
};
void a2A(std::string &s){
    for(auto ch = s.begin();ch!=s.end();ch++){
        if((*ch) >= 'a'&&(*ch)<='z'){
            (*ch)-=32;
        }
    }
}
int env_pre(){
    FILE *f;
    if(com_cfg.result_file_name == "stdout"){
        f = stdout;
    }else if(com_cfg.result_file_name == "stderr"){
        f = stderr;
    }else{
        f = fopen(com_cfg.result_file_name.c_str(),"w");
    }
    if(com_cfg.input_file_name == ""){
        fprintf(f,"ERROR : input_file_name is empty\n");
        exit(-1);
    }
    if(com_cfg.out_file_name == ""){
        fprintf(f,"ERROR : output_file_name is empty");
        exit(-1);
    }

    if(chdir(com_cfg.work_math.c_str()) == -1){
        fprintf(f, "ERROR:failture in change the work path to %s ",com_cfg.work_math.c_str());
        exit(-1);
    }
    a2A(com_cfg.language);
    if(com_cfg.language == "C++11"){
        com_cfg.args = "g++ -O2 -w -static -fmax-errors=3 -std=++c11 "
                       + com_cfg.input_file_name
                       + " -o "
                       + com_cfg.out_file_name
                       + " > " + com_cfg.error_file_name;
    }
    else if(com_cfg.language == "C++14"){
        com_cfg.args = "g++ -O2 -w -static -fmax-errors=3 -std=c++14 "
                                + com_cfg.input_file_name
                                + " -o "
                                + com_cfg.out_file_name
                                + " > " + com_cfg.error_file_name;
    }else if(com_cfg.language=="C++17"){
        com_cfg.args = "g++ -O2 -w -static -fmax-errors=3 -std=c++17 "
                               + com_cfg.input_file_name
                               + " -o "
                               + com_cfg.out_file_name
                               + " > " + com_cfg.error_file_name;    }
    else if(com_cfg.language == "C"){
        com_cfg.args = "gcc -O2 -w -static -fmax-errors=3 "
                               + com_cfg.input_file_name
                               + " -o "
                               + com_cfg.out_file_name
                               + " > " + com_cfg.error_file_name;
    }
    else if(com_cfg.language == "JAVA"){}
    else if(com_cfg.language == "PYTHON"){}else{
        fprintf(f,"NOT SUPPROT LANGUAGE %s",com_cfg.language.c_str());
        exit(0);
    }
}
void parse_opt(int argc, char **argv){
    com_cfg.language  = "C++";
    com_cfg.work_math = "/";
    com_cfg.out_file_name = "";
    com_cfg.input_file_name ="";
    com_cfg.result_file_name = "stdout";
    int longIndex = 0, opt = 0;
    opt = getopt_long(argc,argv,optString,longOpts,&longIndex);
    while (opt != -1){
        switch (opt) {
            case 'l':
                com_cfg.language = optarg;
                break;
            case 'w':
                com_cfg.work_math = optarg;
                break;
            case 'i':
                com_cfg.input_file_name = optarg;
                break;
            case 'o':
                com_cfg.out_file_name = optarg;
                break;
            case 'e':
                com_cfg.error_file_name = optarg;
                break;
            case 'r':
                com_cfg.result_file_name = optarg;
                break;
            case 'h':
                print_usage();
                break;
            default:
                continue;

        }
        opt = getopt_long(argc,argv,optString,longOpts,&longIndex);
    }

}
void compile(){
    // maybe can use popen() to compile
    freopen(com_cfg.error_file_name.c_str(),"w",stderr);
    int stat = system(com_cfg.args.c_str());
    FILE *f;
    if(com_cfg.result_file_name == "stdout"){
        f = stdout;
    }else if(com_cfg.result_file_name == "stderr"){
        f = stderr;
    }else{
        f = fopen(com_cfg.result_file_name.c_str(),"w");
    }
    switch (stat) {
        case 127:
            fprintf(f," [%s] COMMAND NOT EXIST\n",com_cfg.args.c_str());
            exit(-1);
        case -1:
            fprintf(f,"failture in system() fork\n");
            exit(-1);
        case 0:
            fprintf(f,"OK");
            exit(0);
            break;
        default:
            fprintf(f,"COMMAND IS NULL\n");
            exit(-1);
    }
}
int main(int argc,char **argv){
    parse_opt(argc,argv);
    env_pre();
    compile();
}