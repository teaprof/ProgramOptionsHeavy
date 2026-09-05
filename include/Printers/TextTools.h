#include<vector>
#include<string>
#include<ranges>
#include<string_view>

/*std::vector<std::string> wordWrap(const std::string& str, size_t maxWidth, size_t indentFirst = 2) {
    std::vector<std::string> res;
    if(str.empty())
        return res;
    std::string current_line(indentFirst, ' ');    
    for(const auto word : std::views::split(str, ' ')) {
        if(current_line.size() + word.size() > maxWidth) {
            if(word.size() > maxWidth) {

            }

        }
    }
    return res;
}*/