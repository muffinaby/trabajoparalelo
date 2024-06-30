//funcion utilizada para el procesamiento basico

#ifndef PBASIC_H
#define PBASIC_H

#include <iostream>
#include <iomanip>
#include <string>
#include <cstdlib>
#include <vector>
#include <sstream>
#include <algorithm> // Para transform
#include <ctime>
#include <chrono>
#include <fstream>

namespace Pbasic
{
    //convertir string en tm
    std::tm satm(std::string);

    //convertir tm en fc
    std::string tmas(std::tm);

    bool vermonto(const std::string& s);

    //sacar comillas
    std::string sacarcomillas(const std::string& str);
    std::string sacarcomillasimple(const std::string& input);

} // namespace Pbasic


#endif
