#include "Objeto3D.h"
#include "ListaDeCoresRGB.h"
#include <fstream>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <sstream>

#ifdef __linux__
#include <GL/glut.h>
#endif

Objeto3D::Objeto3D() {
    rotacao = 0.0f;
    modeloCarregado = false;
    cor = Green;
}

void Objeto3D::desenhar() {
    glPushMatrix();
        glScalef(escala, escala, escala); // Escala em cubo
        glRotatef(rotacao, 0, 1, 0);
        defineCor(cor);
        
        if (modeloCarregado && !vertices.empty()) {
            glBegin(GL_TRIANGLES);
                for (size_t i = 0; i < indices.size(); i += 3) {
                    if (i + 2 < indices.size()) {
                        Ponto& v1 = vertices[indices[i]];
                        Ponto& v2 = vertices[indices[i + 1]];
                        Ponto& v3 = vertices[indices[i + 2]];
                        
                        // Calcular normal para iluminação usando ProdVetorial
                        Ponto edge1 = v2 - v1;
                        Ponto edge2 = v3 - v1;
                        Ponto normal;
                        ProdVetorial(edge1, edge2, normal);
                        normal.versor(); // Normalizar o vetor
                        
                        glNormal3f(normal.x, normal.y, normal.z);
                        glVertex3f(v1.x, v1.y, v1.z);
                        glVertex3f(v2.x, v2.y, v2.z);
                        glVertex3f(v3.x, v3.y, v3.z);
                    }
                }
            glEnd();
            
            // Restaura a cor
            glColor3f(1.0f, 1.0f, 1.0f);
        } else {
            std::cout << "Ocorreu um erro ao renderizar objeto 3D" << std::endl;
           
        }
    glPopMatrix();
}

bool Objeto3D::carregarTRI(const char* arquivo) {
    std::ifstream file(arquivo);
    if (!file.is_open()) {
        std::cout << "Erro ao abrir o arquivo modelo: " << arquivo << std::endl;
        return false;
    }
    
    vertices.clear();
    indices.clear();
    
    std::string linha;
    int indiceAtual = 0;
    int triangulosLidos = 0;
    
    while (std::getline(file, linha)) {
        if (linha.empty()) continue;
        
        if (linha[0] == '#') continue;
        
        std::istringstream iss(linha);
        float x1, y1, z1, x2, y2, z2, x3, y3, z3;
        std::string cor;
        
        if (iss >> x1 >> y1 >> z1 >> x2 >> y2 >> z2 >> x3 >> y3 >> z3 >> cor) {
            vertices.push_back(Ponto(x1, y1, z1));
            vertices.push_back(Ponto(x2, y2, z2));
            vertices.push_back(Ponto(x3, y3, z3));
            
            indices.push_back(indiceAtual);
            indices.push_back(indiceAtual + 1);
            indices.push_back(indiceAtual + 2);
            
            indiceAtual += 3;
            triangulosLidos++;
        }
    }
    
    file.close();
    
    if (vertices.empty()) {
        std::cout << "Erro: não foram encontrados triangulos no arquivo " << arquivo << std::endl;
        return false;
    }
    
    modeloCarregado = true;
    std::cout << "Modelo carregado com sucesso! " << vertices.size() << " vértices, " 
              << indices.size() << " índices, " << triangulosLidos << " triângulos" << std::endl;
    return true;
}