#ifndef OBJETO3D_H
#define OBJETO3D_H

#include "Ponto.h"
#include <vector>

class Objeto3D {
public:
    Ponto posicao;
    float rotacao;
    bool ativo;
    
    // Dados do modelo 3D
    std::vector<Ponto> vertices;
    std::vector<int> indices;
    bool modeloCarregado;
    
    Objeto3D();
    void inicializar(float x, float z, float rot);
    void desenhar();
    bool carregarTRI(const char* arquivo);
    void resetar(float x, float z, float rot);
};

#endif 