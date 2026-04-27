#ifndef TOOLS_H
#define TOOLS_H
#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include "../../dev/x_moildev.h"
#include "./tools.h"
using namespace cv;
using namespace  std ;

namespace utl {
    bool exists_test3 (const std::string& name) ;
    bool doesFileExist(const std::string& name) ;
    char getch_of_no_excuse(void);
    string now_as_string() ;
    const char *get_home_dir();
    namespace term{
        void cin2empty();
        void cin2buf_nshq_(char path[], int len);
    }
}


typedef struct
{
    enum class MediaType
    {
        NONE,
        IMAGE_FILE,
        CAMERA,
        VIDEO
    };

    enum class SystemState
    {
        NONE, //Not using case in this module
        NPPM,
        CAR_VIEW,
        EXIT,
        ERROR,
        PAUSE
    };
    //SystemState state = SystemState::NONE;

    enum class RecordOnOff
    {
        OFF,
        ON
    };
} ING_STAT; //MODE is never used




typedef struct running_mode
{
    ING_STAT::MediaType util_media_type;     //init(IMAGE_FILE)
    ING_STAT::SystemState util_system_state;    //init(NONE)
    ING_STAT::RecordOnOff recordState; //init(OFF)
    //ING_STAT::CameraPos cameraPos;     //init(OFF)
} running_mode_t;

string str_token(std::string &string_token);
double align_token_str_dbl(std::string &target, double &value) ;
double align_token_str_dbl(std::string &namedValue, double &value, bool debug) ;
bool align_token_str_bool(string &target, bool &true_or_false) ;
bool align_token_str_bool(string &target, bool &true_or_false, bool debug) ;
int align_token_str_int_(std::string &target, int &a_int) ;
int align_token_str_int(std::string &namedValue, int &a_int, bool debug) ;
string align_token_str_str_and_skip_comment(const std::string &target, std::string &a_string) ;
string align_token_str_str(const std::string &namedValue, std::string &a_string, bool debug) ;
//const std::string la_video_stream_uri = "http://192.168.100.199:8000/stream.mjpg";

void align_token_dbls(double values[], bool debug) ;
void kbhit() ;
int  kbhit_2(void) ;

 void caster_dump();

/*
 int logError (const char* file, const int line_nr, const std::string& message)
{
    cerr << "[" << file << "],(line " <<line_nr <<") "<< endl
    <<"MSG:"<< message <<endl<<endl;
    return 0 ;
}
 * */



int logError (const char* file, const int line_nr, const std::string& message) ;
#define LOG( message ) logError( __FILE__, __LINE__, message )

template<typename T, typename I>
void extLog(char* filename, int line_nr, I hint, const string &var_token, T var) {
    //string part_1(__FILE__);
    //string part_1(static_cast<string.cstr()> (hint));
    stringstream ss ;
    ss<< "[" << filename << "],(line " <<line_nr <<") "<< endl << " [0] Brief MSG: " << hint <<endl
    <<" [1] Named:" <<
    //" {("  << var_token << ", " << var <<")}" << endl
    " {"  << var_token <<"}" << endl
    << " [2] Value: {" << var << "} ";
    string part_1 = ss.str() ;
    //logError( filename, line_nr, part_1 ) ;
    cerr <<  part_1 <<endl << endl;
    ss.str("");
    ss.clear();
}
#define MY_MSG_ON

#ifdef MY_MSG_ON
#define tLog(p1, p2, p3) \
{                        \
char pr[1024];           \
strcpy(pr, __FILE__);    \
extLog(pr, __LINE__, p1, p2, p3 ); \
}

#define tExit(p1, p2, p3) \
{                          \
    tLog(p1,p2,p3); exit(0) ;\
}

#define tLog_1(p1) tLog(p1, p1, p1)
#define tLog_2(p1, p2) tLog(p1, p2, p2)
#else
#define tLog(p1, p2, p3) \
{}
#endif

#endif