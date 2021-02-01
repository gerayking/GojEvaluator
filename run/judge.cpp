#include<getopt.h>
#include<stdio.h>
#include<cstring>
#include<string>
#define LINE_BUF_MAX_SIZE  200
#define STR_BUF_MAX_SIZE 2000000
static const char *optString = "m:i:a:r:h";
static const struct option longOpts[] = {
        {"MODE", required_argument, NULL, 'm'},
        {"INPUT_FILE_NAME", required_argument, NULL, 'i'},
        {"ANSWER_FILE_NAME", required_argument, NULL, 'a'},
        {"RESULT_FILE_NAME", required_argument, NULL, 'r'},
        {"COMMAND HELP", required_argument, NULL, 'h'},
};
enum MODE{
    ACCURATE = 1,
    ROUGH
};
// 定义输入环境
struct judge_config{
    int mode = MODE::ROUGH;
    std::string input_file_name;
    std::string answer_file_name;
    // result_file_name 考虑后期加入错误提示
    std::string result_file_name;
    int status;
}judge_cfg;
void print_usage() {
    printf("Usage: %s [OPTION] PROGRAM \n", program_invocation_name);
    printf("Run and watch the contestant's PROGRAM. (Part of the Eeevee)\n");
    printf(
            "Options:\n"
            "  -m, --COMPARE MODE\n"
            "  1. accurate compare\n"
            "  2. rough compare\n"
            "  -i, input file name\n"
            "  -a  answer file name\n"
            "  -r, result file name\n"
            "  -h, --COMMAND HELP\n"
            "Output:\n"
            "  1.STATUS\n"
            "Notes : NO!\n");
    exit(1);
}
// 精确对比，不忽略空格
bool accuratejudge(char *buf1, char *buf2){
    int i;
    for(i = LINE_BUF_MAX_SIZE-1; buf1[i] != '\n'; --i);
    if(i > 0)
        buf1[i] = '\000';
    for(i = LINE_BUF_MAX_SIZE-1; buf2[i] != '\n'; --i);
    if(i > 0)
        buf2[i] = '\000';
    for(i=0; i<LINE_BUF_MAX_SIZE; ++i){
        if(buf1[i] != buf2[i])
            return 1;
    }
    return 0;
}
char stmp1[STR_BUF_MAX_SIZE];
char stmp2[STR_BUF_MAX_SIZE];
// 粗糙对比， 忽略空格
bool roughjudge(FILE *buf1, FILE *buf2){
    int fd1,fd2;
    while(1){
        fd2 = fscanf(buf2,"%s",stmp2);
        fd1 = fscanf(buf1,"%s",stmp1);
        if(fd1 == EOF || fd2 == EOF)break;
        if(strcmp(stmp1,stmp2)){
            return 1;
        }
    }
    fd2 = fscanf(buf2,"%s",stmp2);
    if(fd1 != EOF || fd2 !=EOF )return 1;
    return 0;
}
// spj 以后写
bool specialjudge();// 特殊判断
// 对比函数
bool judge(){
    FILE* fd_input = fopen(judge_cfg.input_file_name.c_str(),"r");
    FILE* fd_answer = fopen(judge_cfg.answer_file_name.c_str(),"r");
    if(fd_input == NULL || fd_answer == NULL){
        fprintf(stderr,"ERROR fd_input or fd_answer is null\n");
        exit(0);
    }
    if(judge_cfg.mode == MODE::ACCURATE) {
        char input_buf[LINE_BUF_MAX_SIZE] = {0}, answer_buf[LINE_BUF_MAX_SIZE] = {0};
        while (!feof(fd_input) && !feof(fd_answer)) {
            memset(input_buf, 0, LINE_BUF_MAX_SIZE);
            memset(answer_buf, 0, LINE_BUF_MAX_SIZE);
            fgets(input_buf, LINE_BUF_MAX_SIZE, fd_input);
            fgets(answer_buf, LINE_BUF_MAX_SIZE, fd_answer);
            if (accuratejudge(input_buf, answer_buf)) {
                fclose(fd_input);
                fclose(fd_answer);
                return false;
            }
        }
        if (!feof(fd_input) || !feof(fd_answer)) {
            fclose(fd_input);
            fclose(fd_answer);
            return false;
        }
        fclose(fd_input);
        fclose(fd_answer);
        return true;
    }
    else if(judge_cfg.mode == MODE::ROUGH){
        if(roughjudge(fd_input,fd_answer)){
            fclose(fd_input);
            fclose(fd_answer);
            return false;
        }
        return true;
    }
    fclose(fd_input);
    fclose(fd_answer);
    return true;
}
int main(int argc,char **argv){
    int longIndex = 0, opt = 0;
    opt = getopt_long(argc,argv,optString,longOpts,&longIndex);
    while (opt != -1){
        switch (opt) {
            case 'm':
                judge_cfg.mode = atoi(optarg);
                break;
            case 'i':
                judge_cfg.input_file_name = optarg;
                break;
            case 'a':
                judge_cfg.answer_file_name = optarg;
                break;
            case 'r':
                judge_cfg.result_file_name = optarg;
                break;
            case 'h':
                print_usage();
                break;
        }
        opt = getopt_long(argc,argv,optString,longOpts,&longIndex);
    }
    return judge();
}