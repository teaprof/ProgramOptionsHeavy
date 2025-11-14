/*!
hypercube --help
hypercube run --help
hypercube gather --help
hypercube run 
    --help 
    -d 1 -m 1 outputfile
    --bitsRepack 
        --srcLittleEndian
        --dstLittleEndian

helpCommand = command("help,h")
alt1 = opts.addAlternatives(helpCommand)
alt2 = opts.addAlternatives("run")
alt2.addCompatible("dim,d")
alt2.addCompatible("m")
repack = command("repack").
    addCompatibleChild("srcLittleEndian").
    addCompatibleChild("dstLittleEndian")
alt2.addCompatible(repack)



*/
#include <Backend/Option.h>
#include <Backend/Printer.h>
#include <Backend/Matcher.h>



int main(int argc, char* argv[]) {
    /*auto helpOption = std::make_shared<NamedOption>("--help", "-h");
    auto hypercubeOptions = std::make_shared<OneOf>(
        helpOption,
        std::make_shared<NamedOption>("run")->
            addUnlock(std::make_shared<NamedOption>("--dim", "-d"))->
            addUnlock(std::make_shared<NamedOption>("--mIntervalsPerDim", "-m")),
        std::make_shared<NamedOption>("gather")
    );

    Printer printer;
    hypercubeOptions->accept(printer);

    Matcher parser(hypercubeOptions);
    std::cout<<parser.parse({"run", "--dim"})<<std::endl;*/

    return 0;
}