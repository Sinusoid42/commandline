/**
 *   Command Line Parser helper 
 *
 * Copyright <2022> <Vincent Benedict Winter>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included 
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR 
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR 
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 *  Dynamic Command Line Interpreter in C++
 *  
 *  (Except from std:: libraries) a complete overhaul on the CommandLine
 *
 *  Allows for rich CommandLines, automatic error and print feedback, aswell as callbacks
 *
 *  
 *      cli::CommandLine myCommandLine = cli::CommandLine();
 *      cli::Argument* h = cli::NewArgument(
 *          cli::WILDCARD | cli::OPTION,
 *          "h",
 *           "help",
 *           true,
 *           "This is the default help message for the commandline parser in c++"
 *       );
 *       myCommandLine.addArgument(h);
 *       int err = myCommandLine.parse(argc, argv);
 *       cli::Options* cli_cntr = myCommandLine.parsedArgs();
 *
 *
 *  //TODO
 *      cleanUp
 *      allow repeated parameters for methods
 *      execute methods on selection
 *      operatoroverloading for Options
 *      create cli from config.json!
 *
 *  Useable header file to add a command line to any C++ programm with dynamically allocateable
 *  Arguments, Options and Methods to the Commandline
 *
 *  v0.2.1
 */


#ifndef CLI_PARSER
#define CLI_PARSER
#include <iostream>
#include <vector>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*
    DESCRIPTION:
    Small CLI parser helper

    lets you check for given types, can handle repetitive parameters for different flags
    or methods



    if (!required)
        print("--")
        can have required paramters

*/

namespace cli
{


#define EXIT -1
#define CLI_DTYPE_INT 1
#define CLI_DTYPE_STRING 2
#define CLI_DTYPE_URL 3
#define CLI_DTYPE_FILE 4

#define CLI_DTYPE_UNDEF -1

#define VERBOSE_OFF 0
#define VERBOSE_SIMPLE 1
#define VERBOSE_FULL 2

#define ERR_NO_ERR 1                //no error
#define ERR_UNKOWN_INPUT 2          //unkonwn option
#define ERR_INVALID_INPUT 4         //invalid option/wildcard etc..
#define ERR_REQ_ARG_NOT_FOUND 8     //missing
#define ERR_WRONG_DATA 16           //from data
#define ERR_HELP_WILDCARD 32
#define ERR_NOT_FOUND 64
#define ERR_REQ_PARAM_NOT_FOUND 128


#define _cli_arg_count 5




    /**
     * @brief All possible argumenttypes here in the codebase
     *
     */

    //expose to the outside
    const int NO_ERROR                  = ERR_NO_ERR;
    
    int CommandLineVerbosity            = 0;

    typedef int ArgumentType;

    const ArgumentType OPTION           = 1;
    const ArgumentType PARAM            = 2;
    const ArgumentType WILDCARD         = 4;
    const ArgumentType METHOD           = 8;
    const ArgumentType _NULL_ARG_       = 16;


    /**************************************************************************************************************************************/
    //Utils implementations
    void setCLIVerbosity(int verbosity){
        CommandLineVerbosity = verbosity;
    }

    int strlen(const char* str){
        for(int i=0;;i++)
            if (str[i] == '\0')
                return i+1;
        return 0;
    }


    void write_string(char* dst, char* src){
        for (int i=0;i<strlen(src);i++)
            dst[i] = src[i];
        
    }
    int _compare_cstring(const char* str1, char* str2){
        int l = strlen(str1);
        if (l != strlen(str2)){
            return 0;
        }
        int same = 1;
        for (int i=0;i<l;i++)
            same = same && (str1[i] == str2[i]);
        return same;
    }   

    int _compare_cstring_until(char* str1, char* str2, int l){
        int same = 1;
        for (int i=0;i<l;i++)
            same = same && (str1[i] == str2[i]);
        return same;
    }

    char* combineString(char* str1, char* str2){
        int l1 = strlen(str1);
        int l2 = strlen(str2);
        int l = l1 + l2-1;
        char* r = new char[l];
        for (int i=0;i<l1-1;i++)
            r[i] = str1[i];
        for (int i=0;i<l2;i++)
            r[i+l1-1] = str2[i];
        return r;    
    }


    //Check wether the given argument type of any argument is ok, and can be used properly if not throw an exception
    //returns 0 = false when arg_type is not wellformed, otherwise returns 1 as true
    const int _checkArgType(ArgumentType arg_type){
        if (arg_type&METHOD)
            return !(arg_type& (OPTION | PARAM | WILDCARD | _NULL_ARG_)); //returns 0 if any other than method
        if (arg_type&OPTION)
            return (arg_type | ( arg_type  &WILDCARD)) && !(arg_type&(PARAM | METHOD | _NULL_ARG_));  //returns 0 if method or param
        if (arg_type&PARAM)
            return !(arg_type& (WILDCARD | METHOD | OPTION | _NULL_ARG_));
        if (arg_type&WILDCARD)
            return (arg_type | ( arg_type & OPTION)) && !(arg_type& (METHOD | PARAM | _NULL_ARG_));
        if (arg_type&_NULL_ARG_)
            return !(arg_type& (OPTION | PARAM | WILDCARD | METHOD));
        return 0;
    }

    char* _string_from_argtype(ArgumentType arg_type){

        char* r = "";
        int c=0;
        if (arg_type&METHOD){
            c=1;
            r = combineString(r, "Method");
        }
        if (arg_type&OPTION){
            if (c)
                r = combineString(r, ":");
            r = combineString(r, "Option");
            c=1;
        }
        if (arg_type&PARAM){
            if (c)
                r = combineString(r, ":");
            r = combineString(r, "Param");   
            c=1;
        }
        if (arg_type&WILDCARD){
            if (c)
                r = combineString(r, ":");
            r = combineString(r, "Wildcard");
            c=1;
        }
        if (arg_type&_NULL_ARG_){
            if (c)
                r = combineString(r, ":");
            r = combineString(r, "Null");
        }
        return r;
    }


    char* _argtype_header(ArgumentType arg_type, char* flag){
        if (arg_type&OPTION | arg_type&WILDCARD){
            if (_compare_cstring(flag, "long"))
                return "--";
            if (_compare_cstring(flag, "short"))
                return "-";
        }
    }
    char** copySubArgv(int argc, char** argv, int startpoint){
        char** newargv = (char**)malloc(8*(argc-startpoint));
        for (int i=1;i<argc-startpoint;i++)
        {
            newargv[i] = (char*)malloc(strlen(argv[i+startpoint]));
            write_string(newargv[i], argv[i+startpoint]);
        }
        newargv[0] = (char*)malloc(strlen(argv[0]));
        write_string(newargv[0], argv[0]);
        return newargv;
    }


    //returns a properly formatted verbose debug information about, what the cli interpreter is doing atm => see VERBOSE_OFF; VERBOSE_SIMPLE or VERBOSE_FULL
    const char* _get_verbosity_msg (int key){
        switch(key){
            case 1 :return "=> <cli::{anonymous}::argument_tree()>                                                                          : Created the Argument Tree <argument_tree>\n";
            case 2 :return "=> <map> in <cli::{anonymous}::map()>                                                                           : Created the Argument Map after parsing\n";
            case 3 :return "=> <cli::{anonymous}::argument_tree::addArgument(Argument* arg)>                                                : Added a new Argument to the Argument Tree <argument_tree>\n";
            case 4 :return "=> <cli::Argument::setCallback(int (*func)())>                                                                  : Set the callback function to the <Argument> \n";
            case 5 :return "=> <cli::Argument::setCallback(int (*func)())>                                                                  : ... Success\n";
            case 6 :return "=> <cli::Argument::addParameter(Argument* param)>                                                               : Adding a new Parameter to <Argument>\n";
            case 7 :return "=> <cli::Argument::addParameter(Argument param)>                                                                : Adding a new Parameter to <Argument>\n";
            case 8 :return "=> <cli::CommandLine::addArgument(Argument* arg)>                                                               : Adding a new Argument to <CommandLine>\n";
            case 9 :return "=> <cli::CommandLine::addArgument(Argument arg)>                                                                : Adding a new Argument to <CommandLine>\n";
            case 10:return "=> <cli::CommandLine::CommandLine()>                                                                            : Created a new <CommandLine>\n";
            case 11:return "=> <cli::CommandLine::CommandLine(const char *config_file)>                                                     : Created a new <CommandLine>\n";
            case 12:return "=> <cli::CommandLine::CommandLine(const char* config_file)>                                                     : This Constructor requires a json config_file\n";
            case 13:return "=> <cli::CommandLine::CommandLine(int verbose)>                                                                 : Created a new <CommandLine> in\n";
            case 14:return "=> <cli::CommandLine::CommandLine(int verbose)>                                                                 : This Constructor allows higher and custom per callback verbosity!\n";
            case 15:return "=> <cli::CommandLine::CommandLine(const char *config_file, int verbose)>                                        : Created a new <CommandLine>\n";
            case 16:return "=> <cli::CommandLine::CommandLine(const char* config_file)>                                                     : This Constructor requires a json config_file\n";
            case 17:return "=> <cli::NewArgument(arg_type, short_flag, long_flag, required, help)>                                          : Creating a new Argument\n";
            case 18:return "=> <cli::NewParameter(const char* dtype)>                                                                       : Creating a new Parameter with string specified datatype, supports default dtypes (int, double/float, string, file_path, url or custom [use the callback function for this]])\n";
            case 19:return "=> <cli::Argument::setDatatypeCheckCallback(int (*func)(const char* d))>                                        : Set the datatype check callback function function to the <Argument:Parameter> \n";
            case 20:return "=> <cli::Argument::setDatatypeCheckCallback(int (*func)(const char* d))>                                        : ... Success\n";
            case 21:return "=> <cli::Argument::setMethod(int (*func)(std::vector<char*> params, std::vector<Option> options))>              : Set the cb of a method, accepting parameters and options \n";
            case 22:return "=> <cli::CommandLine::string()>                                                                                 : Printing the CommandLine\n";
            default:return "ERROR - No Verbosity Information Available!\n";
        }
    };

     /**************************************************************************************************************************************
     * OPTIONS
     * 
    */
    //Options when retrieved from the CLI or as arguments to any given method or option
    struct Options
    {
        private: 
            ArgumentType arg_type;
            char*   key;                    //represents the long flag from the cli argument

            char*   data;                   //the exact entered argv on call time

            std::vector<char*>  argv;       //the actually parsed values that come with this cli
            int     argc;                   //the count of arguments and options alloed by this argument
            bool parsed;                    //was this argument parsed or successfully parsed ?
            std::vector<Options*> options;   //all the other options that come after this >= addressable with ```options["my_option"]````
        public:       
            Options(){
                this->options =  std::vector<Options*>();
                this->parsed = false;
                this->argc = 0;
                this->argv = std::vector<char*> ();
                this->arg_type = _NULL_ARG_;
                this->setKey("__null__");
                this->setData("__null__");    
                
            }
            Options* operator[](const char* key){
                Options* option  = new Options();;
                for (int i=0;i<this->argc;i++)
                    if (_compare_cstring(key, this->options[i]->key))
                        {
                            option =  this->options[i];
                            return option;
                        };
                return option;
            };
            Options* get(const char* key){
                Options* option = new Options();
                for (int i=0;i<this->argc;i++)
                    if (_compare_cstring(key, this->options[i]->key))
                        {
                            option =  this->options[i];
                            return option;
                        };
                return option;
            };
            ~Options(){
                delete(this->key);
                for (char* a:this->argv)
                    delete(a);
                delete(&this->options);
            }
            void setArgType(ArgumentType t){
                this->arg_type = t;
            }
            char* getKey(){
                return this->key;
            }
            void setKey(char* key){
                this->key = (char*)malloc(strlen(key));
                write_string(this->key, key);
            }
            void setParsed(bool parsed){
                this->parsed = parsed;
            }
            void setArgc(int arg_count){
                this->argc = arg_count;
            }
            int getArgc(){
                return this->argc ;
            }
            bool getParsed(){
                return this->parsed;
            }
            void addOptions(Options* options){
                this->options.push_back(options);
            }
            void addArgv(char* argument){
                this->argv.push_back(argument);
            }
            std::vector<char*> getVars(){
                return this->argv;
            }
            void setData(char* data){
                this->data = (char*)malloc(strlen(data));
                write_string(this->data, data);
            }
            char* getData(){
                return this->data;
            }
            void concat(std::string* str, int indent){
                for (int i=0;i<indent;i++)
                    (*str) += " ";
                (*str) += "-> <";
                (*str) += this->key;
                (*str) += ">\n";

                if (indent>8)
                    return;
                for (int q=0;q<this->argc;q++)
                   this->options[q]->concat(str, indent+2);

            
            }
            std::string string(){
                std::string result = std::string( "<struct::Options::string()>\n");
                this->concat(&result, 3);
                return result;
            }
    };

    const char *ErrParse(int ErrCode)
    {
        if (ErrCode & ERR_HELP_WILDCARD){
            if (ErrCode & 0xFE == 0)
            {
                return "<Help - Wildhard> No Error.\n";
            }
            if (ErrCode & ERR_INVALID_INPUT){
                return "<Help - Wildhard> Invalid Input.\n";
            }
            if (ErrCode & ERR_REQ_ARG_NOT_FOUND){
                return "<Help - Wildhard> Required Argument could not be found.\n";
            }
            if (ErrCode & ERR_WRONG_DATA){
                return "<Help - Wildhard> The input data(type) is incorrect.\n";
            }
            if (ErrCode & ERR_REQ_PARAM_NOT_FOUND){
                return "<Help - Wildhard> Required Parameter could not be found.\n";
            }
        }
        
        if (ErrCode & 0xFE == 0)
        {
            return "No Error.\n";
        }
        if (ErrCode & ERR_INVALID_INPUT){
            return "Invalid Input.\n";
        }
        if (ErrCode & ERR_REQ_ARG_NOT_FOUND){
            return "Required Argument could not be found.\n";
        }
        if (ErrCode & ERR_WRONG_DATA){
            return "The input data(type) is incorrect.\n";
        }
        if (ErrCode & ERR_REQ_PARAM_NOT_FOUND){
            return "Required Parameter could not be found.\n";
        }
        return "Err - No Error Description found. sanity check advised or run with higher verbosity (if possible).\n";
    };

    /**************************************************************************************************************************************
     * ARGUMENT
     * 
    */
    /**
     * @brief The Argument Struct
     * 
     * This datatype represents a single parameter or argument available to the CLI Parser/Interpreter
     * Arguments can be 
     *      WILDCARDS   => Executing or performing some kind of action => callback, but make the cli interpreter exit the entire pipeline
     *                  => is useful for debug or help methods
     *      OPTIONS     => Get added a "--" or "-" to their keys when called, just like any other regular cli interpreter
     *      METHOD      => definately calls the callback function, if not present throws an exception (eg docker run => does "something")
     *      PARAM       => most likely is always some kind of datatype or parameter(s), that some kind of option or method is requiring
     *                  => when more than one parameter is available this will execute the method's
     */
    struct Argument
    {
    public:
        ArgumentType                            arg_type;

        // variables for when this argument is a "parameter" of an option or method
        int                                     dtype;
        char*                                   dtype_custom;
        // the data
        char*                                   the_data; // will need marshalling in parser, wether its file => check for availablability, url => correct formatting ?, int or float

        int                                     is_custom_dtype;
        
        
        int                                     parsed;

        // if this is a method or option, the keys by which it can be read (beware the shortcuts -t -k -l = -tkl = -lkt)
        char*                                   long_flag;
        char*                                   short_flag;// if available and no parameters are needed => has no "arguments" allow for combination
        char**                                  excludes; // if any of the excluded arguments are present, throw an error and escape
        int                                     exclude_count;
        // help single liner for that this cli argument needs as a paramteroo
        char*                                   help_msg;

        // is this a parameter and has available choices or is required
        char**                                  choices;
        int                                     choice_count;
        // is this flag, option, method or parameter required
        bool                                    required;

        // is this probably a function that shall be called or has a function to be called, can be created with a lambda expression outside of the cli command line interpreter
        int                                     (*callback)();
        int                                     (*method)(int argc, char** argv, Options* options);

        //when using a custom dtype as datatype this is available as interface to create a lambda callback function to check wether the parsed dtype was correct
        int                                     (*dtype_check_cb)(const char * d);


        // datastructure and storage
        std::vector<Argument*>                  arguments; //can be parameters of followup options
        Argument *parent; // first one is root, eg the program itself, can be captured then with arguments[0]

    /**
     * @brief Constructors
     * 
     */
    public:
        Argument();
        Argument(
            ArgumentType                        arg_type,
            char*                               short_flag,
            char*                               long_flag,
            bool                                required,
            char*                               help_msg);
    /**
     * @brief Public Methods for each Argument from the CommandLine
     * 
     */
    public:
        Argument *addArgument(Argument *arg);

        std::vector<Argument*> getArguments();
        Argument *addChoices(const std::initializer_list<char *> &list);
        Argument *setCallback(int (*func)());
        Argument *setMethod(int (*func)(int argc, char** argv, Options* options));
        Argument *setDatatypeCheckCallback(int (*func)(const char* d));
        ArgumentType getArgType();
        Argument *setRequired(bool rqrd);
        //std::vector<Argument> getArguments();
        char* string(char* spacer);
        int parse(int argc, char** argv, Options* options);
    };

     /**
     * @brief THE ARGPRASE FUNCTION
     * 
     * Can also be used to sanity check arguments vs the argv input
     * 
     */
    int parseArg(Argument* arg, char* thearg){


        if (arg->arg_type&OPTION)
            if (_compare_cstring(thearg, combineString(_argtype_header(arg->arg_type, "long") ,arg->long_flag)) || 
                _compare_cstring(thearg, combineString(_argtype_header(arg->arg_type, "short") ,arg->short_flag))){
            return ERR_NO_ERR;
        }
        if (arg->arg_type&WILDCARD)
            if (_compare_cstring(thearg, combineString(_argtype_header(arg->arg_type, "long") ,arg->long_flag)) || 
                _compare_cstring(thearg, combineString(_argtype_header(arg->arg_type, "short") ,arg->short_flag))){
                    return ERR_NO_ERR;
        }
        if (arg->arg_type&METHOD){
            if (_compare_cstring(thearg,arg->long_flag) || 
                _compare_cstring(thearg,arg->short_flag))
            return ERR_NO_ERR;
        }
        

        if (arg->arg_type&PARAM){

            

            if (_compare_cstring_until(thearg, "-", 1) || _compare_cstring_until(thearg, "--", 2)){
                return ERR_REQ_PARAM_NOT_FOUND | ERR_INVALID_INPUT | ERR_WRONG_DATA; //probably attached another option instead of an paramter
            }

            if (arg->is_custom_dtype){
                return arg->dtype_check_cb(thearg);
            }
            if (_compare_cstring(arg->dtype_custom, "string")){
                return strlen(thearg)>0 ? ERR_NO_ERR : ERR_WRONG_DATA;
            }
            if (_compare_cstring(arg->dtype_custom, "int")){
                char* end;
                long number = strtol(thearg, &end, 0);
                if (*end == '\0')
                    return ERR_NO_ERR;
                else
                    return arg->required?ERR_WRONG_DATA | ERR_REQ_PARAM_NOT_FOUND:ERR_WRONG_DATA;
                
            }
            if (_compare_cstring(arg->dtype_custom, "file")){
                if (access(thearg, F_OK) == 0) {
                    return ERR_NO_ERR;
                } else {
                    return ERR_WRONG_DATA;
                }
            }
            if (_compare_cstring(arg->dtype_custom, "url")){
                if (_compare_cstring_until(thearg, "http", 4))
                    return ERR_NO_ERR;
                return 0;
            }
            if (_compare_cstring(arg->dtype_custom, "file")){
                    //TODO
                    return 1;
            }
            if (arg->dtype_check_cb != nullptr){
                arg->dtype_check_cb(thearg);
            }
        }
        return arg->required?ERR_REQ_ARG_NOT_FOUND:ERR_NOT_FOUND;
    };


    Argument* NewArgument(ArgumentType Type,
                          char *short_flag,
                          char *long_flag,
                          bool required,
                          char *help);
    /**
     * @brief Here are hidden implementations, that should not be exposed to the outside
     * Hidden Namespace with state and container variables
     * 
     */
    namespace
    {
        struct argument_tree
        {
            Argument *root;
            int size;
            int methods;
            int options;
            int* verbosity;

            argument_tree(int* cli_verbosity);
            int addArgument(Argument* arg);
        };


        struct map_node
        {
            map_node();
        };

        /**
         * @brief Internal Datastructure to capture and hold parsed values and display them as
         *
         */
        struct map
        {
            map(){
                 if (CommandLineVerbosity>=VERBOSE_FULL){
          
                    std::cout << _get_verbosity_msg(2);
                }
            };

            template <typename dtype>
            std::vector<dtype> operator[](const char *key)
            {
                // check for key, if not parsed, return error method and help menu
                // if node present, check the datatype of the "wanted" dtype and the currently available "dtype"
                return std::vector<dtype>();
            };
        };


        argument_tree::argument_tree(int* cli_verbosity){
            this->verbosity = cli_verbosity;
            this->root = new Argument(
                _NULL_ARG_,
                "r",
                "root",
                true,
                "The root argument of the argument tree");
            this->methods = 0;
            this->options = 0;


            if (CommandLineVerbosity>=VERBOSE_FULL){
                std::cout << _get_verbosity_msg(1);
            }
        };



        int argument_tree::addArgument(Argument* arg){
            if (CommandLineVerbosity>=VERBOSE_FULL){
                std::cout << _get_verbosity_msg(3);
            }
              
            this->methods += (arg->getArgType()&METHOD) > 0;
            this->options += (arg->getArgType()&OPTION) > 0;
    
            this->root->addArgument(arg);
            return ERR_NO_ERR;
        };

    };

   

    /**
     *   THE COMMAND LINE STRUCT
     *   This struct encapsulates the datastructure after parsing, the arguments in the argument tree
     *   Traverses the argument input stack and represents the pipeline for parsing arguments
     *
     */
    struct CommandLine
    {

    private:
        int verbosity;
        argument_tree *args;
        int* cli_verbosity;

        Options* options;

    public:
        CommandLine();
        CommandLine(const char *config_file);
        CommandLine(int verbose);
        CommandLine(const char *config_file, int verbose);
        Options* build_options_tree();
        int addArgument(Argument *arg);
        int parse(int argc, char **argv);
        void printHelp();
        void printHelpFull();
        Options* parsedArgs();
        Argument *operator[](char *key);
        char* string();
    };


    /**************************************************************************************************************************************
     * COMMANDLINE IMPLEMENTATIONS
     * 
    */

    /**
     * @brief Construct a new Command Line:: Command Line object
     * 
     */
    CommandLine::CommandLine(){
        if (CommandLineVerbosity>=VERBOSE_SIMPLE){
            std::cout << _get_verbosity_msg(10);
        }
        this->verbosity = 0;
        this->cli_verbosity = &this->verbosity;
        this->args = new argument_tree(this->cli_verbosity);
    };

    CommandLine::CommandLine(const char *config_file){
        if (CommandLineVerbosity>=VERBOSE_SIMPLE){
            std::cout << _get_verbosity_msg(11);
        }
        if (CommandLineVerbosity>=VERBOSE_FULL){
            std::cout << _get_verbosity_msg(12);
        }
        this->args = new argument_tree(this->cli_verbosity);
    };

    CommandLine::CommandLine(int verbose){
        if (CommandLineVerbosity>=VERBOSE_SIMPLE){
            std::cout << _get_verbosity_msg(13);
        }
        if (CommandLineVerbosity>=VERBOSE_FULL){

            std::cout << _get_verbosity_msg(14);
        }
        this->args = new argument_tree(this->cli_verbosity);
    };

    CommandLine::CommandLine(const char *config_file, int verbose){
        if (CommandLineVerbosity>=VERBOSE_SIMPLE){
            std::cout << _get_verbosity_msg(15);
        }
        if (CommandLineVerbosity>=VERBOSE_FULL){
            std::cout << _get_verbosity_msg(16);
        }
        this->args = new argument_tree(this->cli_verbosity);
    };

    /**************************************************************************************************************************************/


 
    
    int CommandLine::addArgument(Argument* arg){
        if (CommandLineVerbosity>=VERBOSE_FULL){
            std::cout << _get_verbosity_msg(9);
        }

        this->args->addArgument(arg);
        return ERR_NO_ERR;
    };

    void CommandLine::printHelp(){
        
        std::cout << "USAGE:" << std::endl;
        std::cout << "Required:\n";
        int indent = 4;
        for (int i=0;i<this->args->root->arguments.size();i++){ 
            if (this->args->root->arguments[i]->required){
                for (int ind=0;ind<indent;ind++)
                    std::cout << " ";
                std::cout << "[" << this->args->root->arguments[i]->long_flag << " : ";
                if (this->args->root->arguments[i]->arguments.size()>0){
                    for(int k=0;k<this->args->root->arguments[i]->arguments.size();k++){
                        if (this->args->root->arguments[i]->arguments[k]->required)
                            std::cout << this->args->root->arguments[i]->arguments[k]->long_flag << "!";
                        else
                            std::cout << this->args->root->arguments[i]->arguments[k]->long_flag;
                        if (k<k<this->args->root->arguments[i]->arguments.size()-1)
                            std::cout << ", ";
                        else
                            std::cout << "]\n";
                    }
                }
                
            }
            
        }
        std::cout << "Options:\n";
        indent = 4;
        for (int i=0;i<this->args->root->arguments.size();i++){ 
            if (this->args->root->arguments[i]->arg_type | OPTION | WILDCARD){
                for (int ind=0;ind<indent;ind++)
                    std::cout << " ";
                std::cout << "[ " << this->args->root->arguments[i]->long_flag << ((this->args->root->arguments.size()>i+1)?" : ":" ");
                if (this->args->root->arguments[i]->arguments.size()>0){
                    for(int k=0;k<this->args->root->arguments[i]->arguments.size();k++){
                        if (this->args->root->arguments[i]->arguments[k]->required)
                            std::cout << this->args->root->arguments[i]->arguments[k]->long_flag << "!";
                        else
                            std::cout << this->args->root->arguments[i]->arguments[k]->long_flag;
                        if (k<k<this->args->root->arguments[i]->arguments.size()-1)
                            std::cout << ", ";
                        else
                            std::cout << "]\n";
                    }
                }
                else
                    std::cout << "]\n";
            }
        }
    };

    void CommandLine::printHelpFull(){
        
        std::cout << "USAGE:" << std::endl;
        std::cout << "    Use '-vCLI | --verboseCLI' for more Debug Information\n\n";
        std::cout << "Required:\n";
        int indent = 4;
        for (int i=0;i<this->args->root->arguments.size();i++){ 
            if (this->args->root->arguments[i]->required){
                for (int ind=0;ind<indent;ind++)
                    std::cout << " ";
                std::cout << "[" << this->args->root->arguments[i]->long_flag << " : ";
                if (this->args->root->arguments[i]->arguments.size()>0){
                    for(int k=0;k<this->args->root->arguments[i]->arguments.size();k++){
                        if (this->args->root->arguments[i]->arguments[k]->required)
                            std::cout << "<"<<this->args->root->arguments[i]->arguments[k]->long_flag << " : <!" << this->args->root->arguments[i]->arguments[k]->dtype_custom <<">";
                        else
                            std::cout << this->args->root->arguments[i]->arguments[k]->long_flag<< " : <" << this->args->root->arguments[i]->arguments[k]->dtype_custom <<">";
                        if (k<k<this->args->root->arguments[i]->arguments.size()-1)
                            std::cout << "| ";
                    }
                    std::cout << "]\n";
                }
            }
            
        }
        std::cout << "Options:\n";
        indent = 4;
        for (int i=0;i<this->args->root->arguments.size();i++){ 
            if (this->args->root->arguments[i]->arg_type | OPTION | WILDCARD){
                for (int ind=0;ind<indent;ind++)
                    std::cout << " ";
                std::cout << (strlen(this->args->root->arguments[i]->short_flag)>1?"-":" ") << (strlen(this->args->root->arguments[i]->short_flag)>0?this->args->root->arguments[i]->short_flag : "   ");
                for (int p=0;p<4-strlen(this->args->root->arguments[i]->short_flag);p++)
                    std::cout << " ";
                std::cout << (this->args->root->arguments[i]->arg_type&(OPTION | WILDCARD)?" |  --" : " >") << this->args->root->arguments[i]->long_flag << ((this->args->root->arguments.size()>0)?" : [":" ");
                if (this->args->root->arguments[i]->arguments.size()>0){
                    for(int k=0;k<this->args->root->arguments[i]->arguments.size();k++){
                        if (this->args->root->arguments[i]->arguments[k]->required)
                            std::cout << this->args->root->arguments[i]->arguments[k]->long_flag << " : <!" << this->args->root->arguments[i]->arguments[k]->dtype_custom <<">";
                        else
                            std::cout << this->args->root->arguments[i]->arguments[k]->long_flag << " : <" << this->args->root->arguments[i]->arguments[k]->dtype_custom <<">";
                        if (k<k<this->args->root->arguments[i]->arguments.size()-1)
                            std::cout << "| ";   
                    }
                    std::cout << "]\n";
            
                }
                else
                    std::cout << "]\n";
            }
            for (int ind=0;ind<indent;ind++)
                    std::cout << " ";
                std::cout << "        -> <" << this->args->root->arguments[i]->help_msg << ">\n\n";
        }
    };



    Options* CommandLine::build_options_tree(){


        this->options = new Options();

        this->options->setKey(this->args->root->long_flag);
        this->options->setParsed(true);
        this->options->setArgc(0);

        return this->options;
    }


    int CommandLine::parse(int argc, char **argv)
    {
        Options* options_tree = this->build_options_tree();
        int help=0;
        //check for verbosity
        for (int i = 0; i < argc; i++)
        {
            if (_compare_cstring(argv[i], "-vCLI") || _compare_cstring(argv[i], "--verboseCLI")){
                this->verbosity = VERBOSE_FULL;
            }
             if (_compare_cstring(argv[i], "-h") || _compare_cstring(argv[i], "--help")){
                this->printHelpFull();
                help = ERR_HELP_WILDCARD;
            }
        }

        if (this->verbosity>=VERBOSE_SIMPLE){
            if (argc>=0){
                std::cout << "******************\nRunning the CommandLine : (" << argv[0] <<")\n******************\n";
            }
        }
        int err=0;


        if (this->verbosity>=VERBOSE_SIMPLE)
            std::cout << "<CommandLine::parse(int argc, char **argv)>"<< std::endl;

        char** newargv= copySubArgv(argc, argv, 1);
        //run over all top level arguments, run recursive into each branch checking wether the argument parsing was successful
        for (int a=0;a<this->args->root->getArguments().size();a++){
            Argument* arg = this->args->root->getArguments()[a];
            if (this->verbosity>=VERBOSE_FULL)
                std::cout << "Parsing recursively the Argument: "<< arg->long_flag <<std::endl;
            
            err |= arg->parse(argc, argv, options_tree) | err;
            if (!arg->required && !(err&ERR_REQ_PARAM_NOT_FOUND)){
                 err = err & ~ERR_REQ_ARG_NOT_FOUND;   
            }
            if (arg->required && arg->arg_type&PARAM)
                err |=ERR_REQ_PARAM_NOT_FOUND;
       
        }

        if (this->verbosity>=VERBOSE_SIMPLE)
            std::cout << "<Finished parsing, start cleaning>"<< std::endl;


        //std::cout << err << std::endl;

        if (err== ERR_NO_ERR || help){
            err |= help;
                //TODO store all the values in an option contianer and parse everything 

        }else{
            char* errmsg;


            if (err & ERR_UNKOWN_INPUT || err & ERR_INVALID_INPUT){
                //PRINT HELP
                this->printHelp();
            }
            if (err & ERR_REQ_ARG_NOT_FOUND){
                //scan for required argument and print the errro to console
                this->printHelp();
            }
            if (err & ERR_WRONG_DATA){
                this->printHelp();
            }

        }
        for (int q=0;q<argc-1;q++)
                    free(newargv[q]);

                free(newargv);
        return err;
    };


    Argument* CommandLine::operator[](char *key)
    {

        return nullptr;
    };    

    /**
    *   Contianer after parsing argc, argv matching the given selection of parameters
    *
    */
    Options* CommandLine::parsedArgs()
    {
        return this->options;
    }

    /**
    *   Returns a humanreadable printable string that contains information about the datastructure within this command line
    *   
    */
    char* CommandLine::string(){
        char* r = " <struct::CommandLine::string()>\n";


        if (CommandLineVerbosity>=VERBOSE_SIMPLE){
            std::cout << _get_verbosity_msg(22);
        }

        r = combineString(r, this->args->root->string("  -> "));
        

       
        return r;        
    }


   

    /**************************************************************************************************************************************
     * ARGUMENT IMPLEMENTATIONS
     * 
    */
    /**
     * @brief Construct a new empty null Argument:: Argument object
     * 
     */
    Argument::Argument(){
        this->arguments = std::vector<Argument*>();
        this->arg_type = _NULL_ARG_;
        this->long_flag = "null";
        this->short_flag = "n";

        this-> dtype = CLI_DTYPE_UNDEF;
        this-> dtype_custom = "";
        this-> the_data = "";

        this->excludes = new char*[0];
        this->exclude_count = 0;

        this->required = false;
        this->parsed = 0;   
        this->is_custom_dtype   = 0;
        this-> callback = [](){
            return NO_ERROR;
        };
        this->method = [](int argc, char** argv, Options* options){
            return NO_ERROR;
        };
        this->dtype_check_cb = [](const char* dtype){
            return NO_ERROR;
        };
        this->parent = nullptr;
    };

    /**
     * @brief Construct a new Argument:: Argument object
     * 
     * @param arg_type 
     * @param short_flag 
     * @param long_flag 
     * @param required 
     * @param help_msg 
     */
    Argument::Argument(
        ArgumentType arg_type,
        char* short_flag,
        char* long_flag,
        bool required,
        char* help_msg)
    {

        this->arg_type =arg_type;
     
        this->short_flag = new char[strlen(short_flag)];
        write_string(this->short_flag, short_flag);

        this->long_flag = new char[strlen(long_flag)];
        write_string(this->long_flag, long_flag);

        this->help_msg = new char[strlen(help_msg)];
        write_string(this->help_msg, help_msg);

        this->arg_type      = arg_type;
        this->required      = required;
        this->parsed = 0;
        this->is_custom_dtype   = 0;

        this->arguments = std::vector<Argument*>();

        this-> dtype = CLI_DTYPE_UNDEF;
        this-> dtype_custom = "";
        this-> the_data = "";

        this->excludes = new char*[0];
        this->exclude_count = 0;
      
        this-> callback = [](){
            return NO_ERROR;
        };
        this->method = [](int argc, char** argv, Options* options){
            return NO_ERROR;
        };
        this->dtype_check_cb = [](const char* dtype){
            return NO_ERROR;
        };

        this->parent = nullptr;

                

        //TODO   

    };

  /**************************************************************************************************************************************/
   



    int attachOptions(Argument* arg, Options* options,  int i,  char** argv, int argc){
   
  
  
        Options* nop = new Options();
        nop->setParsed(true);
        nop->setData(argv[i]);
        nop->setArgType(arg->arg_type);
        nop->setKey(arg->long_flag);

        options->setArgc(options->getArgc()+1);
        options->addOptions(nop);
        
    }   



    /*
    int Argument::parse(int argc, char** argv, Options* options){
        
        int e = 0;
        int err_glob= 1;
        if (this->required && argc<2)
            e |= ERR_REQ_ARG_NOT_FOUND;
        err_glob |= e;
       


        if (argc < 2){
            return err_glob;
        }
        

        for (int i=1;i<argc;i++){
            e = 0;

            //parse the current argument
            e |= parseArg(this, argv[i]);

            err_glob =e;
            if (e  != ERR_NO_ERR){
                continue;
            }

            if(i< argc-1 && e&ERR_NO_ERR){

                attachOptions(this, options, i, argv, argc);
                if (this->arguments.size()==0){
                    return err_glob;
                }
                char** newargv= copySubArgv(argc, argv, i);
                if (e & ERR_NO_ERR == ERR_NO_ERR){
                   
                    //match found, check if parameters are there
                    //are there still arguments left in the tree =?
                    
                    for (int q=0;q<this->arguments.size();q++){
                        e |= this->arguments[q]->parse(argc-i, newargv, options->get(this->long_flag)); //adds itself to the tree of argument
                        if (e& ERR_REQ_ARG_NOT_FOUND && !this->required && this->arguments[q]->required){
                            e = e & ~ERR_REQ_ARG_NOT_FOUND;
                        }
                    }


                    if (e == ERR_NO_ERR){
                        for (int q=0;q<this->arguments.size();q++){
                            //int k= this->arguments[q]->parse(argc-1, newargv, options->get(this->long_flag)); //adds itself to the tree of arguments

                        }
                        return e;
                    }else{
                    }
                    err_glob |= e;
                    }
                else{
                   
                    // if (this->arguments.size()){
                    //     for (int q=0;q<this->arguments.size();q++){
                    //         //e |=this->arguments[q]->parse(0,argv, options->get(this->long_flag));
                    //     }
                    // }
                    
                }   
                 err_glob |= e;


                //the parser jumped recursively into the argument tree
                if (err_glob & ERR_NO_ERR){
                    //Argument a = this->
                }
                for (int q=0;q<argc-1;q++)
                    free(newargv[q]);
                free(newargv);
            }
            else{
                attachOptions(this, options, argc-1, argv, argc);
                return err_glob;
            }
        }

        if (argc == 2){
            return err_glob;
        }

        err_glob |= e;

        return err_glob;
    }*/

    int Argument::parse(int argc, char** argv, Options* options){
       
        int e = 0;
        int err_glob= 1;


        if (this->required && argc<2)
            e |= (this->required ? ERR_REQ_ARG_NOT_FOUND : ERR_NO_ERR);
        err_glob |= e;



        if (argc < 2){
            return err_glob;
        }
        

        for (int i=1;i<argc;i++){
            e = 0;
           
            //parse the current argument
            e |= parseArg(this, argv[i]);
           
            err_glob =e;
           
            if (e  != ERR_NO_ERR){
                continue;
            }

            if(i< argc-1 && e&ERR_NO_ERR){

                attachOptions(this, options, i, argv, argc);
                if (this->arguments.size()==0){
                    return err_glob;
                }
                char** newargv= copySubArgv(argc, argv, i);
                if (e & ERR_NO_ERR == ERR_NO_ERR){
                   
                    //match found, check if parameters are there
                    //are there still arguments left in the tree =?
                    
                    for (int q=0;q<this->arguments.size();q++){
                        e |= this->arguments[q]->parse(argc-i, newargv, options->get(this->long_flag)); //adds itself to the tree of argument



                        if  ( e& ERR_REQ_ARG_NOT_FOUND && (!this->required && this->arguments[q]->required)){
                            e = e & ~ERR_REQ_ARG_NOT_FOUND;
                        }
                    }


                    if (e == ERR_NO_ERR){
                        for (int q=0;q<this->arguments.size();q++){
                            //int k= this->arguments[q]->parse(argc-1, newargv, options->get(this->long_flag)); //adds itself to the tree of arguments

                        }
                        return e;
                    }else{
                    }
                        err_glob |= e;
                        return err_glob;
                    }
                else{
                   
                    // if (this->arguments.size()){
                    //     for (int q=0;q<this->arguments.size();q++){
                    //         //e |=this->arguments[q]->parse(0,argv, options->get(this->long_flag));
                    //     }
                    // }
                    
                }   
                 err_glob |= e;


                //the parser jumped recursively into the argument tree
                if (err_glob & ERR_NO_ERR){
                    //Argument a = this->
                }
                for (int q=0;q<argc-1;q++)
                    free(newargv[q]);
                free(newargv);
            }
            else{
                 
                attachOptions(this, options, argc-1, argv, argc);
                for (int g=0;g<this->arguments.size()>0;g++)
                    if (this->arguments[g]->required)
                        err_glob |=(this->arguments[g]->arg_type & PARAM? (this->required && this->arg_type&(OPTION | WILDCARD | METHOD)? ERR_REQ_PARAM_NOT_FOUND : ERR_NO_ERR) : ERR_REQ_ARG_NOT_FOUND);
                return err_glob;
            }
        }

        for (int g=0;g<this->arguments.size()>0;g++)
            if (this->arguments[g]->required)
                err_glob |=(this->arguments[g]->arg_type & PARAM? (this->required && this->arg_type&(OPTION | WILDCARD | METHOD)? ERR_REQ_PARAM_NOT_FOUND : ERR_NO_ERR) : ERR_REQ_ARG_NOT_FOUND);

        if (argc == 2){
            return err_glob;
        }
        err_glob |= e;


        return err_glob;
    }


    Argument *Argument::setCallback(int (*func)())
    {   
        if (CommandLineVerbosity>=VERBOSE_FULL)
            std::cout << _get_verbosity_msg(4);
        
        //check for functionalities
        //if verbose print
        
        
        this->callback = func;

         if (CommandLineVerbosity>=VERBOSE_FULL)
            std::cout << _get_verbosity_msg(5);
        

        return this;
    };

    Argument *Argument::setDatatypeCheckCallback(int (*func)(const char* d))
    {   
        if (CommandLineVerbosity>=VERBOSE_FULL)
            std::cout << _get_verbosity_msg(19);
        
        //check for functionalities
        //if verbose print
        this->dtype_check_cb = func;
        this->is_custom_dtype = func != nullptr;
         if (CommandLineVerbosity>=VERBOSE_FULL)
            std::cout << _get_verbosity_msg(20);
        

        return this;
    };



    Argument * Argument::setRequired(bool rqrd){
        this->required = rqrd;

        return this;
    };
    Argument *Argument::setMethod(int (*func)(int argc, char** argv, Options* options)){
        if (CommandLineVerbosity>=VERBOSE_FULL){
            std::cout << _get_verbosity_msg(21);
        }

        this->method = func;
        
        return this;
    };
    




    //adds a argumentparameter by reference
    Argument* Argument::addArgument(Argument* arg)
    {
        if (CommandLineVerbosity>=VERBOSE_FULL){
            std::cout << _get_verbosity_msg(6);
        } 
        this->arguments.push_back(arg);
       
        return this;
    };

    
  

    //returns the argumenttype of an argument
    ArgumentType Argument::getArgType(){
        return this->arg_type;
    }

    std::vector<Argument*> Argument::getArguments(){
        return this->arguments;
    };

    char* Argument::string(char* spacer){
        
        char* r = "<Arg: ";
        r = combineString(spacer, r);
        r = combineString(r, this->long_flag);
        r = combineString(r, " | Type:");
        r = combineString(r, _string_from_argtype(this->arg_type));
        if (this->arg_type & PARAM){
            r = combineString(r, combineString(" | dtype: ", this->dtype_custom));
        }
        r = combineString(r, ">\n");
        for (int i=0;i < this->arguments.size();i++){
            r = combineString(r,this->arguments[i]->string(combineString("   ", spacer)));
        }
        return r;
    }


    /**************************************************************************************************************************************/
    /**
     * Allows to create a new CLI argument, similar to those from python
     *
     *
     */
    Argument* NewArgument(
        ArgumentType                arg_type,
        char *                      short_flag,
        char *                      long_flag,
        bool                        required,
        char *                      help
        )
    {
        if (CommandLineVerbosity>=VERBOSE_SIMPLE){
            std::cout << _get_verbosity_msg(17);
        }
        if (!_checkArgType(arg_type))
            return new cli::Argument();
        Argument* arg = new Argument(
            arg_type,
            short_flag,
            long_flag,
            required,
            help);
        //TODO
        //sanity checks
        
        return arg;
    };

    /**
     * @brief Creates a new param, datatype is still argument, after all, all strings entered are arguments parseable
     * 
     * @param dtype 
     * @return Argument 
     */
    Argument* NewParamter(
            
         char* title,
         char *dtype){

        if (CommandLineVerbosity>=VERBOSE_SIMPLE)
            std::cout << _get_verbosity_msg(18);
        
        Argument* arg = new Argument(
            PARAM,
            title,
            title,
            false,
            "");
        arg->dtype_custom = new char[strlen(dtype)];
        write_string(arg->dtype_custom, dtype);

        return arg;
    };
}
#endif