#include "pbasic.h"

//convertir string en tm
std::tm Pbasic::satm(std::string fechastr)
{

    std::tm fecha = {};

    fechastr = sacarcomillas(fechastr);
    std::stringstream ss(fechastr);
    ss >>  std::get_time(&fecha, "%Y-%m-%d %H:%M:%S");
    return fecha;
}

//verificar monto
bool Pbasic::vermonto(const std::string& s) {
    char* end = nullptr;
    std::strtod(s.c_str(), &end);
    return end != s.c_str() && *end == '\0';
}

//sacar comillas
std::string Pbasic::sacarcomillas(const std::string& str)
{
    if (str.length() >= 2 && str[0] == '"' && str[str.length() - 1] == '"') {
        return str.substr(1, str.length() - 2);
    }
    return str;
}

//convertir tm en fc
std::string Pbasic::tmas(std::tm tm)
{
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string Pbasic::sacarcomillasimple(const std::string& input) {
    std::string resultado;
    for (char c : input) {
        if (c != '\'') {
            resultado += c;
        }
    }
    return resultado;
}
