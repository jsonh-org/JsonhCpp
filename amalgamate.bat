:: Prevent quit on error
if not defined in_subprocess (cmd /k set in_subprocess=y ^& %0 %*) & exit

:: Amalgamate with Quom (https://github.com/Viatorus/quom)
quom "jsonh_cpp/jsonh_cpp.hpp" "jsonh_cpp_amalgamated.hpp"