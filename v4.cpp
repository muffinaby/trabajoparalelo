/* Programa principal que llama a las clases para resolver el problema */

#include <iostream>
#include <iomanip>
#include <string>
#include <cstdlib>
#include <vector>
#include <chrono>
#include <omp.h>
#include <fstream>
#include <sstream>
#include "boleta1.h"
#include "pbasic.h"
#include "bd.h"

#define pd_csv "pd.csv"
#define pen_clp "Datos históricos PEN_CLP.csv"
#define delimitador ";"

void participantes(std::string programa);
void secuencial();
void paralelo();
bool process_line(const std::string& linea, std::vector<Boleta>& boletas);

int main(int argc, char** argv) {
    auto inicio = std::chrono::high_resolution_clock::now();
    Db db;

  std::ifstream archivo(pd_csv);
    if (!archivo.is_open()) {
        std::cerr << "No se pudo acceder al archivo " << pd_csv << std::endl;
        return 1;
    }
    std::string lncont;
    int totalln = 56916429;

    std::cout << "son " << totalln << " lineas "<< std::endl;

    archivo.close();


    archivo.open(pd_csv);
    std::vector<Boleta> boletas; // Vector compartido para todas las tareas
    int numthreads = 16;

    int start, end, step;
    start = 0;
    step = (totalln/numthreads);
    end = totalln/step;

    boletas.reserve(step/numthreads);  // Reserva de espacio para eficiencia
    std::string linea;

    for(int k=start; k < end; k++)
{
    db.ejecutar("BEGIN TRANSACTION;");
    std::cout << "estamos en el " << k << " paso de " << end << std::endl;

    #pragma omp parallel for num_threads(numthreads) private(linea)
    for (int i = 0; i < omp_get_num_threads(); ++i) {
        std::vector<Boleta> local_boletas; // Vector local para cada hilo
        local_boletas.reserve(step/numthreads);  // Reserva de espacio para eficiencia
        std::cout << "el procesador " << omp_get_thread_num() << " comenzara a ejecutar" << std::endl;
        // Lectura y procesamiento de líneas en paralelo
        for (int j = 0; j < step/numthreads; ++j) {
            std::string linea;
            #pragma omp critical
            {
                if (!std::getline(archivo, linea)) {
                    linea = ""; // Si no hay más líneas, se inicializa vacía para salir del bucle interno
                }
            }

            if (!linea.empty()) {
                if (process_line(linea, local_boletas)) {
                }
            }
        }

        // Sincronización y fusión de local_boletas en boletas
        #pragma omp critical
        {
            boletas.insert(boletas.end(), local_boletas.begin(), local_boletas.end());
        }
    }
    std::cout << "el procesador " << omp_get_thread_num() << " comenzara a guardar" << std::endl;
    db.guardar(boletas);
    boletas.clear();
    db.ejecutar("COMMIT;");}
    archivo.close();

    std::map<std::string, double> conversions;
    conversions = db.leerPEN_CLP(pen_clp);
    std::map<std::string, double> prommes;
    prommes = db.calcularprommes(conversions, 2021, 2, 2024, 4);

    std::cout << "Promedio de la conversión de un sol peruano a peso chileno por mes:" << std::endl;
    for (const auto& pair : prommes) {
        std::cout << pair.first << ": " << std::fixed << std::setprecision(2) << pair.second << std::endl;
    }


    for (int year = 2021; year <= 2024; ++year) {
        std::cout << "Calculating for year: " << year << std::endl;

        // Comienza la búsqueda de productos para la canasta básica
        std::vector<std::string> canastaBasica;
        canastaBasica.clear();
        try {
            canastaBasica = db.calcularCanastaBasica(year);
        } catch (const std::exception& e) {
            std::cerr << "error al calcular la canasta basica en  " << year << ": " << e.what() << std::endl;
            continue;
        }
        std::cout << "-------------------------------------------" << std::endl;
        std::cout << "son " << canastaBasica.size() << " productos" << std::endl;
        std::cout << "-------------------------------------------" << std::endl;

        // Calcular el monto gastado en el mes en canasta basica
        std::map<std::string, double> totalMes;
        try {
            totalMes = db.calcularTotalMes(year, canastaBasica);
        } catch (const std::exception& e) {
            std::cerr << "error en calcular el monto total del mes en  " << year << ": " << e.what() << std::endl;
            continue;
        }

        // Mostrar total por mes
        std::cout << std::setw(10) << "Month-Year" << std::setw(20) << "Total Amount" << std::endl;
        std::cout << "-------------------------------------------" << std::endl;
        for (const auto& pair : totalMes) {
            std::cout << std::setw(10) << pair.first << std::setw(20) << std::fixed << std::setprecision(2) << pair.second << std::endl;
        }
        std::cout << std::endl;

        // Calcular variacion intermensual
        std::vector<double> variacion;
        try {
            variacion = db.variacionmensual(totalMes);
        } catch (const std::exception& e) {
            std::cerr << "Error calculating total amount per month for year " << year << ": " << e.what() << std::endl;
            continue;
        }

        // Mostrar cálculos
        std::cout << "Month-Year       Variation" << std::endl;
        std::cout << "---------------------------" << std::endl;

        int i = 1; // se comienza del segundo mes debido a que el primer mes no tiene un mes pasado con el que comparar
        for (const auto& pair : totalMes) {
            if (i < variacion.size() + 1) {
                std::cout << std::setw(10) << pair.first << std::setw(20) << std::fixed << std::setprecision(2) << variacion[i - 1] << "%" << std::endl;
            }
            ++i;
        }

        // Convertir los montos de la canasta básica de soles peruanos a pesos chilenos
        for (auto& pair : totalMes) {
            std::string key = pair.first.substr(0, 7); // Obtener "YYYY-MM" de la fecha
            if (prommes.find(key) != prommes.end()) {
                pair.second *= prommes[key];
            } else {
                std::cerr << "No se encontró promedio para el mes: " << key << std::endl;
            }
        }

        // Imprimir la canasta básica en pesos chilenos (opcional)
        std::cout << "\nTotal Mes en Pesos Chilenos:" << std::endl;
        for (const auto& pair : totalMes) {
            std::cout << pair.first << ": " << std::fixed << std::setprecision(2) << pair.second << " CLP" << std::endl;
        }

        std::vector<double> variacioncl;
        try {
            variacioncl = db.variacionmensual(totalMes);
        } catch (const std::exception& e) {
            std::cerr << "Error calculating total amount per month for year " << year << ": " << e.what() << std::endl;
            continue;
        }

        // Mostrar cálculos
        std::cout << "Month-Year       Variation" << std::endl;
        std::cout << "---------------------------" << std::endl;

        int j = 1; // se comienza del segundo mes debido a que el primer mes no tiene un mes pasado con el que comparar
        for (const auto& pair : totalMes) {
            if (j < variacioncl.size() + 1) {
                std::cout << std::setw(10) << pair.first << std::setw(20) << std::fixed << std::setprecision(2) << variacioncl[j - 1] << "%" << std::endl;
            }
            ++j;
        }

        std::cout << std::endl;
    }

    auto fin = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duracion = fin - inicio;
    std::cout << "Tiempo de ejecución: " << duracion.count() << " segundos" << std::endl;

    if (argc > 1) {
        // Manejo de argumentos si es necesario
    } else {
        participantes(argv[0]);
    }

    return EXIT_SUCCESS;
}

bool process_line(const std::string& linea, std::vector<Boleta>& boletas) {
    std::stringstream ss(linea);
    std::vector<std::string> datos;
    std::string dato;
    while (std::getline(ss, dato, ';')) {
        datos.push_back(dato);
    }

    if (datos.size() < 10 || datos.size() > 12) {
        return false; // Se omiten líneas con número incorrecto de campos
    }

    try {
        std::tm fecha = Pbasic::satm(datos[0]);
        int numbol = std::stoi(Pbasic::sacarcomillas(datos[1]));
        int numtienda = std::stoi(Pbasic::sacarcomillas(datos[2]));
        std::string id = Pbasic::sacarcomillas(datos[6]);
        int cantidad = std::stoi(Pbasic::sacarcomillas(datos[7]));
        double monto = Pbasic::vermonto(Pbasic::sacarcomillas(datos[9])) ?
                       std::stod(Pbasic::sacarcomillas(datos[9])) :
                       std::stod(Pbasic::sacarcomillas(datos[10]));


        if (!id.empty() && id != "0") {
            Boleta boleta(fecha, numbol, numtienda, "", "", "", id, cantidad, "", monto);
            boletas.push_back(boleta);
        }
    } catch (const std::exception& e) {
        std::cerr << "Excepción al procesar la línea: " << linea << "\nError: " << e.what() << std::endl;
        return false;
    }

    return true;
}

void participantes(std::string programa){
    std::cout << std::endl << "=== Taller " << programa << " ===" << std::endl;
    std::cout << std::endl << "Sebastian Gacitua Caro";
    std::cout << std::endl << "Pablo Martinez Canto" << std::endl;
}
