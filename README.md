# Simulador de Cidade 3D em OpenGL

Simulação 3D de uma cidade em C++ e OpenGL: um carro percorre ruas texturizadas, desvia de calçadas e precisa reabastecer para não ficar sem combustível, em um mapa montado a partir de um arquivo de texto.

Trabalho em grupo da disciplina de **Computação Gráfica** da PUCRS (2025/1), construído sobre o código-base da disciplina.

## O que tem no projeto

- **Mapa por arquivo:** a cidade é lida de `textureMap.txt`, onde cada número define o que fica em cada célula (rua, calçada, prédio, árvore, posto de combustível…).
- **Texturas:** cada tipo de rua usa uma textura diferente (retas, curvas e cruzamentos), carregadas com a biblioteca SOIL.
- **Objetos 3D:** modelos carregados de arquivos `.tri` (carro, árvores, vaca, cachorro, cactos e outros).
- **Carro controlável:** movimento com o teclado, detecção de colisão com as calçadas.
- **Combustível:** o carro consome combustível ao andar e reabastece ao passar pelos postos; um painel mostra o nível na tela.
- **Prédios** gerados a partir do mapa, com alturas variadas.

## Como rodar

Pré-requisitos: compilador C++ (g++) e OpenGL/GLUT instalados.

**Linux ou macOS:**

```bash
make
./BasicoOpenGL
```

**Windows:** o repositório inclui `freeglut.dll` e as pastas `include/` e `lib/`, além de um executável já compilado (`ProgOpenGL.exe`).

## Tecnologias

C++ · OpenGL · GLUT · SOIL (texturas) · Transformações geométricas · Modelagem 3D
