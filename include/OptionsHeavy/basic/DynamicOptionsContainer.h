#ifndef __DYNAMIC_OPTIONS_CONTAINER_H__
#define __DYNAMIC_OPTIONS_CONTAINER_H__
#include <OptionsHeavy/basic/DynamicOption.h>

class DynamicOptionsContainer {
    public:
        //void onNewOccurrenceFinished(std::shared_ptr<>)
    private:
    std::vector<std::shared_ptr<DynamicOption>> options_;
}
#endif