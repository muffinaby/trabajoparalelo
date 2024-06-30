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

    db.ejecutar("BEGIN TRANSACTION;");

    std::vector<Boleta> boletas; // Vector compartido para todas las tareas
    boletas.reserve(1000);  // Reserva de espacio para eficiencia

    std::string linea;
    #pragma omp parallel for num_threads(12) private(linea)
    for (int i = 0; i < omp_get_num_threads(); ++i) {
        std::vector<Boleta> local_boletas; // Vector local para cada hilo
        local_boletas.reserve(1000);  // Reserva de espacio para eficiencia

        // Lectura y procesamiento de líneas en paralelo
        for (int j = 0; j < 1000; ++j) {
            std::string linea;
            #pragma omp critical
            {
                if (!std::getline(archivo, linea)) {
                    linea = ""; // Si no hay más líneas, se inicializa vacía para salir del bucle interno
                }
            }

            if (!linea.empty()) {
                if (process_line(linea, local_boletas)) {
                    // Opcionalmente, se puede verificar el tamaño de local_boletas para guardar en lotes
                }
            }
        }

        // Sincronización y fusión de local_boletas en boletas
        #pragma omp critical
        {
            boletas.insert(boletas.end(), local_boletas.begin(), local_boletas.end());
        }
    }

    db.guardar(boletas);
    db.ejecutar("COMMIT;");
    archivo.close();

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
        std::string tienda = Pbasic::sacarcomillasimple(Pbasic::sacarcomillas(datos[3]));
        std::string categoria = Pbasic::sacarcomillasimple(Pbasic::sacarcomillas(datos[4]));
        std::string envio = datos[5].empty() ? "No se especifica" : Pbasic::sacarcomillas(datos[5]);
        std::string id = Pbasic::sacarcomillas(datos[6]);
        int cantidad = std::stoi(Pbasic::sacarcomillas(datos[7]));
        std::string producto = Pbasic::sacarcomillasimple(Pbasic::sacarcomillas(datos[8]));
        double monto = Pbasic::vermonto(Pbasic::sacarcomillas(datos[9])) ?
                       std::stod(Pbasic::sacarcomillas(datos[9])) :
                       std::stod(Pbasic::sacarcomillas(datos[10]));

        if (!id.empty() && id != "0") {
            Boleta boleta(fecha, numbol, numtienda, tienda, categoria, envio, id, cantidad, producto, monto);
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
