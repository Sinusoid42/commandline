#include "iostream"
#include "../../src/commandline.hpp"


int main(int argc, char** argv){


    cli::CommandLineVerbosity = 0 ;
    cli::CommandLine myCommandLine = cli::CommandLine();
    
  

    cli::Argument* h = cli::NewArgument(
        cli::WILDCARD | cli::OPTION,
        "tt",
        "testing",
        false,
        "This is the default help message for the commandline parser in c++"
    )->addArgument(cli::NewParamter("test", "string"));

    myCommandLine.addArgument(h);


  
    cli::Argument* mynextArg= cli::NewArgument(
        cli::OPTION,
        "r",
        "reference",
        false,
        "This is the default help message for the commandline parser in c++"
    )->addArgument(cli::NewParamter("number", "int"));
    myCommandLine.addArgument(mynextArg);


     cli::Argument* mynextArg2= cli::NewArgument(
        cli::OPTION,
        "q",
        "question",
        false,
        "This is the default help message for the commandline parser in c++"
    )->addArgument(cli::NewParamter("theq", "string"));
    myCommandLine.addArgument(mynextArg2);


    
    

    int err = myCommandLine.parse(argc, argv);

    if (err){
        std::cout << cli::ErrParse(err) << std::endl;
    }

    cli::Options* cli_cntr = myCommandLine.parsedArgs();


    std::cout << cli_cntr->string() << std::endl;

    
    cli::Options* rst2=  cli_cntr->get("reference");
    std::cout << rst2->get("number")->getData() << std::endl;


    cli::Options* rst3=  cli_cntr->get("question");
    std::cout << rst3->get("theq")->getData() << std::endl;

    cli::Options* rst4=  cli_cntr->get("testing");
    std::cout << rst4->get("test")->getData() << std::endl;

}