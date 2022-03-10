#ifndef EXCEPTION_HPP_
#define EXCEPTION_HPP_

#include <exception>
#include <iostream>

class Exception : public std::exception {
    public:
        Exception(const std::string &msg) : _msg(msg) {}
        ~Exception() = default;

        virtual const char* what() const noexcept override
        {
            return _msg.c_str();
        }

    protected:
    private:
        std::string _msg;
};

#endif /* !EXCEPTION_HPP_ */
