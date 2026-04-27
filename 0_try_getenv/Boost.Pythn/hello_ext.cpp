//
// Created by cj on 2022-06-21.
//
#include <boost/python.hpp>

BOOST_PYTHON_MODULE(hello_ext)
        {
                using namespace boost::python;
                def("greet", greet);
        }