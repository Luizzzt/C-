#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>

// Função para codificar dados em Base64
char* base64_encode(const unsigned char* buffer, size_t length) {
    BIO *bio, *b64;
    BUF_MEM *buffer_ptr;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); // Remove quebras de linha
    BIO_write(bio, buffer, length);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &buffer_ptr);

    char* encoded = (char*)malloc((buffer_ptr->length + 1) * sizeof(char));
    memcpy(encoded, buffer_ptr->data, buffer_ptr->length);
    encoded[buffer_ptr->length] = '\0';

    BIO_free_all(bio);
    return encoded;
}

// Função para codificar uma senha usando SHA-512 e Base64
char* codificar_senha(const char* senha) {
    unsigned char hash[SHA512_DIGEST_LENGTH];

    // Gerar hash SHA-512
    SHA512((unsigned char*)senha, strlen(senha), hash);

    // Codificar o hash em Base64
    char* senha_codificada = base64_encode(hash, SHA512_DIGEST_LENGTH);
    return senha_codificada;
}

// Função para gerar combinações recursivamente
void gerar_combinacoes(char* palavras[], int n, int profundidade, char* combinacao_atual, FILE* saida) {
    if (profundidade == 0) {
        fprintf(saida, "%s\n", combinacao_atual);
        return;
    }

    for (int i = 0; i < n; i++) {
        char nova_combinacao[1024];
        if (strlen(combinacao_atual) > 0) {
            snprintf(nova_combinacao, sizeof(nova_combinacao), "%s %s", combinacao_atual, palavras[i]);
        } else {
            snprintf(nova_combinacao, sizeof(nova_combinacao), "%s", palavras[i]);
        }
        gerar_combinacoes(palavras, n, profundidade - 1, nova_combinacao, saida);
    }
}

// Função principal
int main() {
    char buffer[256];
    int num_palavras = 0;

    // Abrir arquivos de entrada
    FILE *arquivo_palavras = fopen("palavras.txt", "r");
    FILE *arquivo_senhas = fopen("usuarios_senhacodificadas.txt", "r");

    if (!arquivo_palavras || !arquivo_senhas) {
        fprintf(stderr, "Erro ao abrir arquivos de entrada.\n");
        return 1;
    }

    // Ler palavras do arquivo
    char* palavras[1000];
    num_palavras = 0;

    while (fgets(buffer, sizeof(buffer), arquivo_palavras)) {
        buffer[strcspn(buffer, "\n")] = '\0';  // Remover nova linha
        palavras[num_palavras] = strdup(buffer);
        num_palavras++;
    }

    // Ler senhas codificadas
    char usuarios[1000][256];
    char senhas_codificadas[1000][256];
    int num_usuarios = 0;

    while (fscanf(arquivo_senhas, "%s %s", usuarios[num_usuarios], senhas_codificadas[num_usuarios]) == 2) {
        num_usuarios++;
    }

    fclose(arquivo_senhas);

    // Arquivos de saída
    FILE *arquivo_quebradas = fopen("senhas_quebradas.txt", "w");
    FILE *arquivo_nao_quebradas = fopen("senhas_nao_quebradas.txt", "w");

    if (!arquivo_quebradas || !arquivo_nao_quebradas) {
        fprintf(stderr, "Erro ao abrir arquivos de saída.\n");
        return 1;
    }

    // Gerar combinações e verificar senhas
    for (int profundidade = 1; profundidade <= 5; profundidade++) {
        FILE* combinacoes = tmpfile(); // Arquivo temporário para armazenar combinações
        gerar_combinacoes(palavras, num_palavras, profundidade, "", combinacoes);
        rewind(combinacoes);

        while (fgets(buffer, sizeof(buffer), combinacoes)) {
            buffer[strcspn(buffer, "\n")] = '\0'; // Remover nova linha
            char* senha_codificada = codificar_senha(buffer);

            for (int i = 0; i < num_usuarios; i++) {
                if (strcmp(senha_codificada, senhas_codificadas[i]) == 0) {
                    fprintf(arquivo_quebradas, "%s %s\n", usuarios[i], buffer);
                    strcpy(senhas_codificadas[i], ""); // Marcar como quebrada
                }
            }
            free(senha_codificada);
        }
        fclose(combinacoes);
    }

    // Registrar senhas não quebradas
    for (int i = 0; i < num_usuarios; i++) {
        if (strlen(senhas_codificadas[i]) > 0) {
            fprintf(arquivo_nao_quebradas, "%s\n", usuarios[i]);
        }
    }

    fclose(arquivo_quebradas);
    fclose(arquivo_nao_quebradas);

    // Liberar memória
    for (int i = 0; i < num_palavras; i++) {
        free(palavras[i]);
    }

    printf("Processamento concluído.\n");
    return 0;
}
