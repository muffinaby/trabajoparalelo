#include "bd.h"

//se conecta o crea la base de datos
Db::Db() {
    int estado = sqlite3_open("boletas.db", &db);
    if (estado == SQLITE_OK) {
        ejecutar(R"(
            CREATE TABLE IF NOT EXISTS Transactions (
                TransactionID INTEGER PRIMARY KEY AUTOINCREMENT,
                CreationDate TEXT NOT NULL,
                ReceiptNumber INTEGER NOT NULL,
                Quantity INTEGER NOT NULL,
                Amount REAL NOT NULL,
                ProductID TEXT NOT NULL
            );
        )");
    } else {
        std::cerr << "No fue posible abrir la base de datos: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        db = nullptr;
    }
}
Db::Db(const Db& orig) {
    this->db = orig.db;
}

//destructor de la clase que cerrara la base de datos
Db::~Db() {
    if (db) {
        sqlite3_close(db);
    }
}

bool Db::ejecutar(const std::string& consulta) {
    char* error = nullptr;
    int estado = sqlite3_exec(db, consulta.c_str(), nullptr, nullptr, &error);
    if (estado != SQLITE_OK) {
        std::cerr << "Error al ejecutar la consulta: " << error << std::endl;
        sqlite3_free(error);
        return false;
    }
    return true;
}



void Db::guardar(const std::vector<Boleta>& boletas) {
    if (boletas.empty()) {
        return;
    }

    std::string sql;
    for (const auto& boleta : boletas) {
        sql += "INSERT INTO Transactions (CreationDate, ReceiptNumber, Quantity, Amount, ProductID) VALUES ('" +
               Pbasic::tmas(boleta.getfecha()) + "', '" +
               std::to_string(boleta.getnumbol()) + "', '" +
               std::to_string(boleta.getcant()) + "', '" +
               std::to_string(boleta.getmonto()) + "', '" +
               boleta.getid() + "');";
    }

    std::cerr << "Guardando " << boletas.size() << " boletas en la base de datos" << std::endl;

    ejecutar(sql);
}

void Db::guardar1( Boleta boleta)
{
    bool ver1;
    std::string inserttrans = "INSERT INTO Transactions (CreationDate, ReceiptNumber,Quantity, Amount, ProductID) VALUES ('" +
    Pbasic::tmas(boleta.getfecha()) +
    "', '" +
    std::to_string(boleta.getnumbol()) +
    "', '" +
    std::to_string(boleta.getcant()) +
    "', '" +
    std::to_string(boleta.getmonto())+
    "', '" +
    boleta.getid() + "');";
    ejecutar(inserttrans);
}
std::map<std::string, double> Db::calcularTotalMes(int year, const std::vector<std::string>& canastaBasica) {
    std::map<std::string, double> totalMes;
    std::set<std::string> canastaSet(canastaBasica.begin(), canastaBasica.end());

    std::ostringstream query;
    query << "SELECT strftime('%Y-%m', CreationDate), ProductID, SUM(Amount) FROM Transactions WHERE strftime('%Y', CreationDate) = '"
          << year << "' GROUP BY strftime('%Y-%m', CreationDate), ProductID";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query.str().c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Error al preparar la consulta: " + query.str());
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string monthYear = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string productId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        double amount = sqlite3_column_double(stmt, 2);

        if (canastaSet.find(productId) != canastaSet.end()) {
            totalMes[monthYear] += amount;
        }
    }
    sqlite3_finalize(stmt);

    return totalMes;
}

std::vector<std::string> Db::calcularCanastaBasica1(int year) {
  std::vector<std::string> canastaBasica;
      canastaBasica.clear();

      int requiredMonths = (year == 2024) ? 4 : 12;

      std::ostringstream query;
      query << "SELECT ProductID "
            << "FROM ( "
            << "    SELECT ProductID, strftime('%Y-%m', CreationDate) AS YearMonth, COUNT(*) AS Frequency "
            << "    FROM Transactions "
            << "    WHERE strftime('%Y', CreationDate) = '" << year << "' "
            << "    GROUP BY ProductID, YearMonth) AS MonthlyFrequency "
            << "GROUP BY ProductID "
            << "HAVING COUNT(DISTINCT YearMonth) = " << requiredMonths << ";";


      sqlite3_stmt* stmt;
      if (sqlite3_prepare_v2(db, query.str().c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
          throw std::runtime_error("Error al preparar la consulta: " + std::string(sqlite3_errmsg(db)));
      }

      while (sqlite3_step(stmt) == SQLITE_ROW) {
          std::string productId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
          canastaBasica.push_back(productId);
      }
      sqlite3_finalize(stmt);

      return canastaBasica;
}
std::vector<std::string> Db::calcularCanastaBasica(int year) {
  std::vector<std::string> canastaBasica;
   canastaBasica.clear();

   int requiredMonths;
   if (year == 2021) {
       requiredMonths = 11;
   } else if (year == 2024) {
       requiredMonths = 4;
   } else {
       requiredMonths = 12;
   }


   std::ostringstream query;
   query << "SELECT ProductID "
         << "FROM ( "
         << "    SELECT ProductID, strftime('%Y-%m', CreationDate) AS YearMonth, COUNT(*) AS Frequency "
         << "    FROM Transactions "
         << "    WHERE strftime('%Y', CreationDate) = '" << year << "' "
         << "    GROUP BY ProductID, YearMonth) AS MonthlyFrequency "
         << "GROUP BY ProductID "
         << "HAVING COUNT(DISTINCT YearMonth) = " << requiredMonths << ";";


   sqlite3_stmt* stmt;
   if (sqlite3_prepare_v2(db, query.str().c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
       throw std::runtime_error("Error al preparar la consulta: " + std::string(sqlite3_errmsg(db)));
   }


   std::vector<std::string> localCanastaBasica;


   while (sqlite3_step(stmt) == SQLITE_ROW) {
       std::string productId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
       localCanastaBasica.push_back(productId);
   }
   sqlite3_finalize(stmt);


   #pragma omp parallel
   {
       std::vector<std::string> privateCanastaBasica;

       #pragma omp for nowait
       for (size_t i = 0; i < localCanastaBasica.size(); ++i) {
           privateCanastaBasica.push_back(localCanastaBasica[i]);
       }

       #pragma omp critical
       {
           canastaBasica.insert(canastaBasica.end(), privateCanastaBasica.begin(), privateCanastaBasica.end());
       }
   }

   return canastaBasica;
}

std::vector<double> Db::variacionmensual(std::map<std::string, double> totalMes)
{
  std::vector<double> variacion;
  variacion.clear();
  bool firstMonth = true;
  double previousMonthTotal = 0.0;
  variacion.push_back(previousMonthTotal);

  for (const auto& pair : totalMes) {
      if (!firstMonth) {
          double variation = (pair.second / previousMonthTotal - 1)*100;
          variacion.push_back(variation);
      } else {
          firstMonth = false;
      }
      previousMonthTotal = pair.second;
  }
  return variacion;
}

std::map<std::string, double> Db::calcularprommes(std::map<std::string, double> conversions, int startYear, int startMonth, int endYear, int endMonth) {
    std::map<std::string, std::pair<double, int>> monthTotals;
    std::map<std::string, double> averagePerMonth; // Mapa para almacenar el promedio mensual
    for (const auto& pair : conversions) {
      //bucle que calcula el promedio de conversion de soles peruanos a pesos chilenos
      int year = std::stoi(pair.first.substr(0, 4));
      int month = std::stoi(pair.first.substr(5, 2));
      if ((year > startYear || (year == startYear && month >= startMonth)) &&
          (year < endYear || (year == endYear && month <= endMonth))) {

          std::string key = pair.first;
          double value = pair.second;

          if (monthTotals.find(key) == monthTotals.end()) {
              monthTotals[key] = {0.0, 0};
          }
          monthTotals[key].first += value;
          monthTotals[key].second++;
      }
    }

    for (const auto& pair : monthTotals) {

      double average = pair.second.first / pair.second.second;
      averagePerMonth[pair.first] = average;
    }

    return averagePerMonth;
}

std::map<std::string, double> Db::leerPEN_CLP(const std::string &filename) {
    std::cout << "se leera el archivo" << std::endl;
    std::map<std::string, double> conversions;
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Error opening file: " + filename);
    }
    std::string line;
    for(int i = 0; i<8; i++)
    {
      std::getline(file,line);
    }

    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string dateStr, valueStr;
        std::getline(ss, dateStr, ',');
        std::getline(ss, valueStr, ',');
        valueStr = Pbasic::sacarcomillas(valueStr);

        int day, month, year;
        char delimiter;
        std::istringstream dateStream(dateStr);
        dateStream >> day >> delimiter >> month >> delimiter >> year;


        valueStr.erase(0, 4);
        std::replace(valueStr.begin(), valueStr.end(), ',', '.');
        double value = std::stod(Pbasic::sacarcomillas(valueStr));


        std::string key = std::to_string(year) + "-" + (month < 10 ? "0" : "") + std::to_string(month);

        if (conversions.find(key) == conversions.end()) {
            conversions[key] = value;
        } else {
            conversions[key] = value;
        }
    }
    file.close();
    return conversions;
}
