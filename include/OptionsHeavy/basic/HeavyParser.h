#ifndef __DYNAMIC_OPTIONS_CONTAINER_H__
#define __DYNAMIC_OPTIONS_CONTAINER_H__
#include <Backend/Parser.h>
#include <OptionsHeavy/basic/HeavyOption.h>

class HeavyParser : public Parser {
    public:
        //void onNewOccurrenceFinished(std::shared_ptr<>)
    private:
        std::vector<std::shared_ptr<HeavyOptionBase>> options_;
}
#endif