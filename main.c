#include "mealy.h"

/**
 * @brief Ponto de entrada principal para o Simulador de Máquina de Mealy.
 *
 * Argumentos da linha de comando:
 * argv[1]: Caminho para o arquivo de definição da máquina (ex: MM.txt)
 * argv[2]: Caminho para o arquivo da palavra de entrada (ex: w.txt)
 *
 * A saída é impressa no stdout no formato de imagem .ppm.
 */
int main(int argc, char* argv[]) {

    // 1. Validar argumentos da linha de comando [cite: 101]
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <MM.txt> <w.txt>\n", argv[0]);
        return 1;
    }

    const char* mm_filename = argv[1];
    const char* word_filename = argv[2];

    // 2. Carregar a Máquina de Mealy
    MealyMachine* machine = load_mealy_machine(mm_filename);
    if (machine == NULL) {
        fprintf(stderr, "Falha ao carregar a máquina de Mealy do arquivo: %s\n", mm_filename);
        return 1;
    }

    // 3. Carregar a palavra de entrada
    char* input_word = read_file_to_string(word_filename);
    if (input_word == NULL) {
        fprintf(stderr, "Falha ao ler a palavra de entrada do arquivo: %s\n", word_filename);
        free_mealy_machine(machine);
        return 1;
    }

    // 4. Simular a máquina
    // A 'binary_matrix' é a saída pura da máquina de Mealy [cite: 125]
    char* binary_matrix = simulate_mealy_machine(machine, input_word);
    if (binary_matrix == NULL) {
        fprintf(stderr, "Falha durante a simulação.\n");
        free(input_word);
        free_mealy_machine(machine);
        return 1;
    }

    // 5. Determinar dimensões da grade (n x n)
    // Encontra o comprimento 'l' do primeiro número da célula [cite: 51]
    int l = 0;
    while (input_word[l] != '.' && input_word[l] != '\0') {
        l++;
    }

    // Calcula n = 2^l [cite: 53]
    int n = (int)pow(2, l);
    if (l == 0) n = 0; // Trata caso de palavra de entrada vazia

    // 6. Imprimir a saída final no formato PPM [cite: 71-73]
    // Header P1 (preto e branco)
    printf("P1\n");
    // Dimensões (largura e altura)
    printf("%d %d\n", n, n);
    // Matriz binária (a saída da máquina)
    printf("%s", binary_matrix);


    // 7. Limpar memória
    free(binary_matrix);
    free(input_word);
    free_mealy_machine(machine);

    return 0;
}