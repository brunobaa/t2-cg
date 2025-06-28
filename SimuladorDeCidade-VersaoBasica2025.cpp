// **********************************************************************
// PUCRS/Escola Politecnica
// COMPUTACAO GRAFICA
//
// Programa basico para criar aplicacoes 3D em OpenGL
//
// Marcio Sarroglia Pinho
// pinho@pucrs.br
// **********************************************************************

#include <cstdlib>
#include <iostream>
#include <cmath>
#include <ctime>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

using namespace std;


#ifdef WIN32
#include <windows.h>
#include <glut.h>
#else
#include <sys/time.h>
#endif

#ifdef APPLE
#include <GLUT/glut.h>
#endif

#ifdef linux
#include <GL/glut.h>
#endif

#include "Temporizador.h"
Temporizador T;
double AccumDeltaT=0;

#include "Ponto.h"
#include "ListaDeCoresRGB.h"
#include "Texture.h"
#include "Objeto3D.h"

GLfloat AnguloDeVisao=90;
GLfloat AspectRatio, angulo=0;
GLfloat AlturaViewportDeMensagens = 0.35; // percentual em relacao altura da tela


// Controle do modo de projecao
// 0: Projecao Paralela Ortografica; 1: Projecao Perspectiva
// A funcao "PosicUser" utiliza esta variavel. O valor dela eh alterado
// pela tecla 'p'

int ModoDeProjecao = 1;

// Controle do modo de projecao
// 0: Wireframe; 1: Faces preenchidas
// A funcao "Init" utiliza esta variavel. O valor dela eh alterado
// pela tecla 'w'
int ModoDeExibicao = 1;
int anguloCarro = 0;

double nFrames=0;
double TempoTotal=0;

// Qtd de ladrilhos do piso. Inicialzada na INIT
int QtdX;
int QtdZ;
int direcao = -1; //se esta para frente ou para tras/ para direita ou esquerda
bool percorrer = false;


// Representa o conteudo de uma celula do piso
class Elemento{
public:
    int tipo;
    int corDoObjeto;
    int corDoPiso;
    float altura;
    int textureID;
};

// codigos que definem o o tipo do elemento que esta em uma celula
#define VAZIO 0
#define PREDIO 10
#define RUA 20
#define COMBUSTIVEL 30
#define VEICULO 40
#define CALCADA 50

// Matriz que armazena informacoes sobre o que existe na cidade
Elemento Cidade[30][30];
int** textureMap = nullptr; // Matriz dinâmica para textureMap

Ponto Observador, Alvo, TerceiraPessoa, PosicaoVeiculo;
Ponto aux = Ponto(-15,2,18.7);
Ponto posicaoCarro = Ponto(2.5,0,2.5);

bool ComTextura = true;

// Novas variáveis para o sistema de controle
double velocidade = 3.0; // 3 m/s
double combustivel = 100.0; // 100% inicial
double consumoCombustivel = 3.0; // 3% por segundo
int direcaoMovimento = 0; // 0: parado, 1: frente, -1: trás
int direcaoRotacao = 0; // 0: sem rotação, 1: direita, -1: esquerda
int ultimoX = 0, ultimoY = 0;
Objeto3D vaca, cactus, veiculo, dog, arvore;

// Declarações das funções
bool lerTextureMapDoArquivo(const string& nomeArquivo, int& linhas, int& colunas);
void liberarTextureMap();
void cleanup();

// Função para verificar se uma posição está dentro dos limites da cidade
bool estaDentroDosLimites(double x, double z) {
    return x >= 0 && x < QtdX && z >= 0 && z < QtdZ;
}

// Função para verificar se uma posição é uma rua válida
bool ehRuaValida(double x, double z) {
    int ix = (int)x;
    int iz = (int)z;
    
    if (!estaDentroDosLimites(ix, iz)) return false;
    
    // Verifica se não é uma calçada (valor 12 no textureMap)
    return textureMap[iz][ix] != 12;
}

// Função para verificar colisão com combustível
void verificarCombustivel() {
    int ix = (int)posicaoCarro.x;
    int iz = (int)posicaoCarro.z;
    
    if (estaDentroDosLimites(ix, iz) && textureMap[iz][ix] == -1) {
        combustivel = 100.0; // Reabastece
        cout << "Combustível coletado! Nível: " << combustivel << "%" << endl;
    }
}

// Função para verificar se o carro está em uma posição válida
bool posicaoValida(double x, double z) {
    // Verifica se está dentro dos limites
    if (!estaDentroDosLimites(x, z)) {
        cout << "Colisão: Fora dos limites - x:" << x << " z:" << z << endl;
        return false;
    }
    
    // Usa (int)x e (int)z para alinhar com o grid
    int ix = (int)x;
    int iz = (int)z;
    
    // Verifica se é uma calçada (valor 12 no textureMap original)
    if (textureMap[iz][ix] == 12) {
        cout << "Colisão: Calçada - x:" << x << " z:" << z << " ix:" << ix << " iz:" << iz << " textureMap:" << textureMap[iz][ix] << endl;
        return false;
    }
    
    // Permite movimento em qualquer lugar que não seja calçada
    return true;
}


void carregarObjetosTRI() {
    if (vaca.carregarTRI("Cow.tri")){
        vaca.modeloCarregado = true;
        vaca.escala = 0.003f;
        vaca.rotacao = 90;
        vaca.cor = White;
    } else {
        cout << "Erro ao carregar o objeto vaca" << endl;
    }

    if (dog.carregarTRI("dog.tri")){
        dog.modeloCarregado = true;
        dog.escala = 0.1;
        dog.cor = Red;
        dog.rotacao = 90;
    } else {
        cout << "Erro ao carregar o objeto dog" << endl;
    }

    if (cactus.carregarTRI("cactus.tri")){
        cactus.modeloCarregado = true;
        cactus.escala = 0.2;
        cactus.cor = GreenYellow;
    } else {
        cout << "Erro ao carregar o objeto cactus" << endl;
    }

    if (arvore.carregarTRI("tree.tri")){
        arvore.modeloCarregado = true;
        arvore.escala = 0.4;
        arvore.cor = GreenCopper;
    } else {
        cout << "Erro ao carregar o objeto arvore" << endl;
    }
    
    if (veiculo.carregarTRI("moto.tri")){
        veiculo.modeloCarregado = true;
        veiculo.escala = 0.2;
        veiculo.cor = SkyBlue;
        veiculo.rotacao = 90;
    } else {
        cout << "Erro ao carregar o objeto veiculo" << endl;
    }
}

// **********************************************************************
//
// **********************************************************************
void ImprimeCidade() {
    for (int i = 0; i < QtdZ; i++) {
        for (int j = 0; j < QtdX; j++) {
            cout << Cidade[i][j].corDoPiso << " ";
        }
        cout << endl;
    }
}

// **********************************************************************
// void InicializaCidade(int QtdX, int QtdZ)
// Esta funcao sera substituida por uma que le a matriz que representa
// a Cidade
// **********************************************************************
void InicializaCidade(int QtdX, int QtdZ)
{
    for (int i=0;i<QtdZ;i++) {
        for (int j=0;j<QtdX;j++)
        {
            if (textureMap[i][j] == -5) {
                Cidade[i][j].tipo = PREDIO;
                Cidade[i][j].corDoObjeto = i * 5;
            } else if (textureMap[i][j] == -1) {
                Cidade[i][j].tipo = COMBUSTIVEL;
                Cidade[i][j].corDoObjeto = Green;
            } else if (textureMap[i][j] == 12) {
                Cidade[i][j].tipo = CALCADA;
                Cidade[i][j].corDoObjeto = White;
            } else if (textureMap[i][j] == VAZIO) {
                Cidade[i][j].corDoObjeto = White;
                Cidade[i][j].textureID = textureMap[i][j];
                Cidade[i][j].tipo = VAZIO;
            } else {
                Cidade[i][j].corDoPiso = rand()%40;
                Cidade[i][j].corDoObjeto = White;
                Cidade[i][j].textureID = textureMap[i][j];
                Cidade[i][j].tipo = RUA;
            }
        }
    }
}

// **********************************************************************
// void PosicionaEmTerceiraPessoa()
//   Este metodo posiciona o obeservador fora do cenario, olhando para o
// centro do mapa
// As variaveis "TerceiraPessoa" e "PosicaoVeiculo" sao setadas na INIT
// **********************************************************************
void PosicionaEmTerceiraPessoa() {
    Observador = Ponto(QtdX/2, 30, QtdZ + 10);   // Posicao do Observador
    Alvo = Ponto(QtdX/2, 0, QtdZ/2);        // Posicao do Alvo
}

void posicionaEmPrimeiraPessoa() {
    // Posiciona a câmera na posição do carro com rotação
    double alturaCamera = 2.0;
    Observador = Ponto(posicaoCarro.x, posicaoCarro.y + alturaCamera, posicaoCarro.z);
    
    // Calcula a direção do olhar baseada na rotação do carro e da câmera
    double distanciaOlhar = 5.0;
    
    Alvo.x = posicaoCarro.x + distanciaOlhar * sin(anguloCarro * M_PI / 180.0);
    Alvo.y = posicaoCarro.y + alturaCamera;
    Alvo.z = posicaoCarro.z + distanciaOlhar * cos(anguloCarro * M_PI / 180.0);
}

void InicializaTexturas() {
    LoadTexture ("None.png");//0
    LoadTexture ("CROSS.png");//1
    LoadTexture ("DL.png");//2
    LoadTexture ("DLR.png");//3
    LoadTexture ("DR.png");//4
    LoadTexture ("LR.png");//5
    LoadTexture ("UD.png");//6
    LoadTexture ("UDL.png");//7
    LoadTexture ("UDR.png");//8
    LoadTexture ("UL.png");//9
    LoadTexture ("ULR.png");//10
    LoadTexture ("UR.png");//11
    LoadTexture ("bricks.jpg");//12
}

// **********************************************************************
//  void init(void)
//    Inicializa os parametros globais de OpenGL
// **********************************************************************
void init(void)
{
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f); // Fundo de tela preto
   
    glClearDepth(1.0);
    glDepthFunc(GL_LESS);
    glShadeModel(GL_SMOOTH); // (GL_FLAT);
    glColorMaterial ( GL_FRONT, GL_AMBIENT_AND_DIFFUSE );
    
    glEnable(GL_DEPTH_TEST);
    //glEnable (GL_CULL_FACE);
    
    if (ModoDeExibicao) // Faces Preenchidas??
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    glEnable(GL_NORMALIZE);
    
    // Carrega a matriz textureMap do arquivo
    int linhas, colunas;
    if (!lerTextureMapDoArquivo("textureMap.txt", linhas, colunas)) {
        cout << "Erro ao carregar textureMap. Usando valores padrão." << endl;
        // Em caso de erro, define valores padrão
        QtdX = 30;
        QtdZ = 30;
    } else {
        // Define as dimensões baseadas no arquivo carregado
        QtdX = colunas;
        QtdZ = linhas;
    }
    
    InicializaTexturas();
    InicializaCidade(QtdX, QtdZ);
    
    // Define a posicao do observador e do veiculo
    // com base no tamanho do mapa
    TerceiraPessoa = Ponto(QtdX/2, 30, QtdZ + 10);
    PosicaoVeiculo = Ponto(QtdX/2, 0, QtdZ/2);
    posicaoCarro = Ponto(2.5,0,2.5);    
    PosicionaEmTerceiraPessoa();
    glDisable(GL_TEXTURE_2D);
    
    AnguloDeVisao = 45;
    UseTexture (-1); // desabilita o uso de textura, inicialmente
    carregarObjetosTRI();
}
// **********************************************************************
//
// **********************************************************************
void animate() {
    double dt;
    dt = T.getDeltaT();
    AccumDeltaT += dt;
    TempoTotal += dt;
    nFrames++;

    // Movimento do carro com velocidade constante
    if (percorrer && combustivel > 0) {
        double distancia = velocidade * dt; // 3 m/s
        
        // Calcula nova posição baseada na direção do carro
        double novaX = posicaoCarro.x;
        double novaZ = posicaoCarro.z;
        
         double anguloRad = anguloCarro * M_PI / 180.0;
            
            // Sempre move para frente quando percorrer está ativo
            novaX += distancia * sin(anguloRad);
            novaZ += distancia * cos(anguloRad);
            
            // Verifica se a nova posição é válida
            if (posicaoValida(novaX, novaZ)) {
                posicaoCarro.x = novaX;
                posicaoCarro.z = novaZ;
                combustivel -= consumoCombustivel * dt;
                verificarCombustivel();
                
                // Para o carro se combustível chegar a 0
                if (combustivel <= 0) {
                    combustivel = 0;
                    direcaoMovimento = 0;
                    direcaoRotacao = 0;
                    percorrer = false;
                    cout << "Combustível esgotado! Carro parado." << endl;
                }
            } else {
                // Colisão detectada - para o movimento
                percorrer = false;
                direcaoMovimento = 0;
                cout << "=== COLISÃO DETECTADA ===" << endl;
                cout << "Posição atual: x=" << posicaoCarro.x << " z=" << posicaoCarro.z << endl;
                cout << "Tentativa de movimento para: x=" << novaX << " z=" << novaZ << endl;
                
                // Debug da posição que causou colisão
                int novaIX = (int)novaX;
                int novaIZ = (int)novaZ;
                if (estaDentroDosLimites(novaIX, novaIZ)) {
                    cout << "Tipo na nova posição: " << Cidade[novaIZ][novaIX].tipo << endl;
                } else {
                    cout << "Nova posição fora dos limites!" << endl;
                }
                cout << "=========================" << endl;
            }
        
        // Rotação do carro
        if (direcaoRotacao != 0) {
            anguloCarro += direcaoRotacao * 90.0 * dt; // 90 graus por segundo
            if (anguloCarro >= 360) anguloCarro -= 360;
            if (anguloCarro < 0) anguloCarro += 360;
        }
    }

    if (AccumDeltaT > 1.0/30) {
        AccumDeltaT = 0;
        angulo+= 1;
        glutPostRedisplay();
    }

    if (TempoTotal > 5.0) {
        cout << "Tempo Acumulado: "  << TempoTotal << " segundos. " ;
        cout << "Nros de Frames sem desenho: " << nFrames << endl;
        cout << "FPS(sem desenho): " << nFrames/TempoTotal << endl;
        TempoTotal = 0;
        nFrames = 0;
    }
}

void posicionaCarro() {
    glTranslated(posicaoCarro.x, posicaoCarro.y, posicaoCarro.z);
    
    // Aplica a rotação do carro
    glRotatef(anguloCarro, 0.0, 1.0, 0.0);
}

void DesenhaCarro() {
    posicionaCarro();
     if (!ModoDeProjecao) return;
    
    // Diminui o tamanho do carro para 0.2
    glScalef(0.2, 0.2, 0.2);
    
    // Carro centralizado - o centro está em (0,0,0)
    // Corpo principal do carro
    glPushMatrix();
        defineCor(Red);
        // Corpo principal: 4x2x6 unidades, centralizado
        glScalef(4.0, 2.0, 6.0);
        glutSolidCube(1.0);
    glPopMatrix();
    
    // Teto do carro (mais baixo)
    glPushMatrix();
        defineCor(Red);
        glTranslatef(0, 1.5, 0);
        glScalef(3.0, 1.0, 4.0);
        glutSolidCube(1.0);
    glPopMatrix();
    
    // Rodas - posicionadas nos cantos
    defineCor(Black);
    
    // Roda dianteira esquerda
    glPushMatrix();
        glTranslatef(-1.5, 0.5, -2.5);
        glRotatef(90, 0, 1, 0);
        glutSolidTorus(0.3, 0.8, 16, 8);
    glPopMatrix();
    
    // Roda dianteira direita
    glPushMatrix();
        glTranslatef(1.5, 0.5, -2.5);
        glRotatef(90, 0, 1, 0);
        glutSolidTorus(0.3, 0.8, 16, 8);
    glPopMatrix();
    
    // Roda traseira esquerda
    glPushMatrix();
        glTranslatef(-1.5, 0.5, 2.5);
        glRotatef(90, 0, 1, 0);
        glutSolidTorus(0.3, 0.8, 16, 8);
    glPopMatrix();
    
    // Roda traseira direita
    glPushMatrix();
        glTranslatef(1.5, 0.5, 2.5);
        glRotatef(90, 0, 1, 0);
        glutSolidTorus(0.3, 0.8, 16, 8);
    glPopMatrix();
    
    // Faróis
    defineCor(Yellow);
    
    // Farol esquerdo
    glPushMatrix();
        glTranslatef(-1.0, 0.5, -3.0);
        glScalef(0.3, 0.3, 0.1);
        glutSolidSphere(1.0, 8, 8);
    glPopMatrix();
    
    // Farol direito
    glPushMatrix();
        glTranslatef(1.0, 0.5, -3.0);
        glScalef(0.3, 0.3, 0.1);
        glutSolidSphere(1.0, 8, 8);
    glPopMatrix(); 

    veiculo.desenhar();
}


// **********************************************************************
//  Desenha um predio no meio de uma c lula
// **********************************************************************
void DesenhaPredio(float altura, int cor)
{
    float fator = 0.5f;            // 50% da altura original
    float h = altura * fator;      

    defineCor(cor);
    glPushMatrix();
        // sobe metade de "h" para alinhar a base em y=0
        glScalef(0.5f, h, 0.5f);
        glTranslatef(0.0f, 0.5f, 0.0f);
        // escala X,Z fixo; Y = h (já com o fator)
        glutSolidCube(1);
    glPopMatrix();
    defineCor(White);
}

// **********************************************************************
// void DesenhaLadrilhoTex(int idTextura)
// **********************************************************************
void DesenhaLadrilhoTEX(int idTextura)
{
    UseTexture(idTextura); // Habilita a textura id_textura
    glBegin ( GL_QUADS );
        glNormal3f(0,1,0);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-0.5f,  0.0f, -0.5f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-0.5f,  0.0f,  0.5f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f( 0.5f,  0.0f,  0.5f);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f( 0.5f,  0.0f, -0.5f);
    glEnd();
    glDisable(GL_TEXTURE_2D);

}

// **********************************************************************
// void DesenhaLadrilho(int corBorda, int corDentro)
// Desenha uma celula do piso.
// Eh possivel definir a cor da borda e do interior do piso
// O ladrilho tem largula 1, centro no (0,0,0) e esta sobre o plano XZ
// Para aqueles que forem usar texturas, esta funcao serah substituida
// pela funcao "DesenhaLadrilhoTex"
// **********************************************************************
void DesenhaLadrilho(int corBorda, int corDentro)
{
    defineCor(corDentro); // desenha QUAD preenchido
    glBegin ( GL_QUADS );
        glNormal3f(0,1,0);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-0.5f,  0.0f, -0.5f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-0.5f,  0.0f,  0.5f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f( 0.5f,  0.0f,  0.5f);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f( 0.5f,  0.0f, -0.5f);
    glEnd();
    
    defineCor(corBorda);
    glBegin ( GL_LINE_STRIP ); // desenha borda do ladrilho
        glNormal3f(0,1,0);
        glVertex3f(-0.5f,  0.01f, -0.5f);
        glVertex3f(-0.5f,  0.01f,  0.5f);
        glVertex3f( 0.5f,  0.01f,  0.5f);
        glVertex3f( 0.5f,  0.01f, -0.5f);
    glEnd();
}

void desenhaGasolina() {
    DesenhaPredio(2, Green); // por ser o msm objeto apenas abstraimos
}

void desenhaVaca() {
    glPushMatrix();
        vaca.desenhar();
    glPopMatrix();
}

void desenhaCactus() {
    glPushMatrix();
        cactus.desenhar();
    glPopMatrix();
}

void desenhaDog() {
    glPushMatrix();
        dog.desenhar();
    glPopMatrix();
}

void desenhaArvore() {
    glPushMatrix();
        arvore.desenhar();
    glPopMatrix();
}


void desenhaObstaculo(int indice) {
    if (indice % 4 == 0) {
        desenhaDog();
        return;
    }

    if (indice % 5 == 0) {
        desenhaCactus();
        return;
    }

    if (indice % 7 == 0) {
        desenhaVaca();
        return;
    }

    if (indice % 3 == 0) {
        desenhaArvore();
        return;
    }

}

void DesenhaCidade(int QtdX, int QtdZ) {
    glPushMatrix();
    defineCor(White);
    int p =0;
    for(int x=0; x<QtdX;x++)
    {
        glPushMatrix();
        for(int z=0; z<QtdZ;z++)
        {
            bool desenharObstaculo = true;
            if (Cidade[x][z].tipo == PREDIO) {
                if (p == 5) p = 0;
                DesenhaPredio((z * 1.2f) * 0.5f, Cidade[x][z].corDoObjeto);
                DesenhaLadrilhoTEX(12); // coloca o chão como vermelho
                desenharObstaculo = false;
            } else if (Cidade[x][z].tipo == COMBUSTIVEL) {
                desenhaGasolina();
                DesenhaLadrilhoTEX(12); // coloca o chão como vermelho
                desenharObstaculo = false;
            } else if (Cidade[x][z].tipo == CALCADA) {
                
                DesenhaLadrilhoTEX(12); // coloca o chão como vermelho
            } else {
               // desenhaObstaculo(z);
                DesenhaLadrilhoTEX(Cidade[x][z].textureID);
            }


            glTranslated(0, 0, 1);
        }
        
        glPopMatrix();
        glTranslated(1, 0, 0);
    }
    glPopMatrix();

}

// **********************************************************************
//  void DefineLuz(void)
// **********************************************************************
void DefineLuz(void)
{
  // Define cores para um objeto dourado
  GLfloat LuzAmbiente[]   = {0.4, 0.4, 0.4f } ;
  GLfloat LuzDifusa[]   = {0.7, 0.7, 0.7};
  GLfloat LuzEspecular[] = {0.9f, 0.9f, 0.9 };
    
  glLoadIdentity();
  GLfloat PosicaoLuz0[]  = {Alvo.x, Alvo.y, Alvo.z}; 


  //DesenhaPredio(100, VioletRed);
  GLfloat Especularidade[] = {1.0f, 1.0f, 1.0f};

   // ****************  Fonte de Luz 0

  glEnable ( GL_COLOR_MATERIAL );

   // Habilita o uso de iluminacao
  glEnable(GL_LIGHTING);

  // Ativa o uso da luz ambiente
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, LuzAmbiente);
    
  // Define os parametros da luz numero Zero
  glLightfv(GL_LIGHT0, GL_AMBIENT, LuzAmbiente);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, LuzDifusa  );
  glLightfv(GL_LIGHT0, GL_SPECULAR, LuzEspecular  );
  glLightfv(GL_LIGHT0, GL_POSITION, PosicaoLuz0 );
  glEnable(GL_LIGHT0);

  // Ativa o "Color Tracking"
  glEnable(GL_COLOR_MATERIAL);

  // Define a reflectancia do material
  glMaterialfv(GL_FRONT,GL_SPECULAR, Especularidade);

  // Define a concentracaoo do brilho.
  // Quanto maior o valor do Segundo parametro, mais
  // concentrado sera o brilho. (Valores v lidos: de 0 a 128)
  glMateriali(GL_FRONT,GL_SHININESS,50);

}

// **********************************************************************
//  void PosicUser()
// **********************************************************************
void PosicUser()
{

    // Define os parametros da projecao Perspectiva
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Define o volume de visualizacao sempre a partir da posicao do
    // observador

    if (ModoDeProjecao == 0) {
        posicionaEmPrimeiraPessoa();
    } else {
        PosicionaEmTerceiraPessoa(); 
    }

    gluPerspective(AnguloDeVisao,AspectRatio,0.01,1500); // Projecao perspectiva

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    gluLookAt(Observador.x, Observador.y, Observador.z,   // Posicao do Observador
              Alvo.x, Alvo.y, Alvo.z,     // Posicao do Alvo
              0.0f,1.0f,0.0f); // UP


}
// **********************************************************************
//  void reshape( int w, int h )
//		trata o redimensionamento da janela OpenGL
//
// **********************************************************************
void reshape( int w, int h )
{

	// Evita divisao por zero, no caso de uma janela com largura 0.
	if(h == 0) h = 1;
    // Ajusta a relacao entre largura e altura para evitar distorcao na imagem.
    // Veja funcao "PosicUser".
    AspectRatio = (float)w / h; // recalcula o AspectRatio
    
	// Reset the coordinate system before modifying
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    
	// Ajusta a viewport para ocupar toda a janela
    //glViewport(0, 0, w, h);
    
    // Seta a viewport para ocupar a parte superior da janela
    glViewport(0, h*AlturaViewportDeMensagens, w, h-h*AlturaViewportDeMensagens);
    
    gluPerspective(AnguloDeVisao,AspectRatio,0.01,1500); // Projecao perspectiva

}
// **********************************************************************
//
// **********************************************************************
void printString(string s, int posX, int posY, int cor)
{
    defineCor(cor);
    
    glRasterPos3i(posX, posY, 0); //define posicao na tela
    for (int i = 0; i < s.length(); i++)
    {
//GLUT_BITMAP_HELVETICA_10,
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, s[i]);
    }
    
}
// **********************************************************************
// Esta funcao permite que se escreva mensagens na tela
//
// **********************************************************************
void DesenhaEm2D()
{
    int ativarLuz = false;
    if (glIsEnabled(GL_LIGHTING))
    {
        glDisable(GL_LIGHTING);
        ativarLuz = true;
    }

    // Salva o estado atual do OpenGL
    glPushAttrib(GL_TRANSFORM_BIT | GL_VIEWPORT_BIT | GL_ENABLE_BIT);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);
    glViewport(0, 0, w, h*AlturaViewportDeMensagens);
    glOrtho(0,10, 0,10, 0,1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Linha divisória
    defineCor(GreenCopper);
    glLineWidth(10);
    glBegin(GL_LINES);
        glVertex2f(0,9.5);
        glVertex2f(10,9.5);
    glEnd();

    // Título centralizado
    printString("SIMULADOR DE CIDADE", 2.5, 8.8, White);

    // Coluna esquerda: status e informações do carro
    int colEsq = 0;
    float y = 7.8;
    float step = 1.1;
    string infoStatus = percorrer ? "Status: Movendo" : "Status: Parado";
    if (combustivel <= 0) infoStatus = "Status: Sem Combustível";
    printString(infoStatus, colEsq, y, percorrer ? Green : Yellow); y -= step;
    string infoCombustivel = "Combustível: " + to_string((int)combustivel) + "%";
    int corCombustivel = (combustivel <= 20) ? Red : Green;
    printString(infoCombustivel, colEsq, y, corCombustivel); y -= step;
    string infoPosicao = "Pos: (" + to_string((int)posicaoCarro.x) + ", " + to_string((int)posicaoCarro.z) + ")";
    printString(infoPosicao, colEsq, y, White); y -= step;
    string infoVelocidade = "Velocidade: " + to_string((int)velocidade) + " m/s";
    printString(infoVelocidade, colEsq, y, White); y -= step;
    string infoCamera = ModoDeProjecao == 0 ? "Câmera: 1ª Pessoa" : "Câmera: 3ª Pessoa";
    printString(infoCamera, colEsq, y, White);

    // Coluna direita: controles e debug
    int colDir = 5;
    y = 7.8;
    printString("Controles:", colDir, y, White); y -= step;
    printString("Setas: Girar", colDir, y, White); y -= step;
    printString("Espaço: Iniciar/Parar", colDir, y, White); y -= step;
    printString("P: Mudar Câmera", colDir, y, White); y -= step;
    printString("D: Debug Posição", colDir, y, White); y -= step;
    printString("A: Debug Área", colDir, y, White); y -= step;

    // Restaura o estado anterior do OpenGL
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();

    if (ativarLuz)
        glEnable(GL_LIGHTING);
}

// **********************************************************************
//  void display( void )
//
//
// **********************************************************************
void display( void )
{

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glMatrixMode(GL_MODELVIEW);


    glLineWidth(2);
	
	PosicUser();
	glMatrixMode(GL_MODELVIEW);
    
    DesenhaCidade(QtdX,QtdZ);
    DesenhaCarro();    

    DefineLuz();
    DesenhaEm2D();

    

    defineCor(Green);
	glutSwapBuffers();
}


// Função para processar teclas especiais (setas)
void processarSetas(int tecla) {
    switch (tecla) {
        case GLUT_KEY_LEFT:
            direcaoRotacao = -1;
            break;
        case GLUT_KEY_RIGHT:
            direcaoRotacao = 1;
            break;
        case GLUT_KEY_UP:
            direcaoMovimento = 1;
            break;
        case GLUT_KEY_DOWN:
            direcaoMovimento = -1;
            break;
    }
}

// Função para parar movimento quando tecla é solta
void pararMovimento(int tecla) {
    switch (tecla) {
        case GLUT_KEY_LEFT:
        case GLUT_KEY_RIGHT:
            direcaoRotacao = 0;
            break;
        case GLUT_KEY_UP:
        case GLUT_KEY_DOWN:
            direcaoMovimento = 0;
            break;
    }
}


// Função para debug - mostra informações sobre a posição atual
void debugPosicao() {
    int ix = (int)posicaoCarro.x;
    int iz = (int)posicaoCarro.z;
    
    cout << "=== DEBUG POSIÇÃO ===" << endl;
    cout << "Posição atual: x=" << posicaoCarro.x << " z=" << posicaoCarro.z << endl;
    cout << "Índices: ix=" << ix << " iz=" << iz << endl;
    
    if (estaDentroDosLimites(ix, iz)) {
        cout << "Valor no textureMap: " << textureMap[iz][ix] << endl;
        
        // Interpreta o valor
        if (textureMap[iz][ix] == -5) {
            cout << "Tipo: PRÉDIO (obstáculo)" << endl;
        } else if (textureMap[iz][ix] == -1) {
            cout << "Tipo: COMBUSTÍVEL" << endl;
        } else if (textureMap[iz][ix] == 12) {
            cout << "Tipo: CALÇADA (obstáculo)" << endl;
        } else if (textureMap[iz][ix] == 0) {
            cout << "Tipo: VAZIO" << endl;
        } else {
            cout << "Tipo: RUA/ÁREA LIVRE" << endl;
        }
    } else {
        cout << "Posição fora dos limites!" << endl;
    }
    cout << "=====================" << endl;
}

// Função para mostrar área ao redor da posição atual
void debugAreaAoRedor() {
    int ix = (int)posicaoCarro.x;
    int iz = (int)posicaoCarro.z;
    
    cout << "=== DEBUG ÁREA 3x3 AO REDOR ===" << endl;
    cout << "Posição central: x=" << posicaoCarro.x << " z=" << posicaoCarro.z << endl;
    cout << "Índices centrais: ix=" << ix << " iz=" << iz << endl;
    cout << endl;
    
    for (int i = iz - 1; i <= iz + 1; i++) {
        for (int j = ix - 1; j <= ix + 1; j++) {
            if (i >= 0 && i < QtdZ && j >= 0 && j < QtdX) {
                cout << textureMap[i][j] << "\t";
            } else {
                cout << "X\t";
            }
        }
        cout << endl;
    }
    cout << endl;
    cout << "Legenda:" << endl;
    cout << "-5 = PRÉDIO (obstáculo)" << endl;
    cout << "-1 = COMBUSTÍVEL" << endl;
    cout << "12 = CALÇADA (obstáculo)" << endl;
    cout << "0 = VAZIO" << endl;
    cout << "Outros = RUA/ÁREA LIVRE" << endl;
    cout << "X = Fora dos limites" << endl;
    cout << "=============================" << endl;
}

// **********************************************************************
//  void keyboard ( unsigned char key, int x, int y )
//
//
// **********************************************************************
void keyboard ( unsigned char key, int x, int y )
{
	switch ( key ) {
    case 27:        // Termina o programa qdo
      cleanup();    // a tecla ESC for pressionada
      exit ( 0 );   
      break; 
    case 32:
        percorrer = !percorrer;
        if (percorrer) {
            // Quando começar a percorrer, define movimento para frente
            direcaoMovimento = 1;
        } else {
            // Quando parar, zera todas as direções
            direcaoMovimento = 0;
            direcaoRotacao = 0;
        }
        break;       
    case 'p':
        ModoDeProjecao = !ModoDeProjecao;
        glutPostRedisplay();
        break;
    case 'e':
        ModoDeExibicao = !ModoDeExibicao;
        init();
        glutPostRedisplay();
        break;
    case 't':
        ComTextura = !ComTextura;
        break;
    case 'd':
        debugPosicao();
        break;
    case 'a':
        debugAreaAoRedor();
        break;
    default:
            cout << key;
    break;
  }
}

// **********************************************************************
//  void arrow_keys ( int a_keys, int x, int y )  
//
//
// **********************************************************************
void arrow_keys ( int a_keys, int x, int y )  
{
	switch ( a_keys ) 
	{
		case GLUT_KEY_UP:       // When Up Arrow Is Pressed...
			processarSetas(GLUT_KEY_UP);
			break;
	    case GLUT_KEY_DOWN:     // When Down Arrow Is Pressed...
			processarSetas(GLUT_KEY_DOWN);
			break;
		case GLUT_KEY_LEFT:     // When Left Arrow Is Pressed...
			processarSetas(GLUT_KEY_LEFT);
			break;
		case GLUT_KEY_RIGHT:    // When Right Arrow Is Pressed...
			processarSetas(GLUT_KEY_RIGHT);
			break;
		default:
			break;
	}
}

// Função para detectar quando teclas especiais são soltas
void arrow_keys_up(int a_keys, int x, int y) {
    pararMovimento(a_keys);
}

// **********************************************************************
//  void main ( int argc, char** argv )
// **********************************************************************
int  main ( int argc, char** argv )
{
    cout << "Simulador de Cidade" << endl;
    
    
    system("pwd");

    glutInit            ( &argc, argv );
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB );
    glutInitWindowPosition (0,0);

    // Define o tamanho inicial da janela grafica do programa
    glutInitWindowSize  ( 900, 700);
   
    // Cria a janela na tela, definindo o nome da
    // que aparecera na barra de titulo da janela.
    glutCreateWindow    ( "Simulador de Cidades" );

    // executa algumas inicializacoes
    init ();

    // Define que o tratador de evento para
    // o redesenho da tela. A funcao "display"
    // sera' chamada automaticamente quando
    // for necessario redesenhar a janela
    glutDisplayFunc ( display );

    // Define que o tratador de evento para
    // o invalidacao da tela. A funcao "display"
    // sera' chamada automaticamente sempre que a
    // maquina estiver ociosa (idle)
    glutIdleFunc(animate);

    // Define que o tratador de evento para
    // o redimensionamento da janela. A funcao "reshape"
    // sera chamada automaticamente quando
    // o usuario alterar o tamanho da janela
    glutReshapeFunc ( reshape );

    // Define que o tratador de evento para
    // as teclas. A funcao "keyboard"
    // sera chamada automaticamente sempre
    // o usuario pressionar uma tecla comum
    glutKeyboardFunc ( keyboard );

    // Define que o tratador de evento para
    // as teclas especiais(F1, F2,... ALT-A,
    // ALT-B, Teclas de Seta, ...).
    // A funcao "arrow_keys" sera chamada
    // automaticamente sempre o usuario
    // pressionar uma tecla especial
    glutSpecialFunc ( arrow_keys );

    // Função para detectar quando teclas especiais são soltas
    glutSpecialUpFunc(arrow_keys_up);

    // inicia o tratamento dos eventos
    glutMainLoop ( );

    return 0;
}

// Função para ler a matriz textureMap do arquivo
bool lerTextureMapDoArquivo(const string& nomeArquivo, int& linhas, int& colunas) {
    ifstream arquivo(nomeArquivo);
    if (!arquivo.is_open()) {
        cout << "Erro ao abrir o arquivo: " << nomeArquivo << endl;
        return false;
    }
    
    vector<vector<int>> matrizTemp;
    string linha;
    
    // Lê o arquivo linha por linha
    while (getline(arquivo, linha)) {
        vector<int> valoresLinha;
        stringstream ss(linha);
        int valor;
        
        // Lê os valores separados por tabulação
        while (ss >> valor) {
            valoresLinha.push_back(valor);
        }
        
        if (!valoresLinha.empty()) {
            matrizTemp.push_back(valoresLinha);
        }
    }
    
    arquivo.close();
    
    if (matrizTemp.empty()) {
        cout << "Arquivo vazio ou formato inválido" << endl;
        return false;
    }
    
    // Define as dimensões
    linhas = matrizTemp.size();
    colunas = matrizTemp[0].size();
    
    // Verifica se todas as linhas têm o mesmo tamanho
    for (int i = 1; i < linhas; i++) {
        if (matrizTemp[i].size() != colunas) {
            cout << "Erro: Linhas com tamanhos diferentes" << endl;
            return false;
        }
    }
    
    // Aloca a matriz dinâmica
    textureMap = new int*[linhas];
    for (int i = 0; i < linhas; i++) {
        textureMap[i] = new int[colunas];
    }
    
    // Copia os valores para a matriz
    for (int i = 0; i < linhas; i++) {
        for (int j = 0; j < colunas; j++) {
            textureMap[i][j] = matrizTemp[i][j];
        }
    }
    
    cout << "Matriz textureMap carregada com sucesso: " << linhas << "x" << colunas << endl;
    return true;
}

// Função para liberar a memória da matriz textureMap
void liberarTextureMap() {
    if (textureMap != nullptr) {
        // Usa as dimensões atuais da matriz
        for (int i = 0; i < QtdZ; i++) {
            delete[] textureMap[i];
        }
        delete[] textureMap;
        textureMap = nullptr;
    }
}

// Função de cleanup para ser chamada quando o programa for encerrado
void cleanup() {
    liberarTextureMap();
    cout << "Memória liberada com sucesso." << endl;
}