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
* FELIPE C�SAR LOPES MACHADO 16.2.5890
* JUL/2018
*/

/*
* === CONSIDERA��ES ===
* Grafo conexo.
* Cores no intervalo de 1 a n�mero de cores.
* As �reas se juntam em uma �rea com a cor do ve�rtice piv�.
*/

#define f(inicio, fim) for(int i = inicio; i < fim; i++)
#define print(container) for(auto elem : container) cout << elem << " "
//#define _PRINT_INITIAL_SOLUTION

using namespace std;

void group_up(vector< set<int> >& adjacentes, int destination, int source);
inline int find(int vertex);
bool union_set(int v1, int v2);
int flood_aux(vector< set<int> >& adjacentes, int flooding, int flooded);
int flood(vector< set<int> >& adjacentes, int flooded_colour, int flooding);
void dfs(int vertice, vector<int>& lista_adjacentes);
void simulate_flood(const vector< set<int> >& adjacentes, int area, int cor_inundada, vector< vector<int> >& colour_count, vector< set<int> >& usados);

struct SubsetPair {
	int pai;
	int rank;

	SubsetPair(int a = 0, int b = 0) : pai(a), rank(b) {};
};

int number_vertices, number_edges, number_colours, pivo, target_colour;
vector< list<int> > graph; // Lista de adjacencias
vector<int> colour, solution;
vector<SubsetPair> subset; // Define conjunto de vertices monocromaticos adjacentes

vector<bool> visited; // DFS

//=================================================================================

int main(int argc, char** argv)
{
	ios_base::sync_with_stdio(false); 
	//cin.tie(NULL);

	vector< set<int> > adjacencies; // Index = vertice pai do conjunto, conteudo = conjuntos adjacentes (pais)

	ifstream file(argv[1]);
	if (!file.is_open()) {
		cerr << "Input file not found.\n";
		exit(1);
	}

	file >> number_vertices >> number_edges >> number_colours >> pivo;
	graph.resize(number_vertices + 1);
	colour.resize(number_vertices + 1);
	adjacencies.resize(number_vertices + 1);

	visited.assign(number_vertices + 1, false); // Inicia visitados = false

	f(1, number_vertices + 1)
		file >> colour[i];

	const vector<int> cor_vetor_inicial(colour.begin(), colour.end()); // Guarda c�pia das cores iniciais

	// Entrada em uma lista de adjac�ncias
	f(0, number_edges)
	{
		int v1, v2;
		file >> v1 >> v2;
		graph[v1].push_back(v2);
		graph[v2].push_back(v1);
	}

	//================================================================================= FIM DAS ENTRADAS

	chrono::high_resolution_clock::time_point tempo_inicio = chrono::high_resolution_clock::now();

	target_colour = colour[pivo];

	f(0, number_vertices + 1)
		subset.emplace_back(i, 0); // Pai = i, Rank = 0

	vector< vector<int> > adj;
	adj.resize(number_vertices + 1);
	vector<int> foo;

	// Recolhe todos os v�rtices at�micos adjacentes a cada uma das �reas visitadas (pos).	
	// A DFS une os v�rtices at�micos adjacentes de mesma cor em um mesmo conjunto (subset).
	f(1, number_vertices + 1) { // O(n + m)
		if (!visited[i]) {
			foo.clear();
			dfs(i, foo); // Une toda a regiao monocromatica relativa ao vertice atomico "i" // foo retorna todas as adjacencias atomicas a area
			int pos = find(i); // numero do conjunto
			adj[pos].insert(adj[pos].begin(), foo.begin(), foo.end()); // vertices atomicos adjacentes ao conjunto
		}
	}
	const vector<SubsetPair> subset_inicial(subset.begin(), subset.end()); // Copia conjuntos iniciais

	// Recolhe todos os v�rtices �rea adjacentes a todos os outros
	// Vetor de conjuntos
	f(1, number_vertices + 1) { // Para todas os v�rtices �rea
		for (auto elem : adj[i])
			adjacencies[i].insert(find(elem)); // Popula adjacencias
	} 

	const vector< set<int> > adjacencias_iniciais(adjacencies.begin(), adjacencies.end()); // Guarda c�pia das adjac�ncias iniciais

	//================================================================================= FIM DO PRE-PROCESSAMENTO

	// HEUR�STICA DE MAXIMIZA��O DE ADJAC�NCIAS EXTRAS DE MESMA COR COM CONSIDERA��O PARCIAL DAS J� EXISTENTES-> SOLU��O INICIAL

	int pivot_group = find(pivo);
	vector< vector<int> > colour_count; // Linha = cor a inundar, Coluna = contagem das cores.

	set<int> initial_colours;
	unsigned passos = 0;
	bool initial_solution_found = false;

	while (!initial_solution_found) {
		passos++;
		colour_count.clear();
		colour_count.assign(number_colours + 1, vector<int>(1, 0)); // Matriz com uma coluna de zeros

		set<int> areas_usadas; // �reas j� inseridas

		initial_colours.clear();
		// Para todas as �reas adjacentes � �rea do piv�
		for (int area : adjacencies[pivot_group]) {
			initial_colours.insert(colour[area]); // Todas as cores adjacentes � area piv�
			areas_usadas.insert(area);
			assert(area == find(area));
		}

		// Para cada cor a ser simulada sua inunda��o, existe um conjunto de �reas j� consideradas
		vector< set<int> > areas_usadas_por_cor(number_colours + 1);

		// Inicia vetores de contagem de cores para cada cor adjacente 
		// e faz a contagem inicial (cores adjacentes iniciais).
		// Para estas areas, a contagem de cores nao e' por frequencia.
		for (int cor_adj : initial_colours) { // Para todas as cores adjacentes � area piv� -> todas as cores poss�veis de ser escolhidas
			colour_count[cor_adj].assign(number_colours + 1, 0);
			areas_usadas_por_cor[cor_adj].insert(areas_usadas.begin(), areas_usadas.end()); // Todas as �reas iniciais j� foram contabilizadas

			for (int cor_ini : initial_colours) {
				if(cor_ini != cor_adj) // Uma cor nao � adjacente a ela mesma
					colour_count[cor_adj][cor_ini]++;
					// Considera��o parcial das cores adjacentes iniciais (sem contagem de frequ�ncia)
					// Conta-se UMA vez todas as cores iniciais.
					// Para esta cor (cor_ini) que � adjacente � �rea piv�, adiciona-se parcialmente (contagem "qualitativa") todas as cores iniciais.
			}
		}

		// Simula inunda��o de "�rea".
		// Relativo � todas as cores iniciais.
		for (int area : adjacencies[pivot_group]) {
			simulate_flood(adjacencies, area, colour[find(area)], colour_count, areas_usadas_por_cor);
		}

		// Aponta � cor inundada que maximizar� o n�mero de adjac�ncias de mesma cor.
		vector< vector<int> >::iterator choice = max_element(colour_count.begin(), colour_count.end(), 
			[](const vector<int>& a, const vector<int>& b) { 
				return *max_element(a.begin(), a.end()) < *max_element(b.begin(), b.end());
			});		

		// A cor � o �ndice no vetor.
		int chosen_colour = distance(colour_count.begin(), choice);
		if(chosen_colour != 0)
			solution.push_back(chosen_colour);

		else { // cor_escolhida == 0 : Quando para qualquer cor escolhida a maior qtde de adjac�ncias de mesma cor for zero, � porque resta apenas uma �rea a ser inundada.
									// max_element pegara a primeira igual a zero (cor 0 -> inexistente)
			int last_colour = colour[*adjacencies[pivot_group].begin()];
			for(int elem : adjacencies[pivot_group])
				assert(colour[elem] == last_colour); // As cores das �ltimas adjac�ncias s�o iguais (resta apenas uma cor).

			solution.push_back(last_colour); // Adiciona a �ltima cor inundada � solu��o.
			initial_solution_found = true;
			continue;
		}

		pivot_group = flood(adjacencies, chosen_colour, pivot_group);

		assert(find(pivo) == pivot_group);
	}

	assert(passos == solution.size());

#ifdef _PRINT_INITIAL_SOLUTION
		cout << "\nInitial solution: " << passos << "\n";
#endif

	// HEURISTICA DE REMOCAO SEQUENCIAL DE CORES DA SOLUCAO INICIAL (Descent, First Improvement)

		bool final_solution = false;
		int pos_pop = 0;

		if (passos > 0) {
			while (!final_solution) {
				copy(cor_vetor_inicial.begin(), cor_vetor_inicial.end(), colour.begin()); // Reinicia vetor de cores
				copy(subset_inicial.begin(), subset_inicial.end(), subset.begin()); // Reinicia o vetor de conjuntos
				pivot_group = find(pivo);

				adjacencies.clear();
				adjacencies.resize(number_vertices + 1);
				f(0, adjacencias_iniciais.size()) // Copia o estado inicial para "adjacencias"
					adjacencies[i].insert(adjacencias_iniciais[i].begin(), adjacencias_iniciais[i].end());

				bool solucao_viavel = false;
				f(0, solution.size()) { // Simula todas as inundacoes retirando um elemento da solucao

					if (i != pos_pop) { // Retira a cor na posicao pos_pop
						pivot_group = flood(adjacencies, solution[i], pivot_group);
						if (adjacencies[find(pivo)].size() == 0) {
							solution.resize(i + 1); // Retira o resto das cores (nao utilizadas para gerar a solucao viavel)
							solution.erase(solution.begin() + pos_pop); // Retira a cor naquela posicao desnecessaria
							pos_pop = 0; // Retorna ao inicio
							solucao_viavel = true;
							break;
						}
					}
				}

				if (solucao_viavel)
					continue;

				if (pos_pop == solution.size() - 1) // Chegou 'a ultima cor e nao gerou solucao
					final_solution = true;

				pos_pop++;
			}
		}

		passos = solution.size(); // Atualiza a quantidade de passos

		chrono::high_resolution_clock::time_point tempo_fim = chrono::high_resolution_clock::now();
		chrono::duration<double> time_span = chrono::duration_cast< chrono::duration<double> >(tempo_fim - tempo_inicio);

		cout << "\nEstimated number of steps: " << passos << "\n"
			<< "Time: " << time_span.count() << " s\n";

	exit(EXIT_SUCCESS);
}

//=================================================================================

void group_up(vector< set<int> >& adjacentes, int destination, int source) { // Atualiza adjacencias da area que inunda -> pai do conjunto = 'destination'.
	adjacentes[destination].insert(adjacentes[source].begin(), adjacentes[source].end());
	for (auto v : adjacentes[source]) { // Para todos os adjacentes ao inundado.
		adjacentes[v].erase(source); // Remove adjacente antigo (inundado).
		adjacentes[v].insert(destination); // Inclui novo adjacente (nova area formada).
	}
	adjacentes[destination].erase(destination); // Remove iguais.
	colour[destination] = target_colour; // Colore o conjunto.
}

// --------------------------------------------------------------------------------

// Union-Find (Disjoint Set By Rank And Path Compression) -> Une vertices em um mesmo conjunto

inline int find(int vertice) { // Small constant -> O(log n) Pior Caso
	if (subset[vertice].pai != vertice)
		subset[vertice].pai = find(subset[vertice].pai); // Compressao de caminhos no subconjunto
	return subset[vertice].pai; // Retorna pai atualizado
}

bool union_set(int v1, int v2) { // Une grupos de acordo com rank
	int grupo1 = find(v1);
	int grupo2 = find(v2);

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

		return true;
	}
	else
		return false;
}

// --------------------------------------------------------------------------------

int flood_aux(vector< set<int> >& adjacentes, int flooding, int flooded) { // Para evitar repetica na funcao flood
	if (union_set(flooding, flooded)) {
		int conj_atual = find(flooding);

		// Varia apenas qual n�mero (v�rtice at�mico) representa a �rea unida (piv�) atual.
		// O segundo argumento � o n�mero correspondente � �rea (piv�).
		if (conj_atual != flooding)
			group_up(adjacentes, flooded, flooding);
		else
			group_up(adjacentes, flooding, flooded);

		return conj_atual;
	}
	else
		return flooding;
}

int flood(vector< set<int> >& adjacent, int cor_inundada, int flooding) {
	set<int>::iterator it = adjacent[flooding].begin();

	// Evita encadeamentos proibidos.
	set<int> forbidden_areas;

	while (it != adjacent[flooding].end()) { // Para todos os adjacentes ao flooding.
		int iter1 = *it;
		if (colour[iter1] == cor_inundada && forbidden_areas.find(colour[iter1]) == forbidden_areas.end()) { // �rea a ser inundada.		
			flooding = flood_aux(adjacent, flooding, iter1); // Une as duas �reas e retorna o conjunto v�rtice atual.

			set<int>::iterator it2 = adjacent[iter1].begin(); // N�o alterado na chamada de "group_up".
			while (it2 != adjacent[iter1].end()) {
				int iter2 = *it2;

				if (colour[iter2] == target_colour) { // Procura pelos vertices de dist�ncia 2 ao piv� que possuem a mesma cor de inunda��o.
					// Para evitar que se encadeie inunda��es proibidas para apenas uma escolha de cor, j� que o iterador volta ao begin() mais adiante.
					for (int forb : adjacent[iter2]) {
						if (colour[find(forb)] == cor_inundada && adjacent[flooding].find(forb) == adjacent[flooding].end()) // Igual � cor a ser inundada por�m de dist�ncia > 1 � �rea piv�.
							forbidden_areas.insert(find(forb));
					}
					flooding = flood_aux(adjacent, flooding, iter2); // Une as �reas e atualiza o v�rtice piv�.
				}

				advance(it2, 1);
			}

			it = adjacent[flooding].begin(); // "flooding" pode ter mudado.
			assert(flooding = find(pivo));
			continue;
		}
		advance(it, 1);
	}

	return flooding; // �rea piv� atual.
}

void dfs(int vertice, vector<int>& lista_adjacentes) {
	visited[vertice] = true;

	for (auto elem : graph[vertice]) {
		if (colour[elem] == colour[vertice]) {
			if (!visited[elem]) {
				union_set(elem, vertice);
				dfs(elem, lista_adjacentes);
			}
		}
		else
			lista_adjacentes.push_back(elem);
	}
}

void simulate_flood(const vector< set<int> >& adjacentes, int area, int flooded_colour, vector< vector<int> >& colour_count, vector< set<int> >& used) { // Simula inunda��o da "cor_inundada".
	// Para todas as �reas adjacentes � "area".
	for (int ADJACENT_AREA : adjacentes[area]) {
		if (ADJACENT_AREA != find(pivo) && used[flooded_colour].find(ADJACENT_AREA) == used[flooded_colour].end()) { // �rea diferente da que cont�m o piv� e n�o usada por efeito de inunda��o daquela cor.
			if (colour[ADJACENT_AREA] != target_colour) { // Se a cor adjacente (dist�ncia 2) n�o for igual � cor do piv�.
				colour_count[flooded_colour][colour[find(ADJACENT_AREA)]]++; // Contagem de frequ�ncia das cores das �reas adjacentes.
				used[flooded_colour].insert(ADJACENT_AREA);
			}
			else {
				used[flooded_colour].insert(ADJACENT_AREA); // A �rea de mesma cor do piv� e dist�ncia 2 n�o poder� fazer parte de nova simula��o nesta cor candidata � inunda��o.
				simulate_flood(adjacentes, ADJACENT_AREA, flooded_colour, colour_count, used); // Quer dizer que uma �rea adjacente a "elem" tem a mesma cor da �rea piv�.
			}
		}
	}
}