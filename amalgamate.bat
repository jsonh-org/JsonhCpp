:: Prevent quit on error
if not defined in_subprocess (cmd /k set in_subprocess=y ^& %0 %*) & exit

:: Amalgamate with https://github.com/rindeal/Amalgamate
set amalgamate_path="C:/Users/patri/Documents/Programs/amalgamate-v0.99.0-windows-amd64/amalgamate.exe"
%amalgamate_path% "jsonh_cpp/jsonh_cpp.hpp" "jsonh_cpp_amalgamated.hpp"