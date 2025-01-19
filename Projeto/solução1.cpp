#include <iostream>
#include <vector>
#include <cmath>
#include <limits>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <random>

using namespace std;
using namespace std::chrono;

struct Ponto {
    int id;
    double x, y;
};

// Função para calcular a distância entre dois pontos
int calcularDistancia(const Ponto& a, const Ponto& b) {
    double xd = a.x - b.x;
    double yd = a.y - b.y;
    return round(sqrt(xd * xd + yd * yd));
}

// Pré-calcula a matriz de distâncias
vector<vector<int>> calcularMatrizDistancias(const vector<Ponto>& pontos) {
    int n = pontos.size();
    vector<vector<int>> distancias(n, vector<int>(n, 0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            distancias[i][j] = calcularDistancia(pontos[i], pontos[j]);
        }
    }
    return distancias;
}

// Retorna a maior distância no percurso
int maiorDistancia(const vector<int>& percurso, const vector<vector<int>>& distancias) {
    int maxDistance = 0;
    int n = percurso.size();
    for (int i = 0; i < n - 1; ++i) {
        int d = distancias[percurso[i] - 1][percurso[i + 1] - 1];
        maxDistance = max(maxDistance, d);
    }
    return maxDistance;
}

// Função para construir um percurso inicial usando GRASP
vector<int> construirPercursoGRASP(const vector<Ponto>& pontos, const vector<vector<int>>& distancias, double alpha) {
    int n = pontos.size();
    vector<bool> visitado(n, false);
    vector<int> percurso;
    percurso.reserve(n);

    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis(0.0, 1.0);

    int atual = 0;
    visitado[atual] = true;
    percurso.push_back(pontos[atual].id);

    while (percurso.size() < n) {
        vector<pair<int, int>> candidatos;

        for (int i = 0; i < n; ++i) {
            if (!visitado[i]) {
                int dist = 0;
                for (int j : percurso) {
                    int idAtual = j - 1;
                    dist = max(dist, distancias[idAtual][i]);
                }
                candidatos.emplace_back(dist, i);
            }
        }

        // Ordena os candidatos por menor distância máxima
        sort(candidatos.begin(), candidatos.end());

        // Seleciona um próximo ponto aleatoriamente entre os melhores (controlado por alpha)
        int limite = max(1, (int)(candidatos.size() * alpha));
        int escolhido = candidatos[dis(gen) * limite].second;

        visitado[escolhido] = true;
        percurso.push_back(pontos[escolhido].id);
        atual = escolhido;
    }

    percurso.push_back(percurso[0]);
    return percurso;
}

// Refinamento usando 2-opt
vector<int> Opt(vector<int> percurso, const vector<vector<int>>& distancias, int maxIter) {
    bool melhoria = true;
    int n = percurso.size();
    int iteracao = 0;

    while (melhoria && iteracao < maxIter) {
        melhoria = false;
        ++iteracao;

        for (int i = 1; i < n - 2; ++i) {
            for (int j = i + 1; j < n - 1; ++j) {
                int atualDist = distancias[percurso[i - 1] - 1][percurso[i] - 1] +
                                distancias[percurso[j] - 1][percurso[j + 1] - 1];

                int novaDist = distancias[percurso[i - 1] - 1][percurso[j] - 1] +
                               distancias[percurso[i] - 1][percurso[j + 1] - 1];

                if (novaDist < atualDist) {
                    reverse(percurso.begin() + i, percurso.begin() + j + 1);
                    melhoria = true;
                }
            }
        }
    }

    return percurso;
}

int main() {
    cout << "Digite o nome do arquivo para gravar a solução: ";
    string arquivoSaida;
    cin >> arquivoSaida;

    vector<Ponto> pontos;
    Ponto ponto;

    cout << "Insira os pontos:" << endl;
    while (cin >> ponto.id >> ponto.x >> ponto.y) {
        pontos.push_back(ponto);
    }

    auto startTotal = high_resolution_clock::now();

    // Pré-calcula a matriz de distâncias
    auto distancias = calcularMatrizDistancias(pontos);

    // Parâmetros do GRASP
    int iteracoes = 10;  // Número de iterações do GRASP
    double alpha = 0.3;  // Controle da aleatoriedade (0.0: guloso, 1.0: completamente aleatório)
    int maxIterBuscaLocal = 100;

    vector<int> melhorPercurso;
    int menorMaiorDistancia = numeric_limits<int>::max();

    for (int it = 0; it < iteracoes; ++it) {
        // Construção inicial com GRASP
        vector<int> percurso = construirPercursoGRASP(pontos, distancias, alpha);

        // Refinamento usando 2-opt
        percurso = Opt(percurso, distancias, maxIterBuscaLocal);

        // Avaliação
        int maxDistance = maiorDistancia(percurso, distancias);
        if (maxDistance < menorMaiorDistancia) {
            menorMaiorDistancia = maxDistance;
            melhorPercurso = percurso;
        }
    }

    auto endTotal = high_resolution_clock::now();
    cout << "Tempo total de execução: "
         << duration_cast<seconds>(endTotal - startTotal).count() << " s" << endl;

    // Grava a solução no arquivo especificado
    ofstream arquivo(arquivoSaida);
    if (arquivo.is_open()) {
        for (int id : melhorPercurso) {
            arquivo << id << " ";
        }
        arquivo << endl;
        arquivo.close();
    } else {
        cerr << "Erro ao abrir o arquivo " << arquivoSaida << " para escrita.\n";
        return 1;
    }

    cout << "Maior distância entre dois pontos: " << menorMaiorDistancia << endl;

    return 0;
}
