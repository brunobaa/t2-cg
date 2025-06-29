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
    cor = Green; // cor padrão em destaque
}

void Objeto3D::renderizar() {
    glPushMatrix();
        glScalef(escala, escala, escala); // Escala em cubo
        glRotatef(rotacao, 0, 1, 0);
        defineCor(cor);
        
        if (!vertices.empty()) {
            glBegin(GL_TRIANGLES);
                for (size_t i = 0; i < indices.size(); i += 3) {
                    if (i + 2 < indices.size()) {
                        Ponto& p1 = vertices[indices[i]];
                        Ponto& p2 = vertices[indices[i + 1]];
                        Ponto& p3 = vertices[indices[i + 2]];
                        
                        Ponto borda1 = p2 - p1;
                        Ponto borda2 = p3 - p1;
                        Ponto normal;
                        ProdVetorial(borda1, borda2, normal);
                        normal.versor(); // normaliza
                        
                        glNormal3f(normal.x, normal.y, normal.z);
                        glVertex3f(p1.x, p1.y, p1.z);
                        glVertex3f(p2.x, p2.y, p2.z);
                        glVertex3f(p3.x, p3.y, p3.z);
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

bool Objeto3D::lerObjetoTRI(const char* arquivo) {
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
    return true;
}