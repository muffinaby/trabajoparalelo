#ifndef BD_H
#define BD_H

#include <iostream>
#include <iomanip>
#include <string>
#include <cstdlib>
#include <vector>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <chrono>
#include <fstream>
#include <sqlite3.h>
#include <omp.h>
#include "boleta1.h"
#include "pbasic.h"
#include <map>
#include <stdexcept>
#include <unordered_map>
#include <set>
#include <unordered_set>


#define pd_csv "pd.csv"
#define delimitador ";"


//clase para interactuar con la base de datos
class Db {
public:
    //se conecta o crea la base de datos
    Db();
    //ingresa datos
    Db(const Db& orig);
    //destructor de la clase que cerrara la base de datos
    virtual ~Db();

    void guardar(const std::vector<Boleta>& boletas);
    bool verificar(std::string id);
    bool ejecutar(const std::string& consulta);
    void guardar1( Boleta boleta);
    std::vector<std::string> calcularCanastaBasica(int year);
    std::vector<std::string> calcularCanastaBasica1(int year);
    std::map<std::string, double> calcularTotalMes(int year, const std::vector<std::string>& canastaBasica);
    std::vector<double> variacionmensual(std::map<std::string, double> totalMes);
    std::map<std::string, double> leerPEN_CLP(const std::string &filename);
    std::map<std::string, double> calcularprommes(std::map<std::string, double> conversions, int startYear, int startMonth, int endYear, int endMonth);

private:
    sqlite3 *db;
};






#endif
