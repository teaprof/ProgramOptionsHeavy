ALTERNATIVES:
    1. The alternative should have unique options
    2. If such option is encountered the alternative is selected. Options for other altenatives become unavailable.
    3. Check should check names collisions to ensure that p. 1 holds
    4. Check should check cross product if two or more alternative options are used

CROSS PRODUCT:

def processAlternatives(self, OneOf):
    for it in unlocks:
        check and add it
    push
    for options_variant in OneOf.alternatives:
        for it in options_variant:
            check and add it
            yield this
            restoreTop
    pop

Use cases

Commands (or Alternatives):
git push ...
git commit ...

OneOf:
hypercube run --nPoints=100000
hypercube run --nPointsPerCell=10

Easy-start usage:
```
int opt1_val;
auto opt1 = createOpt("--opt1", "-o", "short description", optRequired, opt1_val);
int opt2_val;
auto opt2 = createOpt("--opt1", "-o", "short description", multipleValues, value<int>(opt2_val)->setDefaultValue(10));

optionsGroup = createOptions().add(opt1).add(opt2);
```

Mid usage:
```
int opt1_val;
auto opt1 = createOpt("--opt1", "-o", optRequired | multipleValues | multipleOccurences)
opt1<<"This is a very long description of the option";
opt1<<"And one more line for the description";
opt1.setRequired(true);
setType(opt1, opt1_val).setRange(-10, 10);
```

Heavy usage with more granular control:
```
int opt1val;
std::string strval;


ValueStorage v;
auto opt1 = std::make_shared<AbstractNamedOption<int>>("--opt1", "-o", optRequired | multipleValues | multipleOccurrences)
opt1->validator()->setMinMax(-10, 10);
opt1<<"This is a very long description of the option";
opt1->tie(v, opt1val);

auto opt2 = std::make_shared<AbstractPositionalOption<std::string>>(optRequired | multipleValues)
opt1->validator()->setRegex("*.txt")
opt1<<"This is a very long description of the option";
opt1->tie(v, opt2val);
```

0. Value semantic class \<Type\>:
    + is used to check if the actual value can be accepted

0. Option \<Type\> :
    + long and short names
    + with value or not
    + \<TYPE\> of the value
    + default value
    + is required
    + multiple_values vs multiple_occurrences
    - multiple_occurences: '--' stop symbol or custom stop symbol, multiple values are not allowed with "=": "--opt=1 2 3" is treated as "--opt=1", but "2" and "3" are the next positional arguments
    - add support of comma: "--opt=1, 2, 3"
    - add support of brackets: "--opt=[1, 2, 3]"
    - implicit value (requires reconsidering of the parser and lexer)
    - what is allow_long_disguise (using single "-" for long options)?
    - like TCLAP: hideFromHelp(), visibleFromHelp()
    - revisit Exceptions
    + double dash can be used to indicate the end of the values list: "--input file1.txt file2.txt -- outputfile.txt"
    + all options after the double dash are treated as positional
    - check https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap12.html


Solve equation:
    n1 + n2 + ... + nk = actual_number_of_args
Solution:
    First, find min(n1+n2+..+nk) and max(n1+n2+..+nk). 
    If not min <= act_number_of_args <= max, than no solution exists
    If act_number_of_args == min or == max than solution is found (it corresponds to arg_min or arg_max).
    Otherwise, solution exists if only one i exists such that min n_i < max_ni.         

1. Parser:
    - unknown options could be collected instead of throwing UnknownOption (like program_options::basic_option::unregistered)
    - support of implicit value (only if no positional options are specified)

1. Value storage
    + value regex
    - expected value type and range (or validator, generally)
How it could be done? What is the best way? I don't want to associate values with options

Value should store the following flags:
    - std::vector\<str\>
    - std::vector\<std::variant\<...\>\> value
    - is_default
    - is_implicit
    - shared_ptr to option

2. Examples of different way to initialize

3. Checker:
    + check duplicates among long and short names (check names collision);
    - run all branches for all BaseValueSemantics::getUnlocks(value) and AbstractOptions::unlocks Alternatives ;
    - check duplicates using pointer comparison;
    - check positional arguments: the system of equation should be solvable
    - check cycles



2. Help constructor
3. Help printers (to plain text, to html, to yaml or json)
4. Default -h,--help,--version, --init-auto-complete behavior
4. Add support of value constraints in the help message
4. Different formats of help

4. Default Options: Help, Quiet, Version, FileName, DirName

4. Support for hex values

4. Autocompleter support

5. Different option prefixes ("-" and "--", "/", add support for "/?", "+")

6. Cartesian product for object with multiplicity

4. Support for "--" arg: Additionally, anything after -- will be parsed as a positional argument.

4. Support for file names and dirs in autocompletion

4. Python binding based on POHeavy

TCLAP: multiSwitchArg:
program -q // quiet
program -qq // more quiet

