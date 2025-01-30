# Trabalho-Pratico---Algoritmos-em-Grafos# **Otimização de Rotas com Variação do Problema do Caixeiro Viajante**

## **Descrição**
Este projeto aborda uma variação do Problema do Caixeiro Viajante (TSP - Traveling Salesman Problem), com o objetivo de minimizar a maior distância entre dois pontos consecutivos no percurso. A aplicação prática inclui otimização de rotas de transporte público para melhorar acessibilidade e eficiência.

O programa recebe como entrada:
- Um arquivo para gravar o percurso final.
- Pontos a serem visitados.

Como saída:
- Grava o percurso otimizado no arquivo.
- Imprime a maior distância entre dois pontos consecutivos na tela.

## **Funcionalidade**
- Solução baseada em uma implementação do método GRASP (Greedy Randomized Adaptive Search Procedure), retornando um percurso em que a maior distância entre dois pontos do caminho seja mínima.

## **Tecnologias Utilizadas**
- Linguagem: **C++**
- Bibliotecas:
  - `<iostream>`
  - `<vector>`
  - `<cmath>`
  - `<limits>`
  - `<fstream>`
  - `<algorithm>`
  - `<chrono>`
  - `<random>`
 
 ## **Como utilizar**

- git clone https://github.com/pradohugo/Trabalho-Pratico---Algoritmos-em-Grafos.git.
- cd Trabalho-Pratico-Algoritmos-em-Grafos.
- g++ -o TrabalhoFinal.exe TrabalhoFinal.cpp.
- TrabalhoFinal.exe saida.txt < 01.ins.


