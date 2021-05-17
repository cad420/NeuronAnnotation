#include <iostream>
#include <ErrorMessage.hpp>

int main(){
    std::string s = "aaa";
    ErrorMessage em("???");
    std::cout << em.ToJson() << std::endl;
    return 0;
}