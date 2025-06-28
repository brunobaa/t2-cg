#ifndef OBJETO3D_H
#define OBJETO3D_H

#include "Ponto.h"
#include <vector>

class Objeto3D {
public:
    float rotacao;
    float escala;
    int cor;

    // MOdelagem 3D
    std::vector<Ponto> vertices;
    std::vector<int> indices;
    bool modeloCarregado;
    
    Objeto3D();
    void desenhar();
    bool carregarTRI(const char* arquivo);
};

#endif 