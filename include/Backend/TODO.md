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

Heavy manual usage:
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
    - multiple_values vs multiple_occurrences
    - implicit value (requires reconsidering of the parser and lexer)
    - what is allow_long_disguise (using single "-" for long options)?
    - like TCLAP: hideFromHelp(), visibleFromHelp()

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
    + check duplicates among long and short names


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

