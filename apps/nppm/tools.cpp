
#include "tools.h"
#include <map>
#include <termios.h>
running_mode_t el_state ;
//Get Home Folder
#ifndef GET_HOME_DIR
#define GET_HOME_DIR
//////////////////////////////////////////////////////////////////////////////////////

//#include "fmt/format.h" //這東西如果你不喜歡可以換成iostream
//
#include <iostream>

//using namespace fmt;
using namespace std;

struct CastValue {
    string type;
    string value;
};

map<string, CastValue> caster;

string trim_copy(const string &value) {
    const auto begin = value.find_first_not_of(" \t\r\n");
    if (begin == string::npos) {
        return "";
    }
    const auto end = value.find_last_not_of(" \t\r\n");
    return value.substr(begin, end - begin + 1);
}
//////////////////////////////////////////////////////////////////////////////////////////////
// LOG can now be used to produce a log message that includes the file in which the log occurrred
int logError (const char* file, const int line_nr, const std::string& message)
{
    cerr << "[" << file << "],(line " <<line_nr <<") "<< endl
    <<"MSG:"<< message <<endl<<endl; //;
    return 0 ;
}

//#define LOG( message ) logError( __FILE__, __LINE__, message )
//////////////////////////////////////////////////////////////////////
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

struct passwd *pw = getpwuid(getuid());
const char *homedir = pw->pw_dir;

/////////////////////////////////////////////////////////////
//https://groups.google.com/g/comp.os.linux.development/c/09C0_1pA3sc
#include <termios.h>
#include <unistd.h>

struct termios argin, argout;
unsigned char ch = 0;

void kbhit()
{
    tcgetattr(0,&argin);
    argout = argin;
    argout.c_lflag &= ~(ICANON);
    argout.c_iflag &= ~(ICRNL);
    argout.c_oflag &= ~(OPOST);
    argout.c_cc[VMIN] = 1;
    argout.c_cc[VTIME] = 0;
    tcsetattr(0,TCSADRAIN,&argout);
//    if (~(ICRNL)) {
//        read(0, &ch, 1);
//        cout << ch << endl ;
//    }

    tcsetattr(0,TCSADRAIN,&argin);
}

int kbhit_2(void)
{
    struct timeval tv; fd_set
            read_fd; /* Do not wait at all, not even a microsecond */
    tv.tv_sec=0;
    tv.tv_usec=0; /* Must be done first to initialize read_fd */
    FD_ZERO(&read_fd); /* Makes select() ask if input is ready: *
0 is the file descriptor for stdin */
    FD_SET(0,&read_fd); /* The first parameter is the number of the *
largest file descriptor to check + 1. */
    if(select(1, &read_fd, NULL, /*No writes*/ NULL, /*No exceptions*/
              &tv) == -1)
        return 0; /* An error occured */

/* read_fd now holds a bit map of files that are *
readable. We test the entry for the standard *
input (file 0). */
    if(FD_ISSET(0,&read_fd)) /* Character pending on stdin */
        return 1; /* no characters were pending */
    return 0;
}

//////////////////////////////////////////////////////////////

string utl::now_as_string() {
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[80];
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, sizeof(buffer), "%d-%m-%Y[%H-%M-%S]", timeinfo);
    string str(buffer);
    cout << str;
    return str;
}
///////////////////////////////////////////////////////////////////////
const char *utl::get_home_dir()
{
   const char *aPt = homedir;
   return aPt;
}

const char *get_home_dir()
{
    const char *aPt = homedir;
    return aPt;
}


#include <iostream>
using namespace std;

extern VideoCapture cap0;

/*
int not_main() {
   Mat image = imread("bright.jpg");//loading image in the matrix//
   namedWindow("Track");//declaring window to show image//
   setMouseCallback("Track", locator, NULL);//Mouse callback function on define window//
   imshow("Track", image);//showing image on the window//
   waitKey(0);//wait for keystroke//
   return 0;
}
*/

//char utl::getch_of_no_excuse(void)
//{
//   int ch;
//   struct termios oldt, newt;
//
//   tcgetattr(STDIN_FILENO, &oldt);
//   memcpy(&newt, &oldt, sizeof(newt));
//   newt.c_lflag &= ~(ECHO | ICANON | ECHOE | ECHOK |
//                     ECHONL | ECHOPRT | ECHOKE | ICRNL);
//   tcsetattr(STDIN_FILENO, TCSANOW, &newt);
//   ch = getchar();
//   tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
//
//   return ch;
//}

//#include <iostream>
#undef _WIN32
#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

namespace utl {

    char getch_of_no_excuse(void) {
#ifdef _WIN32
        // In Windows, you can use _getch from conio.h
    return _getch();
#else
        // In UNIX-like systems, use termios to disable echo
        int ch;
        struct termios oldt, newt;

        tcgetattr(STDIN_FILENO, &oldt);
        memcpy(&newt, &oldt, sizeof(newt));
        newt.c_lflag &= ~(ECHO | ICANON | ECHOE | ECHOK | ECHONL | ECHOPRT | ECHOKE | ICRNL);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        ch = getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

        return ch;
#endif
    }

} // namespace utl



void utl::term::cin2empty()
{
   if (cin.fail())
   {
      cin.clear();
      cin.ignore(numeric_limits<streamsize>::max(), '\n');
   }
   //cout << "return from empty2cin()" << endl ;
}

/// quick skip the comment line(s) and return the line after
/// \param el
void get_string_q(string &el)
{
    char aB[1024] ;
    utl::term::cin2buf_nshq_(aB, 1024) ;
    el = trim_copy(aB);
}

//only support void get_string_q(string &el)
/// this function is play as a macro extension of get_string_q(...)
/// Skip lines begin with these letters, and return if not
///           path[0] == '#' ||
///           path[0] == '\0' ||
///           path[0] == '*' ||
///           path[0] == '{' ||
///           path[0] == '}' ||
///           path[0] == '$' ||
//           path[0] == ' ' )
/// \param path
/// \param len

void utl::term::cin2buf_nshq_(char path[], int len)
{
   if(len<1023) {
       cerr << "Buffer length: {"<< len <<"}, 256 required" << endl ;
       exit(0) ;
   }
   int acnt = 0;
   do
   {
      const int TWENTY_ONE = 121;
      static char aDebug[TWENTY_ONE][256];
      utl::term::cin2empty();
      cin.getline(path, len);
      /*
      string aS ;
      cin >> aS ; //only first word been read in
      strcpy(path, aS.c_str()) ;
      */
      if(*path == EOF){
          cerr << "An error occurs, or end-of-file is reached." ;
      }

      strcpy(aDebug[acnt], path);
      utl::term::cin2empty();

      acnt++;
      if (acnt >= TWENTY_ONE)
      {
         cout << "cin2buf_nshq():: Comment lines too many!!" << endl;
         cout << "Match key is: {"<< path << "}, Does c++ name it?" << endl;
         for (int i = 0; i < TWENTY_ONE; i++)
            cout << "[" << i + 1 << "]: " << aDebug[i] << endl;
         cout << endl
              << "Good Manner Aborted!!" << endl
              << endl;
         exit(0); //since the control script is not good
      }
   } while (
           path[0] == '#' ||
           path[0] == '\0' ||
           path[0] == '*' ||
           path[0] == '{' ||
           path[0] == '}' ||
           path[0] == '$' ||
           path[0] == ' ' );
}

const int SIX = 6;
struct
{
   string aDebug[SIX];
} me;
///////////////////////////////////////////////////////////////////////
//function read in a line and to compare with the parameter
string str_token(string &string_token) //
{
    char read_in_2[1024];
    //strcpy(read_in_2, string_token.c_str()) ;
    get_string_q(string_token) ;
    return string_token ;
}

double align_token_str_dbl_(string &target, double &value)
{
    get_string_q(target);
    cin >> value;
    return value;
}
double align_token_str_dbl(string &namedValue, double &value, bool debug)
{
   {
      align_token_str_dbl_(namedValue, value);
      cout << "#?? <double> named variable (double): {" << namedValue << " <- " << value << "}" << endl;
      cout << namedValue << endl << value << endl << "#!!!!" << endl;

      caster[namedValue] = {"double", to_string(value)};
   }
   return value;
}


/*
https://www.geeksforgeeks.org/how-to-split-a-string-in-cc-python-and-java/
 */
void simple_tokenizer(string double_values_string, double vle[])
{
    stringstream ss(double_values_string);
    double word;
    int aI = 0 ;
    while (ss >> word) {
        cout << word << " ";
        vle[aI++] = word;
    }
    cout << endl ;
}

void align_token_dbls(double values[], bool debug){
    string string_token;
    get_string_q(string_token) ;  //strtok in C
    simple_tokenizer(string_token, values) ;
}

/// read token of "true"||"false" from the running cast file
/// \param target
/// \param true_or_false
/// \return
bool align_token_str_bool_(string &target, bool &true_or_false)
{
    get_string_q(target);
    cin >> true_or_false;

    return true_or_false;
}

bool align_token_str_bool(string &target, bool &true_or_false, bool debug)
{
    align_token_str_bool_(target, true_or_false);
    if(debug) {
        cout << "#?? <bool> named variable (   bool): {" << target << " <- " << true_or_false << "}" << endl;
        cout << target << endl << true_or_false << endl;
        caster[target] = {"bool", true_or_false ? "true" : "false"};
    }
   return true_or_false;
}


int align_token_str_int_(string &target, int &a_int)
{
    get_string_q(target);
    cin >> a_int;
    return a_int;
}

int align_token_str_int(string &namedValue, int &a_int, bool debug)
{
   {
       align_token_str_int_(namedValue, a_int);
      if(debug) {
          cout << "#?? <int> named variable (   int): {" << namedValue << " <- " << a_int << "}" << endl;
          caster[namedValue] = {"int", to_string(a_int)};
      }
   }
   return a_int;
}

string align_token_str_str_and_skip_comment(const std::string &target, std::string &a_string)
{
   //char aBuf[1024];
   string a_target ;
    get_string_q(a_target);

   if(a_target != target){
       cerr << "TARGET CAST:< "<<target << " > BUT OBTAINED:< " << a_target <<" >"<< endl ;
       exit(0) ;
   }
   assert(a_target == target) ;
   get_string_q(a_string); // getline() that of no skipping headers

//    utl::term::cin2buf_nshq(aBuf, 1023); // getline() that of no skipping headers
//    a_string = aBuf;
   return a_string;
}



string align_token_str_str(const std::string &namedValue, std::string &a_string, bool debug)
{
   std::string cmp("@");
   if (namedValue.substr(0, 1)==cmp)
     //  if (namedValue.substr(0, 1).compare(cmp) == 0)
   {
       align_token_str_str_and_skip_comment(namedValue, a_string);
       if(debug) {
           cout << "#?? <string> Matched variable, (string): {" << endl << namedValue << endl
                << a_string << endl << "#}" << endl << "###" << endl;
       }
       caster[namedValue] = {"string", a_string};
   }
   else
   {//a fault, not just debugging
      cout << endl
           << "*********************************************************" << endl;
      cout << "..Illegal non-@-headed string: {" << namedValue << "}" << endl;
      cout << "Mandatory to call this with head @" << endl;
      cout << "*********************************************************" << endl;
      exit(0);
   } // namedValue << endl ;
   return a_string;
}

void caster_dump()
{
//      print("{}\n",caster.cbegin());
//      print("{}\n",caster.dump(4));
}

#include <sys/stat.h>

bool utl::exists_test3 (const std::string& name) {
    struct stat buffer;
    return (stat (name.c_str(), &buffer) == 0);
}

#include <filesystem>
namespace fs = std::filesystem;

bool utl::doesFileExist(const std::string& filePath) {
    return fs::exists(filePath) && fs::is_regular_file(filePath);
}

#endif
