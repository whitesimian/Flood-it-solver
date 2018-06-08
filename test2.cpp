#include <iostream>
#include <vector>
#include <list>
#include <set>
#include <fstream>

#define DEBUG

#define fr(inicio, fim) for(int i = inicio; i < fim; i++)
#define frr(inicio, fim) for(int j = inicio; j < fim; j++)
#define frrr(inicio, fim) for(int k = inicio; k < fim; k++)

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
		if (cor[*it] == cor_inundada) { // Area a ser inundada			
			flooding = flood_aux(flooding, *it); // Une as duas areas e retorna o conjunto vertice atual

			set<int>::iterator it2 = adjacencias[*it].begin(); // Nao alterado na chamada de "group_up"

			while (it2 != adjacencias[*it].end()) {
				if (cor[*it2] == target_colour) // Procura pelos vertices de distancia 2 ao pivo que possuem a mesma cor de inundacao
					flooding = flood_aux(flooding, *it2); // Une as areas e atualiza o vertice pivo
				++it2;
			}

			it = adjacencias[flooding].begin(); // "flooding" pode ter mudado
			continue;
		}
		++it;
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

#ifdef DEBUG
		cout << "Adjacencias atomicas:\n";
		int j = 0;
		for (auto elem : adj) {
			cout << j++ << ": ";
			for (auto v : elem)
				cout << v << " ";
			cout << "\n";
		}
#endif
	}

	//================================================================================= FIM DO PRE-PROCESSAMENTO

#ifdef DEBUG

	int s = 1;
	while (s) {
		cout << "\nAdjacencias: (areas)\n";
		int i = 0;
		for (auto elem : adjacencias) {
			cout << i++ << ": ";
			for (auto v : elem)
				cout << v << " ";
			cout << "\n";
		}

		cout << "\nConjuntos:\n";
		fr(1, numero_vertices + 1) cout << busca(i) << " ";
		cout << "\n";

		int grupo_pivo = busca(pivo);
		cout << "\nPivo: " << grupo_pivo << "\n";
		cout << "Inundar (adjacente ao pivo " << grupo_pivo << "): ";
		cin >> s;
		grupo_pivo = flood(cor[busca(s)], grupo_pivo);
		if (adjacencias[grupo_pivo].size() == 0)
			break;
	}

	system("pause");
#endif



	return 0;
}
