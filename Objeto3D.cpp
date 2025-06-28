#include "Objeto3D.h"
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
    ativo = false;
    rotacao = 0.0f;
    modeloCarregado = false;
}

void Objeto3D::inicializar(float x, float z, float rot) {
    posicao = Ponto(x, 3, z);
    rotacao = rot;
    ativo = true;
}

void Objeto3D::desenhar() {
    if (!ativo) {
        std::cout << "Objeto não está ativo!" << std::endl;
        return;
    }

    glPushMatrix();
    glTranslatef(posicao.x, posicao.y, posicao.z);
    glRotatef(rotacao, 0, 1, 0);
    
    if (modeloCarregado && !vertices.empty()) {
        // Desenhar modelo 3D carregado
        std::cout << "Desenhando modelo 3D com " << vertices.size() << " vértices e " << indices.size() << " índices" << std::endl;
        
        // Define uma cor para o modelo
        glColor3f(0.8f, 0.6f, 0.4f); // Cor marrom para a vaca
        
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
        std::cout << "Usando fallback: desenhando vaca simples" << std::endl;
        // Desenhar vaca simples como fallback
        glColor3f(0.0f, 0.0f, 0.0f); // Cor preta
        
        // Corpo da vaca
        glPushMatrix();
        glScalef(1.0f, 0.6f, 1.5f);
        glutSolidCube(1.0f);
        glPopMatrix();
        
        // Cabeça
        glPushMatrix();
        glTranslatef(0.8f, 0.2f, 0);
        glScalef(0.6f, 0.4f, 0.4f);
        glutSolidCube(1.0f);
        glPopMatrix();
        
        // Pernas
        glPushMatrix();
        glTranslatef(-0.3f, -0.8f, 0.3f);
        glScalef(0.1f, 0.8f, 0.1f);
        glutSolidCube(1.0f);
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(-0.3f, -0.8f, -0.3f);
        glScalef(0.1f, 0.8f, 0.1f);
        glutSolidCube(1.0f);
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(0.3f, -0.8f, 0.3f);
        glScalef(0.1f, 0.8f, 0.1f);
        glutSolidCube(1.0f);
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(0.3f, -0.8f, -0.3f);
        glScalef(0.1f, 0.8f, 0.1f);
        glutSolidCube(1.0f);
        glPopMatrix();
        
        // Restaura a cor
        glColor3f(1.0f, 1.0f, 1.0f);
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
    
    // Debug: mostra alguns vértices para verificar se estão sendo lidos corretamente
    if (vertices.size() >= 3) {
        std::cout << "Primeiro vértice: (" << vertices[0].x << ", " << vertices[0].y << ", " << vertices[0].z << ")" << std::endl;
        std::cout << "Segundo vértice: (" << vertices[1].x << ", " << vertices[1].y << ", " << vertices[1].z << ")" << std::endl;
        std::cout << "Terceiro vértice: (" << vertices[2].x << ", " << vertices[2].y << ", " << vertices[2].z << ")" << std::endl;
    }
    
    return true;
}

void Objeto3D::resetar(float x, float z, float rot) {
    inicializar(x, z, rot);
} 