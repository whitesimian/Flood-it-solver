#include <iostream>
#include <vector>
#include <list>
#include <set>
#include <algorithm>
#include <iterator>
#include <fstream>
#include <ctime>
#include <ratio>
#include <chrono>
#include <cassert>

/*
* === CONSIDERAÇÕES ===
* Grafo conexo.
* Cores no intervalo de 1 a número de cores.
* As areas se juntam em uma area com a cor do vertice pivo.
*/

#define f(inicio, fim) for(int i = inicio; i < fim; i++)
#define print(container) for(auto elem : container) cout << elem << " "
//#define VERIFY

using namespace std;

struct SubsetPair {
	int pai;
	int rank;

	SubsetPair(int a, int b) : pai(a), rank(b) {};
};

int numero_vertices, numero_arestas, numero_cores, pivo, target_colour;
vector< list<int> > grafo; // Lista de adjacencias
vector< set<int> > adjacencias; // Index = vertice pai do conjunto, conteudo = conjuntos adjacentes (pais)
vector<int> cor;
vector<SubsetPair> subset; // Define conjunto de vertices monocromaticos adjacentes

vector<bool> visitado; // DFS

					   //=================================================================================

void group_up(int destination, int source) { // Atualiza adjacencias da area que inunda -> pai do conjunto = 'destination'
	adjacencias[destination].insert(adjacencias[source].begin(), adjacencias[source].end());
	for (auto v : adjacencias[source]) { // Para todos os adjacentes ao inundado
		adjacencias[v].erase(source); // Remove adjacente antigo (inundado)
		adjacencias[v].insert(destination); // Inclui novo adjacente (nova area formada)
	}
	adjacencias[destination].erase(destination); // Remove iguais
	cor[destination] = target_colour; // Colore o conjunto
}

// --------------------------------------------------------------------------------

// Union-Find (Disjoint Set By Rank And Path Compression) -> Une vertices em um mesmo conjunto

inline int busca(int vertice) { // Small constant -> O(log n) Pior Caso
	if (subset[vertice].pai != vertice)
		subset[vertice].pai = busca(subset[vertice].pai); // Compressao de caminhos no subconjunto
	return subset[vertice].pai; // Retorna pai atualizado
}

void uniao(int v1, int v2) { // Une grupos de acordo com rank
	int grupo1 = busca(v1);
	int grupo2 = busca(v2);

	if (grupo1 != grupo2)
	{
		if (subset[grupo1].rank < subset[grupo2].rank)
			subset[grupo1].pai = grupo2;
		else if (subset[grupo1].rank > subset[grupo2].rank)
			subset[grupo2].pai = grupo1;
		else
		{
			subset[grupo1].pai = grupo2;
			subset[grupo2].rank++;
		}
	}
}

// --------------------------------------------------------------------------------

int flood_aux(int flooding, int flooded) { // Para evitar repetica na funcao flood
	uniao(flooding, flooded);

	int conj_atual = busca(flooding);

	if (conj_atual != flooding)
		group_up(flooded, flooding);
	else
		group_up(flooding, flooded);

	return conj_atual;
}

int flood(int cor_inundada, int flooding) {  // Para evitar repetica na funcao flood
	set<int>::iterator it = adjacencias[flooding].begin();

	while (it != adjacencias[flooding].end()) {
		int iter1 = *it;
		if (cor[iter1] == cor_inundada) { // Area a ser inundada			
			flooding = flood_aux(flooding, iter1); // Une as duas areas e retorna o conjunto vertice atual

			set<int>::iterator it2 = adjacencias[iter1].begin(); // Nao alterado na chamada de "group_up"
			while (it2 != adjacencias[iter1].end()) {
				int iter2 = *it2;
				if (cor[iter2] == target_colour) { // Procura pelos vertices de distancia 2 ao pivo que possuem a mesma cor de inundacao
					flooding = flood_aux(flooding, iter2); // Une as areas e atualiza o vertice pivo
				}

				advance(it2, 1);
			}

			it = adjacencias[flooding].begin(); // "flooding" pode ter mudado
			assert(flooding = busca(pivo));
			continue;
		}
		advance(it, 1);
	}

	return flooding; // Area pivo atual
}

void dfs(int vertice, vector<int>& lista_adjacentes) {
	visitado[vertice] = true;

	for (auto elem : grafo[vertice]) {
		if (cor[elem] == cor[vertice]) {
			if (!visitado[elem]) {
				uniao(elem, vertice);
				dfs(elem, lista_adjacentes);
			}
		}
		else
			lista_adjacentes.push_back(elem);
	}
}

void simulate_flood(int area, int cor_inundada, vector< vector<int> >& colour_count) { // Simula inundacao da "cor_inundada".
	for (int elem : adjacencias[area]) {
		if (elem != busca(pivo)) { // Area diferente da que contem o pivo
			if (cor[elem] != target_colour)
				colour_count[cor_inundada][cor[busca(elem)]]++;
			else
				simulate_flood(elem, cor_inundada, colour_count); // Quer dizer que uma area adjacente a "elem" tem a mesma cor da area pivo.
		}
	}
}

//=================================================================================

int main(int argc, char** argv)
{
	ios_base::sync_with_stdio(false);
	//cin.tie(NULL);

	ifstream file(argv[1]);
	if (!file.is_open()) {
		cerr << "Arquivo texto nao encontrado.\n";
		exit(1);
	}

	file >> numero_vertices >> numero_arestas >> numero_cores >> pivo;
	grafo.resize(numero_vertices + 1);
	cor.resize(numero_vertices + 1);
	adjacencias.resize(numero_vertices + 1);

	visitado.assign(numero_vertices + 1, false); // Inicia visitados = false

	f(1, numero_vertices + 1)
		file >> cor[i];

	f(0, numero_arestas)
	{
		int v1, v2;
		file >> v1 >> v2;
		grafo[v1].push_back(v2);
		grafo[v2].push_back(v1);
	}

	//================================================================================= FIM DAS ENTRADAS

	chrono::high_resolution_clock::time_point tempo_inicio = chrono::high_resolution_clock::now();

	target_colour = cor[pivo];

	f(0, numero_vertices + 1)
		subset.emplace_back(i, 0); // Pai = i, Rank = 0

	{
		vector< vector<int> > adj;
		adj.resize(numero_vertices + 1);
		vector<int> foo;

		f(1, numero_vertices + 1) { // O(n + m)
			if (!visitado[i]) {
				foo.clear();
				dfs(i, foo); // Une toda a regiao monocromatica relativa ao vertice atomico "i" // foo retorna todas as adjacencias atomicas a area
				int pos = busca(i); // numero do conjunto
				adj[pos].insert(adj[pos].begin(), foo.begin(), foo.end()); // vertices atomicos adjacentes ao conjunto
			}
		}
		f(1, numero_vertices + 1) {
			for (auto elem : adj[i])
				adjacencias[i].insert(busca(elem)); // Popula adjacencias
		}
	}

	//================================================================================= FIM DO PRE-PROCESSAMENTO

	// HEURISTICA DE MAXIMIZACAO DE ADJACENCIAS DE MESMA COR

	{
		int grupo_pivo = busca(pivo);
		vector< vector<int> > colour_count; // Linha = cor a inundar, Coluna = contagem das cores.

		set<int> cores_inicial;
		unsigned int passos = 0;

		while (true) {
			passos++;
			colour_count.clear();
			colour_count.assign(numero_cores + 1, vector<int>(1, 0)); // Matriz com uma coluna de zeros

			cores_inicial.clear();
			for (int area : adjacencias[grupo_pivo])
				cores_inicial.insert(cor[busca(area)]); // Todas as cores adjacentes 'a area pivo

			for (int cor_adj : cores_inicial) { // Para todas as cores adjacentes 'a area pivo
				colour_count[cor_adj].assign(numero_cores + 1, 0);

				set<int>::iterator it = cores_inicial.begin();
				while (it != cores_inicial.end()) {
					int pos = *it;
					if (pos != cor_adj) // Uma cor nao e' adjacente a ela mesma
						colour_count[cor_adj][pos]++;
					advance(it, 1);
				}
			}

			for (int area : adjacencias[grupo_pivo])  // Simula inundacao de "area"
				simulate_flood(area, cor[busca(area)], colour_count);

			vector< vector<int> >::iterator choice = max_element(colour_count.begin(), colour_count.end(), 
				[](const vector<int>& a, const vector<int>& b) { 
					return *max_element(a.begin(), a.end()) < *max_element(b.begin(), b.end());
				});		// Aponta 'a cor inundada que maximizara' o numero de adjacencias de mesma cor.
			
			int cor_escolhida = distance(colour_count.begin(), choice);
			if (cor_escolhida == 0) // Quando para qualquer cor escolhida a maior qtde de adjacencias de mesma cor for zero, e' porque resta apenas uma area a ser inundada.
				break;				// max_element pegara a primeira igual a zero (cor 0 -> inexistente)

			grupo_pivo = flood(cor_escolhida, grupo_pivo);

			assert(busca(pivo) == grupo_pivo);
		}

		chrono::high_resolution_clock::time_point tempo_fim = chrono::high_resolution_clock::now();
		chrono::duration<double> time_span = chrono::duration_cast< chrono::duration<double> >(tempo_fim - tempo_inicio);

#ifdef VERIFY
		set<int> verifiicacao;
		f(1, numero_vertices + 1)
			verifiicacao.insert(busca(i));
		assert(verifiicacao.size() == 2); // Uniu todas as areas exceto a ultima
#endif

		cout << "\nNumero estimado de passos: " << passos << "\n"
			<< "Tempo: " << time_span.count() << " s\n";

	}

	return 0;
}
