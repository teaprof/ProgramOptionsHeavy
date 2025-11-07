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
    auto helpOption = std::make_shared<AbstractNamedOption>("--help", "-h");
    auto hypercubeOptions = std::make_shared<Alternatives>(
        helpOption,
        std::make_shared<AbstractNamedOption>("run")->
            addUnlock(std::make_shared<AbstractNamedOption>("--dim", "-d"))->
            addUnlock(std::make_shared<AbstractNamedOption>("--mIntervalsPerDim", "-m")),
        std::make_shared<AbstractNamedOption>("gather")
    );

    Printer printer;
    hypercubeOptions->accept(printer);

    Parser parser(hypercubeOptions);
    std::cout<<parser.parse({"run", "--dim"})<<std::endl;

    return 0;
}