#include "boleta1.h"

//declaracion de funciones


//constructor default
Boleta::Boleta()
{
    fecha = {};
    num_boleta = 0;
    num_tienda = 0;
    cantidad = 0;
    monto = 0.0;
}

//constructor con variables
Boleta::Boleta(tm Fecha, int Numbol, int Numtienda, std::string Tienda, std::string Categoria, std::string Envio,
std::string Id, int Cant, std::string Prod, double Monto)
{
    fecha = Fecha;
    num_boleta = Numbol;
    num_tienda = Numtienda;
    tienda = Tienda;
    categoria = Categoria;
    envio = Envio;
    id = Id;
    cantidad = Cant;
    producto = Prod;
    monto = Monto;
}

//destructor de la clase
Boleta::~Boleta()
{

}




//variables de trabajo

tm Boleta::getfecha() const
{
    return fecha;
}

int Boleta::getnumbol() const
{
    return num_boleta;
}

int Boleta::getnumtda() const
{
    return num_tienda;
}

std::string Boleta::gettda() const
{
    return tienda;
}

std::string Boleta::getcateg() const
{
    return categoria;
}

std::string Boleta::getenv() const
{
    return envio;
}

std::string Boleta::getid() const
{
    return id;
}

int Boleta::getcant() const
{
    return cantidad;
}

std::string Boleta::getprod() const
{
    return producto;
}


double Boleta::getmonto() const
{
    return monto;
}
