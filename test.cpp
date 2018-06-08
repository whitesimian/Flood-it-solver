#include <iostream>
#include <vector>
#include <list>
#include <set>
#include <algorithm>
#include <iterator>
#include <fstream>
#include <cassert>

//#define DEBUG

#define fr(inicio, fim) for(int i = inicio; i < fim; i++)
#define frr(inicio, fim) for(int j = inicio; j < fim; j++)
#define frrr(inicio, fim) for(int k = inicio; k < fim; k++)

#define print(container) for(auto elem : container) cout << elem << " "

using namespace std;

struct SubsetPair {
	int pai;
	int rank;

	SubsetPair(int a, int b) : pai(a), rank(b) {};
};

int numero_vertices, numero_arestas, numero_cores, pivo, target_colour;
vector< list<int> > grafo; // Lista de adjacencias
vector< set<int> > adjacencias; // index = vertice pai do conjunto, conteudo = conjuntos adjacentes (pais)
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
	adjacencias[destination].erase(destination); // Remove duplicata
	cor[destination] = target_colour; // Colore o conjunto
}

// --------------------------------------------------------------------------------

//Union-Find (Disjoint Set By Rank And Path Compression)

inline int busca(int vertice) { //small constant -> O(log n) Pior Caso
	if (subset[vertice].pai != vertice)
		subset[vertice].pai = busca(subset[vertice].pai); //Compressao de caminhos no subconjunto
	return subset[vertice].pai; //Retorna pai atualizado
}

void uniao(int v1, int v2) {
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

/*int flood(int flooding, int flooded) {	// Une o conjunto do vertice flooding com o do vertice flooded e atualiza adjacencias
										// Aqui, flooding e flooded serao necessariamente os vertices que nomeiam os conjuntos (pais)
	uniao(flooding, flooded);
	int conj_atual = busca(flooding);

	if (conj_atual != flooding)
		group_up(flooded, flooding);
	else
		group_up(flooding, flooded);

	return conj_atual;
}*/

int flood_aux(int flooding, int flooded) {
	uniao(flooding, flooded);

	int conj_atual = busca(flooding);

	if (conj_atual != flooding)
		group_up(flooded, flooding);
	else
		group_up(flooding, flooded);

	return conj_atual;
}

int flood(int cor_inundada, int flooding) {
	set<int>::iterator it = adjacencias[flooding].begin();

	while (it != adjacencias[flooding].end()) {
		int iter1 = *it;
		if (cor[iter1] == cor_inundada) { // Area a ser inundada			
			flooding = flood_aux(flooding, iter1); // Une as duas areas e retorna o conjunto vertice atual

			set<int>::iterator it2 = adjacencias[iter1].begin(); // Nao alterado na chamada de "group_up"
			while (it2 != adjacencias[iter1].end()) {
				//cout << "b " << *it2 << "\n"
				int iter2 = *it2;
				if (cor[iter2] == target_colour) { // Procura pelos vertices de distancia 2 ao pivo que possuem a mesma cor de inundacao
					flooding = flood_aux(flooding, iter2); // Une as areas e atualiza o vertice pivo
					//cout << "ENTROU COM: " << *it2 << "\n";
				}

				//cout << "grupos: " << flooding << " " << busca(pivo) << "\n";

				//print(adjacencias[iter1]); cout << "\n";

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

void simulate_flood(int area, int cor_inundada, vector< set<int> >& colour_count) { // Simula inundacao da "cor_inundada".
	for (int elem : adjacencias[area]) {
		if (elem != busca(pivo)) { // Area diferente da que contem o pivo
			if (cor[elem] != target_colour)
				colour_count[cor_inundada].insert(cor[busca(elem)]);
			else
				simulate_flood(elem, cor_inundada, colour_count); // Quer dizer que uma area adjacente a "elem" tem a mesma cor da area pivo.
		}
	}
}

//=================================================================================

int main(int argc, char** argv)
{
	//ios_base::sync_with_stdio(false);
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

	fr(1, numero_vertices + 1)
		file >> cor[i];

	fr(0, numero_arestas)
	{
		int v1, v2;
		file >> v1 >> v2;
		grafo[v1].push_back(v2);
		grafo[v2].push_back(v1);
	}

	//================================================================================= FIM DAS ENTRADAS

	target_colour = cor[pivo];

	fr(0, numero_vertices + 1)
		subset.emplace_back(i, 0); // Pai = i, Rank = 0

	{
		vector< vector<int> > adj;
		adj.resize(numero_vertices + 1);
		vector<int> foo;

		fr(1, numero_vertices + 1) { // O(n + m)
			if (!visitado[i]) {
				foo.clear();
				dfs(i, foo); // Une toda a regiao monocromatica relativa ao vertice atomico "i" // foo retorna todas as adjacencias atomicas a area
				int pos = busca(i); // numero do conjunto
				adj[pos].insert(adj[pos].begin(), foo.begin(), foo.end()); // vertices atomicos adjacentes ao conjunto
			}
		}
		fr(1, numero_vertices + 1) {
			for (auto elem : adj[i])
				adjacencias[i].insert(busca(elem)); // Popula adjacencias
		}
	}

	//================================================================================= FIM DO PRE-PROCESSAMENTO

	// HEURISTICA DE MAXIMIZACAO DAS OPCOES DE ESCOLHA

	{
		int grupo_pivo = busca(pivo);
		vector< set<int> > colour_count(numero_cores + 1);
		set<int> cores_inicial;
		unsigned int passos = 0;

		while (true) {
			passos++;
			colour_count.clear();
			colour_count.resize(numero_cores + 1);

			cores_inicial.clear();
			for (int area : adjacencias[grupo_pivo]) {
				//cout << "============= AREA: " << area << "\n";
				cores_inicial.insert(cor[busca(area)]);
			}

			//cout << "passos: " << passos << ", tam:" << adjacencias[grupo_pivo].size() << "\n\n";

			//vector< set<int> >::iterator it = colour_count.begin();
			//advance(it, 1); // A partir da cor 1

			set<int>::iterator it = cores_inicial.begin();
			while (it != cores_inicial.end()) {
				int iter = *it;
				colour_count[iter].insert(cores_inicial.begin(), cores_inicial.end());
				colour_count[iter].erase(iter); // Retira a propria cor do adjacente											

				advance(it, 1);
			}

			for (int area : adjacencias[grupo_pivo])  // Simula inundacao de "area"
				simulate_flood(area, cor[busca(area)], colour_count);

			///////////////////////////////////////////////////////////////////////

			/*print(cores_inicial); cout << "\n";

			int pos = 0;
			for (auto v : colour_count) {
				cout << pos++ << ": ";
				for (auto elem : v)
					cout << elem << " ";
				cout << "\n";
			}

			system("pause");*/

			///////////////////////////////////////////////////////////////////////

			vector< set<int> >::iterator choice = max_element(colour_count.begin(), colour_count.end(), [](const set<int>& a, const set<int>& b) { return a.size() < b.size(); }); // Aponta 'a cor inundada que maximizara' as opcoes de escolha
			int cor_escolhida = distance(colour_count.begin(), choice); 
			if (cor_escolhida == 0) // Quando para qualquer cor escolhida, o numero de opcoes for zero e' porque so resta uma cor
				break;				// max_element pegara a primeira igual a zero (== 0)
			
			cout << "COR ESCOLHIDA: " << cor_escolhida << "\n";

			grupo_pivo = flood(cor_escolhida, grupo_pivo);

			///////
			/*cout << "\nConjuntos:\n";
			fr(1, numero_vertices + 1) cout << busca(i) << " ";
			cout << "\n";

			cout << "\nAdjacencias: (areas)\n";
			int i = 0;
			for (auto elem : adjacencias) {
				cout << i++ << ": ";
				for (auto v : elem)
					cout << v << " ";
				cout << "\n";
			}

			cout << "GRUPO PIVO: " << grupo_pivo << ", tam: " << adjacencias[grupo_pivo].size() << "\n";*/
			///////

			assert(busca(pivo) == grupo_pivo);
		}

		cout << "Numero estimado de passos: " << passos << "\n";
	}

	return 0;
}
