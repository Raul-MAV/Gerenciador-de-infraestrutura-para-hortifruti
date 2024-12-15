#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <ctype.h>
#include <stdlib.h>

#if defined(_WIN32) || defined(_WIN64)
    #include <conio.h>  // Necessário para getch() no Windows
#else
    #include <termios.h>  // Necessário para desativar a exibição de caracteres no Unix/Linux
    #include <unistd.h>
#endif

#define MAX_ITEMS 100
#define MAX_VENDEDORES 50

// Estrutura para representar um item no estoque
typedef struct {
    int codigo;
    char nome[50];
    int quantidade;
    float preco;
    char categoria[20];
} Item;

// Estrutura para representar um vendedor
typedef struct {
    char nome[50];
    char senha[50];
} Vendedor;

// Declaração das variáveis globais
Item estoque[MAX_ITEMS];
Vendedor vendedores[MAX_VENDEDORES];
int totalItems = 0;
int totalVendedores = 0;
float ganhoBrutoVendas = 0.0f;
float despesasTotais = 0.0f;

const char adminNome[] = "admin";          // Nome de usuário do administrador
const char adminSenha[] = "admin123";      // Senha do administrador
char usuarioLogado[50];                    // Armazena o usuário atualmente logado

// Códigos iniciais para categorias
int ultimoCodigoFrutas = 1001;
int ultimoCodigoLegumes = 2001;
int ultimoCodigoVerduras = 3001;
int ultimoCodigoErvas = 4001;
int ultimoCodigoRaizes = 5001;
int ultimoCodigoOutros = 6001;

// Função para desativar a exibição de caracteres no Unix/Linux
#ifndef _WIN32
void desativarExibicaoCaracteres() {
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    tty.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

// Função para reativar a exibição de caracteres no Unix/Linux
void reativarExibicaoCaracteres() {
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    tty.c_lflag |= ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}
#endif

// Função para ler a senha do usuário ocultando a entrada com '*'
void lerSenhaOculta(char *senha) {
    int i = 0;
    char ch;

#if defined(_WIN32) || defined(_WIN64)
    while ((ch = getch()) != '\r') { // '\r' para Enter
        if (ch == '\b') {  // Backspace para apagar
            if (i > 0) {
                i--;
                printf("\b \b"); // Move o cursor para trás, apaga e volta
            }
        } else if (i < 49) { // Limite da senha
            senha[i++] = ch;
            printf("*");
        }
    }
#else
    desativarExibicaoCaracteres();  // Desativa a exibição no Unix/Linux
    while ((ch = getchar()) != '\n') {  // '\n' para Enter no Unix/Linux
        if (ch == 127 || ch == '\b') {  // Backspace para apagar
            if (i > 0) {
                i--;
                printf("\b \b"); // Move o cursor para trás, apaga e volta
            }
        } else if (i < 49) { // Limite da senha
            senha[i++] = ch;
            printf("*");
        }
    }
    reativarExibicaoCaracteres();  // Reativa a exibição no Unix/Linux
#endif
    senha[i] = '\0';
}

// Função para salvar o usuário logado em um arquivo de sessão
void salvarSessao(const char *usuario) {
    FILE *file = fopen("login_status.txt", "w");
    if (file == NULL) {
        printf("Erro ao salvar a sessão.\n");
        return;
    }
    fprintf(file, "%s\n", usuario);
    fclose(file);
}

// Função para verificar e carregar o usuário logado de uma sessão anterior
int carregarSessao() {
    FILE *file = fopen("login_status.txt", "r");
    if (file != NULL) {
        fgets(usuarioLogado, sizeof(usuarioLogado), file);
        usuarioLogado[strcspn(usuarioLogado, "\n")] = '\0';  // Remove o caractere de nova linha
        fclose(file);
        printf("Sessão carregada. Usuário logado: %s\n", usuarioLogado);
        return 1;
    }
    return 0;
}

// Função para encerrar a sessão atual (logout)
void encerrarSessao() {
    if (remove("login_status.txt") == 0) {
        printf("Logout realizado com sucesso.\n");
    } else {
        printf("Erro ao encerrar a sessão.\n");
    }
}

// Função para carregar vendedores do arquivo 'vendedores.txt'
// Se o arquivo não existir, será criado quando um novo vendedor for salvo
void carregarVendedores() {
    FILE *file = fopen("vendedores.txt", "r");
    if (file == NULL) {
        printf("Nenhum vendedor cadastrado. Um novo arquivo será criado ao salvar novos vendedores.\n");
        return;
    }
    while (fscanf(file, "%49s %49s", vendedores[totalVendedores].nome, vendedores[totalVendedores].senha) == 2) {
        totalVendedores++;
    }
    fclose(file);
}

// Função para salvar vendedores no arquivo 'vendedores.txt'
void salvarVendedores() {
    FILE *file = fopen("vendedores.txt", "w");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo para salvar os vendedores.\n");
        return;
    }
    for (int i = 0; i < totalVendedores; i++) {
        fprintf(file, "%s %s\n", vendedores[i].nome, vendedores[i].senha);
    }
    fclose(file);
}

// Função para cadastrar um novo vendedor
void cadastrarVendedor() {
    if (totalVendedores >= MAX_VENDEDORES) {
        printf("Limite máximo de vendedores alcançado.\n");
        return;
    }
    Vendedor novoVendedor;
    printf("Digite o nome do novo vendedor: ");
    scanf(" %[^\n]", novoVendedor.nome);
    printf("Digite a senha de 4 dígitos do novo vendedor: ");
    scanf(" %[^\n]", novoVendedor.senha);

    vendedores[totalVendedores++] = novoVendedor;
    salvarVendedores();
    printf("Vendedor cadastrado com sucesso!\n");
}

// Função para exibir a lista de vendedores e remover um
void removerVendedor() {
    printf("\nLista de Vendedores:\n");
    for (int i = 0; i < totalVendedores; i++) {
        printf("%d. %s\n", i + 1, vendedores[i].nome);
    }

    char nome[50];
    printf("Digite o nome do vendedor que deseja remover: ");
    scanf(" %[^\n]", nome);

    for (int i = 0; i < totalVendedores; i++) {
        if (strcmp(vendedores[i].nome, nome) == 0) {
            for (int j = i; j < totalVendedores - 1; j++) {
                vendedores[j] = vendedores[j + 1];
            }
            totalVendedores--;
            salvarVendedores();
            printf("Vendedor removido com sucesso!\n");
            return;
        }
    }
    printf("Vendedor não encontrado.\n");
}

// Função de login e início de expediente
int iniciarExpediente() {
    char nome[50];
    char senha[50];
    int escolha;

    // Verifica se há uma sessão ativa
    if (carregarSessao()) {
        return strcmp(usuarioLogado, adminNome) == 0 ? 2 : 1;  // Retorna 2 se o admin estiver logado, 1 caso contrário
    }

    while (1) {
        printf("\nBem-vindo! Se você é um novo vendedor, escolha '1' para se cadastrar ou '2' para login.\n");
        printf("1. Cadastrar novo vendedor\n");
        printf("2. Login\n");
        printf("3. Sair\n");

        scanf("%d", &escolha);
        getchar(); // Limpa o buffer

        if (escolha == 1) {
            cadastrarVendedor();
            return 1;
        } else if (escolha == 2) {
            printf("Digite o nome de usuário: ");
            scanf(" %[^\n]", nome);
            printf("Digite a senha: ");
            lerSenhaOculta(senha);  // Oculta a entrada da senha
            printf("\n");

            // Verifica se é o administrador
            if (strcmp(nome, adminNome) == 0 && strcmp(senha, adminSenha) == 0) {
                strcpy(usuarioLogado, nome);
                salvarSessao(usuarioLogado);
                printf("Login realizado com sucesso. Bem-vindo(a), admin!\n");
                return 2;
            }

            // Verifica se é um vendedor
            for (int i = 0; i < totalVendedores; i++) {
                if (strcmp(nome, vendedores[i].nome) == 0 && strcmp(senha, vendedores[i].senha) == 0) {
                    strcpy(usuarioLogado, nome);
                    salvarSessao(usuarioLogado);
                    printf("Login realizado com sucesso. Bem-vindo(a), %s!\n", nome);
                    return 1;
                }
            }
            printf("Nome de usuário ou senha incorretos. Tente novamente.\n");
        } else if (escolha == 3) {
            return 0;
        } else {
            printf("Opção inválida. Tente novamente.\n");
        }
    }
}

// Função para registrar despesas de produtos e do estabelecimento
void registrarDespesas() {
    float valorProduto, despesasGerais;

    printf("\nRegistro de Despesas\n");
    printf("Digite o valor gasto na compra de produtos para revenda: R$ ");
    scanf("%f", &valorProduto);

    printf("Digite o valor das despesas gerais do estabelecimento (aluguel, luz, etc.): R$ ");
    scanf("%f", &despesasGerais);

    despesasTotais += (valorProduto + despesasGerais);

    // Verifica se 'financeiro.csv' existe; se não, cria com cabeçalho
    FILE *file = fopen("financeiro.csv", "r");
    if (file == NULL) {
        file = fopen("financeiro.csv", "w");
        if (file == NULL) {
            printf("Erro ao criar o arquivo financeiro.\n");
            return;
        }
        fprintf(file, "Tipo,Valor\n"); // Cabeçalho
    }
    fclose(file);

    // Salva os registros em 'financeiro.csv'
    file = fopen("financeiro.csv", "a");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo financeiro.\n");
        return;
    }

    fprintf(file, "Compra de produtos,%.2f\n", valorProduto);
    fprintf(file, "Despesas gerais,%.2f\n", despesasGerais);
    fclose(file);

    printf("Despesas registradas com sucesso. Total de despesas: R$%.2f\n", despesasTotais);
}

// Função para exibir o conteúdo do estoque a partir do CSV com categorias por extenso
void listarItens() {
    FILE *file = fopen("estoque.csv", "r");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo de estoque.\n");
        return;
    }

    char linha[1024];
    printf("\nItens no estoque:\n");
    printf("---------------------------------------------------------------------\n");
    printf("| Código | Nome                     | Quantidade | Preço    | Categoria     |\n");
    printf("---------------------------------------------------------------------\n");

    // Ignora a primeira linha (cabeçalho)
    fgets(linha, sizeof(linha), file);

    while (fgets(linha, sizeof(linha), file)) {
        char *codigo = strtok(linha, ",");
        char *nome = strtok(NULL, ",");
        char *quantidade = strtok(NULL, ",");
        char *preco = strtok(NULL, ",");
        char *categoriaCodigo = strtok(NULL, ",\n");

        if (codigo == NULL || nome == NULL || quantidade == NULL || preco == NULL || categoriaCodigo == NULL) {
            continue; // Ignora linhas mal formatadas
        }

        int codigoProduto = atoi(codigo);
        char categoriaExtenso[20];

        // Mapeia o código para a categoria correspondente
        if (codigoProduto >= 1001 && codigoProduto < 2000) {
            strcpy(categoriaExtenso, "Fruta");
        } else if (codigoProduto >= 2001 && codigoProduto < 3000) {
            strcpy(categoriaExtenso, "Legume");
        } else if (codigoProduto >= 3001 && codigoProduto < 4000) {
            strcpy(categoriaExtenso, "Verdura");
        } else if (codigoProduto >= 4001 && codigoProduto < 5000) {
            strcpy(categoriaExtenso, "Erva");
        } else if (codigoProduto >= 5001 && codigoProduto < 6000) {
            strcpy(categoriaExtenso, "Raiz");
        } else if (codigoProduto >= 6001 && codigoProduto < 7000) {
            strcpy(categoriaExtenso, "Outro");
        } else {
            strcpy(categoriaExtenso, "Desconhecida");
        }

        printf("| %-6s | %-25s | %-10s | %-8s | %-12s |\n", codigo, nome, quantidade, preco, categoriaExtenso);
    }

    printf("---------------------------------------------------------------------\n");
    fclose(file);
}

// Função para carregar estoque, criando o arquivo se ele não existir
void carregarEstoque() {
    FILE *file = fopen("estoque.csv", "r");
    if (file == NULL) {
        printf("Arquivo de estoque não encontrado. Criando um novo arquivo...\n");
        file = fopen("estoque.csv", "w");
        if (file == NULL) {
            printf("Erro ao criar o arquivo de estoque.\n");
            return;
        }
        fprintf(file, "Código,Nome,Quantidade,Preço,Categoria\n"); // Cabeçalho
        fclose(file);
        printf("Arquivo de estoque criado com sucesso.\n");
    } else {
        char linha[256];
        fgets(linha, sizeof(linha), file); // Ignora a primeira linha (cabeçalho)

        while (fgets(linha, sizeof(linha), file)) {
            Item novoItem;
            // Lê os dados do arquivo e preenche a estrutura Item
            sscanf(linha, "%d,%49[^,],%d,%f,%19[^,\n]", &novoItem.codigo, novoItem.nome, &novoItem.quantidade, &novoItem.preco, novoItem.categoria);
            estoque[totalItems++] = novoItem;
        }

        fclose(file);
        printf("Estoque carregado com sucesso. Total de itens: %d\n", totalItems);
    }
}

// Função para salvar as mudanças no estoque no arquivo 'estoque.csv'
void salvarMudancas() {
    FILE *file = fopen("estoque.csv", "w");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo para salvar.\n");
        return;
    }
    fprintf(file, "Código,Nome,Quantidade,Preço,Categoria\n"); // Cabeçalho
    for (int i = 0; i < totalItems; i++) {
        fprintf(file, "%d,%s,%d,%.2f,%s\n",
                estoque[i].codigo, estoque[i].nome, estoque[i].quantidade, estoque[i].preco, estoque[i].categoria);
    }
    fclose(file);
    printf("Mudanças salvas automaticamente no arquivo 'estoque.csv'.\n");
}

// Função para adicionar item ao estoque com incremento na quantidade se já existir
void adicionarItem() {
    if (totalItems >= MAX_ITEMS) {
        printf("Estoque cheio! Não é possível adicionar mais itens.\n");
        return;
    }

    Item novoItem;

    printf("Digite o nome do item: ");
    scanf(" %[^\n]", novoItem.nome);
    printf("Quantidade: ");
    scanf("%d", &novoItem.quantidade);
    printf("Preço: ");
    scanf("%f", &novoItem.preco);

    int categoriaValida = 0;
    while (!categoriaValida) {
        printf("Categoria (fruta, legume, verdura, erva, raiz, outro): ");
        scanf(" %[^\n]", novoItem.categoria);

        if (strcmp(novoItem.categoria, "fruta") == 0) {
            novoItem.codigo = ultimoCodigoFrutas++;
            categoriaValida = 1;
        } else if (strcmp(novoItem.categoria, "legume") == 0) {
            novoItem.codigo = ultimoCodigoLegumes++;
            categoriaValida = 1;
        } else if (strcmp(novoItem.categoria, "verdura") == 0) {
            novoItem.codigo = ultimoCodigoVerduras++;
            categoriaValida = 1;
        } else if (strcmp(novoItem.categoria, "erva") == 0) {
            novoItem.codigo = ultimoCodigoErvas++;
            categoriaValida = 1;
        } else if (strcmp(novoItem.categoria, "raiz") == 0) {
            novoItem.codigo = ultimoCodigoRaizes++;
            categoriaValida = 1;
        } else if (strcmp(novoItem.categoria, "outro") == 0) {
            novoItem.codigo = ultimoCodigoOutros++;
            categoriaValida = 1;
        } else  {
            printf("Categoria inválida. Tente novamente.\n");
        }
    }

    // Verifica se o item já existe no estoque
    for (int i = 0; i < totalItems; i++) {
        if (estoque[i].codigo == novoItem.codigo && strcmp(estoque[i].nome, novoItem.nome) == 0) {
            estoque[i].quantidade += novoItem.quantidade;
            salvarMudancas();
            printf("Item atualizado com sucesso! Quantidade atualizada: %d\n", estoque[i].quantidade);
            return;
        }
    }

    // Adiciona novo item ao estoque
    estoque[totalItems++] = novoItem;
    salvarMudancas();
    printf("Item adicionado com sucesso! Código atribuído: %d\n", novoItem.codigo);
}

// Função para remover um item do estoque
void removerItem() {
    int codigo;
    printf("Digite o código do item que deseja remover: ");
    scanf("%d", &codigo);

    for (int i = 0; i < totalItems; i++) {
        if (estoque[i].codigo == codigo) {
            for (int j = i; j < totalItems - 1; j++) {
                estoque[j] = estoque[j + 1];
            }
            totalItems--;
            salvarMudancas();
            printf("Item removido com sucesso!\n");
            return;
        }
    }
    printf("Item não encontrado.\n");
}

// Função para calcular uma venda
void calcularVenda() {
    int codigo, quantidade;
    float totalVenda = 0.0, pagamento, troco;
    int formaPagamento, parcelas;
    char nomeDevedor[50], cpfDevedor[12];

    while (1) {
        printf("Digite o código do item que deseja vender (ou 0 para finalizar): ");
        scanf("%d", &codigo);

        if (codigo == 0) {
            break;
        }

        printf("Quantidade: ");
        scanf("%d", &quantidade);

        int itemEncontrado = 0;
        for (int i = 0; i < totalItems; i++) {
            if (estoque[i].codigo == codigo) {
                itemEncontrado = 1;
                if (quantidade > estoque[i].quantidade) {
                    printf("Quantidade solicitada maior que a disponível no estoque para %s.\n", estoque[i].nome);
                } else {
                    float valorItem = quantidade * estoque[i].preco;
                    totalVenda += valorItem;
                    estoque[i].quantidade -= quantidade;
                    printf("Item %s adicionado ao carrinho. Subtotal: R$%.2f\n", estoque[i].nome, valorItem);
                }
                break;
            }
        }

        if (!itemEncontrado) {
            printf("Item com código %d não encontrado no estoque.\n", codigo);
        }
    }

    if (totalVenda == 0) {
        printf("Nenhum item foi adicionado ao carrinho.\n");
        return;
    }

    printf("Total da venda: R$%.2f\n", totalVenda);

    // Confirmação da finalização da venda
    printf("Deseja finalizar a venda? (1 para Sim, 0 para Não): ");
    int finalizar;
    scanf("%d", &finalizar);

    if (!finalizar) {
        printf("Venda cancelada.\n");
        return;
    }

    printf("Forma de pagamento:\n1. Dinheiro\n2. Cartão de Débito\n3. Cartão de Crédito\nEscolha uma opção: ");
    scanf("%d", &formaPagamento);

    if (formaPagamento == 1) {
        printf("Valor recebido: ");
        scanf("%f", &pagamento);
        if (pagamento < totalVenda) {
            printf("Pagamento insuficiente. Operação cancelada.\n");
            return;
        }
        troco = pagamento - totalVenda;
        printf("Troco: R$%.2f\n", troco);
    } else if (formaPagamento == 2) {
        printf("Pagamento realizado com cartão de débito.\n");
    } else if (formaPagamento == 3) {
        printf("Deseja parcelar?\n1. Não\n2. Sim\nEscolha uma opção: ");
        int opcaoParcelamento;
        scanf("%d", &opcaoParcelamento);

        if (opcaoParcelamento == 2) {
            printf("Nome do devedor: ");
            scanf(" %[^\n]", nomeDevedor);
            printf("CPF do devedor (somente números): ");
            scanf(" %[^\n]", cpfDevedor);
            printf("Número de parcelas: ");
            scanf("%d", &parcelas);

            float valorParcela = totalVenda / parcelas;
            printf("Valor por parcela: R$%.2f\n", valorParcela);
        } else {
            printf("Pagamento realizado com cartão de crédito à vista.\n");
        }
    } else {
        printf("Forma de pagamento inválida.\n");
        return;
    }

    ganhoBrutoVendas += totalVenda;
    salvarMudancas();

    // Verifica se 'financeiro.csv' existe; se não, cria com cabeçalho
    FILE *file = fopen("financeiro.csv", "r");
    if (file == NULL) {
        file = fopen("financeiro.csv", "w");
        if (file == NULL) {
            printf("Erro ao criar o arquivo financeiro.\n");
            return;
        }
        fprintf(file, "Tipo,Valor\n"); // Cabeçalho
    }
    fclose(file);

    // Registrar venda no arquivo financeiro
    file = fopen("financeiro.csv", "a");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo financeiro.\n");
        return;
    }
    fprintf(file, "Venda Bruta,%.2f\n", totalVenda);
    fclose(file);

    printf("Venda realizada com sucesso! Total da venda: R$%.2f\n", totalVenda);
}

// Função para calcular lucro líquido (apenas para admin)
void calcularLucroLiquido() {
    float lucroLiquido = ganhoBrutoVendas - despesasTotais;

    printf("\nResumo Financeiro:\n");
    printf("Ganho Bruto com Vendas: R$%.2f\n", ganhoBrutoVendas);
    printf("Despesas Totais: R$%.2f\n", despesasTotais);
    printf("Lucro Líquido: R$%.2f\n", lucroLiquido);

    // Verifica se 'financeiro.csv' existe; se não, cria com cabeçalho
    FILE *file = fopen("financeiro.csv", "r");
    if (file == NULL) {
        file = fopen("financeiro.csv", "w");
        if (file == NULL) {
            printf("Erro ao criar o arquivo financeiro.\n");
            return;
        }
        fprintf(file, "Tipo,Valor\n"); // Cabeçalho
    }
    fclose(file);

    // Grava o lucro líquido no arquivo financeiro
    file = fopen("financeiro.csv", "a");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo financeiro.\n");
        return;
    }
    fprintf(file, "Lucro Líquido,%.2f\n", lucroLiquido);
    fclose(file);
}

// Função principal do programa
int main() {
    setlocale(LC_ALL, "Portuguese");
    int opcao;

    // Carrega os vendedores existentes
    carregarVendedores();

    // Inicia o expediente (login)
    int acesso = iniciarExpediente();
    if (acesso == 0) {
        printf("Encerrando o programa.\n");
        return 0;
    }

    // Carrega o estoque existente
    carregarEstoque();
    printf("Aviso: Digite '0' em qualquer entrada para voltar ao menu principal.\n");

    // Menu principal
    do {
        printf("\nControle de Estoque - Hortifruti\n");
        printf("1. Adicionar Item\n");
        printf("2. Listar Itens\n");
        printf("3. Calcular Venda\n");
        printf("4. Remover Item\n");
        if (acesso == 2) { // Opções exclusivas para o administrador
            printf("5. Remover Vendedor\n");
            printf("6. Registrar Despesas\n");
            printf("7. Calcular Lucro Líquido\n");
        }
        printf("8. Sair sem deslogar\n");
        printf("9. Sair e deslogar\n");
        printf("Escolha uma opcao: ");
        scanf("%d", &opcao);

        switch (opcao) {
            case 1:
                adicionarItem();
                break;
            case 2:
                listarItens();
                break;
            case 3:
                calcularVenda();
                break;
            case 4:
                removerItem();
                break;
            case 5:
                if (acesso == 2) {
                    removerVendedor();
                } else {
                    printf("Opção inválida.\n");
                }
                break;
            case 6:
                if (acesso == 2) {
                    registrarDespesas();
                } else {
                    printf("Acesso negado. Apenas o admin pode acessar esta função.\n");
                }
                break;
            case 7:
                if (acesso == 2) {
                    calcularLucroLiquido();
                } else {
                    printf("Acesso negado. Apenas o admin pode acessar esta função.\n");
                }
                break;
            case 8:
                printf("Saindo do programa sem deslogar...\n");
                return 0;
            case 9:
                printf("Saindo do programa e deslogando...\n");
                encerrarSessao();  // Realiza logout ao sair
                return 0;
            default:
                printf("Opção inválida. Tente novamente.\n");
        }
    } while (1);

    return 0;
}
