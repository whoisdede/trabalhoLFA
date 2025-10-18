#ifndef MEALY_H
#define MEALY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// --- ESTRUTURAS DE DADOS ---

/**
 * @brief Representa uma única transição da máquina de Mealy.
 * Mapeia (estado_atual, simbolo_entrada) -> (proximo_estado, string_saida)
 */
typedef struct {
    int current_state; // Índice do estado atual no array 'states'
    char input_symbol;
    int next_state;    // Índice do próximo estado no array 'states'
    char* output_string; // String de saída (pode ser "", "0", "1", "\n", etc.)
} Transition;

/**
 * @brief Representa a Máquina de Mealy completa.
 */
typedef struct {
    char** states;          // Array de nomes dos estados (ex: "q0", "q1")
    int num_states;
    int start_state;       // Índice do estado inicial
    int* final_states;     // Array de índices dos estados finais
    int num_final_states;

    char* input_alphabet;   // String contendo todos os caracteres de entrada válidos
    char** output_alphabet; // Array de strings do alfabeto de saída
    int num_output_symbols;

    Transition* transitions;
    int num_transitions;
} MealyMachine;


// --- FUNÇÕES AUXILIARES ---

/**
 * @brief Encontra o índice de um estado pelo seu nome.
 * @return O índice ou -1 se não for encontrado.
 */
static inline int find_state_index(const MealyMachine* machine, const char* state_name) {
    if (!machine || !state_name) return -1;
    for (int i = 0; i < machine->num_states; i++) {
        if (strcmp(machine->states[i], state_name) == 0) {
            return i;
        }
    }
    return -1; // Não encontrado
}

/**
 * @brief Encontra a transição correspondente a um estado e um símbolo de entrada.
 * @return Um ponteiro para a transição ou NULL se não for encontrada.
 */
static inline const Transition* find_transition(const MealyMachine* machine, int current_state_index, char input_char) {
    for (int i = 0; i < machine->num_transitions; i++) {
        if (machine->transitions[i].current_state == current_state_index &&
            machine->transitions[i].input_symbol == input_char) {
            return &machine->transitions[i];
        }
    }
    return NULL; // Transição não definida
}

/**
 * @brief Lê o conteúdo completo de um arquivo para uma string.
 * O chamador é responsável por liberar a string retornada.
 */
static inline char* read_file_to_string(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Erro ao abrir arquivo de palavra de entrada");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = (char*)malloc(length + 1);
    if (!buffer) {
        fprintf(stderr, "Erro ao alocar memória para a palavra de entrada\n");
        fclose(file);
        return NULL;
    }

    // Lê o arquivo. Note que a palavra 'w' pode conter 'N'[cite: 49], 
    // então lemos byte a byte, mas precisamos tratar newlines
    // que *não* são parte da palavra (ex: \n do fim do arquivo).
    size_t read_len = fread(buffer, 1, length, file);
    buffer[read_len] = '\0';
    fclose(file);
    
    // Removemos newlines ou carriage returns que possam existir no
    // arquivo w.txt, mas que NÃO são o símbolo 'N'.
    // O símbolo 'N' é preservado.
    char* dst = buffer;
    char* src = buffer;
    while (*src) {
        if (*src != '\n' && *src != '\r') {
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\0';

    return buffer;
}


// --- FUNÇÕES PRINCIPAIS (IMPLEMENTAÇÃO) ---

/**
 * @brief Carrega a definição de uma Máquina de Mealy de um arquivo.
 * Aloca memória para a máquina.
 */
static inline MealyMachine* load_mealy_machine(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Erro ao abrir arquivo da máquina");
        return NULL;
    }

    MealyMachine* machine = (MealyMachine*)calloc(1, sizeof(MealyMachine));
    if (!machine) {
        fprintf(stderr, "Erro ao alocar memória para a máquina\n");
        fclose(file);
        return NULL;
    }

    char line_buffer[1024];
    char token_buffer[100];
    char* token;
    char* rest;

    // 1. Ler Estados (Q) [cite: 114]
    if (!fgets(line_buffer, sizeof(line_buffer), file)) { /* erro */ }
    rest = line_buffer;
    while ((token = strtok_r(rest, " \n", &rest))) {
        machine->num_states++;
        machine->states = (char**)realloc(machine->states, machine->num_states * sizeof(char*));
        machine->states[machine->num_states - 1] = strdup(token);
    }

    // 2. Ler Estado Inicial (q0) [cite: 115]
    if (!fgets(line_buffer, sizeof(line_buffer), file)) { /* erro */ }
    if (sscanf(line_buffer, "%s", token_buffer) == 1) {
        machine->start_state = find_state_index(machine, token_buffer);
        if (machine->start_state == -1) {
            fprintf(stderr, "Erro: Estado inicial '%s' não está na lista de estados.\n", token_buffer);
        }
    }

    // 3. Ler Estados Finais (F) [cite: 116]
    if (!fgets(line_buffer, sizeof(line_buffer), file)) { /* erro */ }
    rest = line_buffer;
    while ((token = strtok_r(rest, " \n", &rest))) {
        int state_idx = find_state_index(machine, token);
        if (state_idx != -1) {
            machine->num_final_states++;
            machine->final_states = (int*)realloc(machine->final_states, machine->num_final_states * sizeof(int));
            machine->final_states[machine->num_final_states - 1] = state_idx;
        }
    }

    // 4. Ler Alfabeto de Entrada (Sigma) [cite: 117]
    if (!fgets(line_buffer, sizeof(line_buffer), file)) { /* erro */ }
    machine->input_alphabet = (char*)malloc(strlen(line_buffer) + 1);
    char* dst = machine->input_alphabet;
    for (char* src = line_buffer; *src; src++) {
        if (*src != ' ' && *src != '\n' && *src != '\r') {
            *dst++ = *src;
        }
    }
    *dst = '\0';

    // 5. Ler Alfabeto de Saída (Delta) 
    if (!fgets(line_buffer, sizeof(line_buffer), file)) { /* erro */ }
    rest = line_buffer;
    while ((token = strtok_r(rest, " \n", &rest))) {
        machine->num_output_symbols++;
        machine->output_alphabet = (char**)realloc(machine->output_alphabet, machine->num_output_symbols * sizeof(char*));
        machine->output_alphabet[machine->num_output_symbols - 1] = strdup(token);
    }

    // 6. Ler Transições (delta) [cite: 119-122]
    char q_i[100], x[10], r_j[100], y[100];
    while (fgets(line_buffer, sizeof(line_buffer), file)) {
        if (sscanf(line_buffer, "%s %s %s %s", q_i, x, r_j, y) != 4) {
            continue; // Linha mal formatada ou vazia
        }

        int current_idx = find_state_index(machine, q_i);
        int next_idx = find_state_index(machine, r_j);
        if (current_idx == -1 || next_idx == -1) {
            fprintf(stderr, "Aviso: Transição inválida ignorada (%s -> %s)\n", q_i, r_j);
            continue;
        }

        machine->num_transitions++;
        machine->transitions = (Transition*)realloc(machine->transitions, machine->num_transitions * sizeof(Transition));
        
        Transition* new_trans = &machine->transitions[machine->num_transitions - 1];
        new_trans->current_state = current_idx;
        new_trans->input_symbol = x[0]; // Assume que input é um único char
        new_trans->next_state = next_idx;

        // Trata saídas especiais: "\n" e "epsilon"
        if (strcmp(y, "\\n") == 0) {
            new_trans->output_string = strdup("\n");
        }else if (strcmp(y, "epsilon") == 0) {
            new_trans->output_string = strdup("");
        }else if (strcmp(y, "0\\n") == 0) {
            new_trans->output_string = strdup("0\n");
        }else if (strcmp(y, "1\\n") == 0) {
            new_trans->output_string = strdup("1\n");
        }else {
            new_trans->output_string = strdup(y);
        }
    }

    fclose(file);
    return machine;
}

/**
 * @brief Libera toda a memória alocada para uma Máquina de Mealy.
 */
static inline void free_mealy_machine(MealyMachine* machine) {
    if (!machine) return;

    for (int i = 0; i < machine->num_states; i++) {
        free(machine->states[i]);
    }
    free(machine->states);

    for (int i = 0; i < machine->num_output_symbols; i++) {
        free(machine->output_alphabet[i]);
    }
    free(machine->output_alphabet);

    for (int i = 0; i < machine->num_transitions; i++) {
        free(machine->transitions[i].output_string);
    }
    free(machine->transitions);

    free(machine->final_states);
    free(machine->input_alphabet);
    free(machine);
}

/**
 * @brief Simula a Máquina de Mealy com uma palavra de entrada.
 * @return Uma nova string alocada dinamicamente com a saída completa.
 * O chamador deve liberar esta string.
 */
static inline char* simulate_mealy_machine(MealyMachine* machine, const char* input_word) {
    int current_state = machine->start_state;
    
    // Aloca um buffer de saída dinâmico
    size_t output_cap = 4096;
    size_t output_len = 0;
    char* output_buffer = (char*)malloc(output_cap);
    if (!output_buffer) {
        fprintf(stderr, "Erro ao alocar buffer de saída\n");
        return NULL;
    }
    output_buffer[0] = '\0';

    // Processa a palavra de entrada, símbolo por símbolo [cite: 55]
    for (int i = 0; input_word[i] != '\0'; i++) {
        char current_char = input_word[i];
        
        const Transition* trans = find_transition(machine, current_state, current_char);
        if (trans == NULL) {
            fprintf(stderr, "Erro: Transição não definida para o estado %s e entrada '%c'\n", 
                    machine->states[current_state], current_char);
            free(output_buffer);
            return NULL;
        }

        const char* output_str = trans->output_string;
        size_t str_len = strlen(output_str);

        // Realoca o buffer de saída se necessário
        if (output_len + str_len + 1 > output_cap) {
            output_cap = (output_len + str_len + 1) * 2;
            char* new_buffer = (char*)realloc(output_buffer, output_cap);
            if (!new_buffer) {
                fprintf(stderr, "Erro ao realocar buffer de saída\n");
                free(output_buffer);
                return NULL;
            }
            output_buffer = new_buffer;
        }

        // Concatena a saída da transição
        strcat(output_buffer, output_str);
        output_len += str_len;
        
        // Atualiza o estado
        current_state = trans->next_state;
    }

    return output_buffer;
}


#endif // MEALY_H