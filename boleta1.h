//header de la clase boleta

#ifndef BOLETA1_H
#define BOLETA1_H




#include <iostream>
#include <iomanip>
#include <string>
#include <cstdlib>
#include <vector>

class Boleta{
    public:
    //constructor default
    Boleta();

    //constructor de variables
    Boleta(tm , int, int, std::string, std::string, std::string, std::string, int ,std::string, double);

    //destructor de clase boleta
    ~Boleta();


    //acceso a los datos
    tm getfecha() const;

    int getnumbol() const;

    int getnumtda() const;

    std::string gettda() const;

    std::string getcateg() const;

    std::string getenv() const;

    std::string getid() const;

    int getcant() const;

    std::string getprod() const;

    double getmonto() const;

    private:
    //los miembros de la boleta seran guardados en las siguientes variables
    tm fecha;
    int num_boleta;
    int num_tienda;
    std::string tienda;
    std::string categoria;
    std::string envio;
    std::string id;
    int cantidad;
    std::string producto;
    double monto;

};


#endif
