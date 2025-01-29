#include <iostream>
#include <vector>
#include <cmath>
#include <limits>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <random>
#include <sstream>

using namespace std;
using namespace std::chrono;

// Estrutura para representar um ponto no espaço bidimensional
struct Ponto {
    int id;      // Identificador do ponto
    double x, y; // Coordenadas x e y
};

// Constantes para cálculos geográficos
const double RRR = 6378.388; // Raio médio da Terra em quilômetros
const double PI = 3.141592;

// Função auxiliar para remover espaços em branco extras de uma string
string auxLeitura(const string& str) {
    size_t start = str.find_first_not_of(" \t");
    if (start == string::npos) return "";
    size_t end = str.find_last_not_of(" \t");
    return str.substr(start, end - start + 1);
}

// Converte coordenadas de graus para radianos
double converterParaRadianos(double coordenada) {
    int graus = static_cast<int>(coordenada);
    double minutos = coordenada - graus;
    return PI * (graus + 5.0 * minutos / 3.0) / 180.0;
}

// Calcula a distância Euclidiana entre dois pontos
int calcularDistanciaEuclidiana(const Ponto& a, const Ponto& b) {
    double xd = a.x - b.x;
    double yd = a.y - b.y;
    return round(sqrt(xd * xd + yd * yd));
}

// Calcula a distância geográfica entre dois pontos (considerando curvatura da Terra)
int calcularDistanciaGeografica(const Ponto& a, const Ponto& b) {
    double latitudeA = converterParaRadianos(a.x);
    double longitudeA = converterParaRadianos(a.y);
    double latitudeB = converterParaRadianos(b.x);
    double longitudeB = converterParaRadianos(b.y);

    double q1 = cos(longitudeA - longitudeB);
    double q2 = cos(latitudeA - latitudeB);
    double q3 = cos(latitudeA + latitudeB);

    double distancia = RRR * acos(0.5 * ((1.0 + q1) * q2 - (1.0 - q1) * q3)) + 1.0;
    return static_cast<int>(distancia);
}

// Calcula a matriz de distâncias entre os pontos com base no tipo de métrica
vector<vector<int>> calcularMatrizDistancias(const vector<Ponto>& pontos, const string& tipoPeso) {
    string tipo = auxLeitura(tipoPeso);
    int n = pontos.size();
    vector<vector<int>> distancias(n, vector<int>(n, 0));
    
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (tipo == "EUC_2D") {
                distancias[i][j] = calcularDistanciaEuclidiana(pontos[i], pontos[j]);
            } else if (tipo == "GEO") {
                distancias[i][j] = calcularDistanciaGeografica(pontos[i], pontos[j]);
            }
        }
    }
    return distancias;
}

// Encontra a maior distância entre dois pontos em um percurso
int maiorDistancia(const vector<int>& percurso, const vector<vector<int>>& distancias) {
    int maxDistance = 0;
    int n = percurso.size();
    for (int i = 0; i < n - 1; ++i) {
        int d = distancias[percurso[i] - 1][percurso[i + 1] - 1];
        maxDistance = max(maxDistance, d);
    }
    return maxDistance;
}

// Constrói um percurso inicial utilizando o algoritmo GRASP
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
        
        sort(candidatos.begin(), candidatos.end());
        int limite = max(1, (int)(candidatos.size() * alpha));
        int escolhido = candidatos[dis(gen) * limite].second;
        
        visitado[escolhido] = true;
        percurso.push_back(pontos[escolhido].id);
        atual = escolhido;
    }

    percurso.push_back(percurso[0]);
    return percurso;
}

// Aplica a busca local 2-Opt para melhorar o percurso
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

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Uso: " << argv[0] << " <nome_do_arquivo_saida>" << endl;
        return 1;
    }

    string arquivoSaida = argv[1];
    vector<Ponto> pontos;
    string tipoPesoAresta;
    string linha;
    bool lerCoordenadas = false;
    while (getline(cin, linha)) {
        linha = auxLeitura(linha);
        if (linha.empty()) continue;
        if (linha.find("EDGE_WEIGHT_TYPE") != string::npos) {
            tipoPesoAresta = auxLeitura(linha.substr(linha.find(":") + 1));
        }
        if (linha == "NODE_COORD_SECTION") {
            lerCoordenadas = true;
            continue;
        }
        if (linha == "EOF") break;
        if (lerCoordenadas) {
            stringstream ss(linha);
            Ponto p;
            if (ss >> p.id >> p.x >> p.y) pontos.push_back(p);
        }
    }

    
    auto startTotal = high_resolution_clock::now();
    auto distancias = calcularMatrizDistancias(pontos, tipoPesoAresta);

    int iteracoes = 10;
    double alpha = 0.3;
    int maxIterBuscaLocal = 100;

    auto startFirst = high_resolution_clock::now();
    vector<int> primeiroPercurso = construirPercursoGRASP(pontos, distancias, alpha);
    auto endFirst = high_resolution_clock::now();

    int primeiraMaiorDistancia = maiorDistancia(primeiroPercurso, distancias);

    cout << "\nTempo do primeiro percurso: "
         << duration_cast<milliseconds>(endFirst - startFirst).count() << " ms" << endl;
    cout << "Maior distância entre dois pontos do primeiro percurso: " << primeiraMaiorDistancia << endl;
	
	primeiroPercurso = Opt(primeiroPercurso, distancias, maxIterBuscaLocal);
    vector<int> melhorPercurso = primeiroPercurso;
    int menorMaiorDistancia = primeiraMaiorDistancia;

    for (int it = 1; it < iteracoes; ++it) {
        vector<int> percurso = construirPercursoGRASP(pontos, distancias, alpha);
        percurso = Opt(percurso, distancias, maxIterBuscaLocal);

        int maxDistance = maiorDistancia(percurso, distancias);
        if (maxDistance < menorMaiorDistancia) {
            menorMaiorDistancia = maxDistance;
            melhorPercurso = percurso;
        }
    }
	
	auto endTotal = high_resolution_clock::now();
    cout << "Tempo total de execução: " 
         << duration_cast<milliseconds>(endTotal - startTotal).count() << " ms" << endl;

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
    
    cout << "Maior distância entre dois pontos após otimização: " << menorMaiorDistancia << endl;
    
    return 0;
}
