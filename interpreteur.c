#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Définition des types de tokens possibles
typedef enum {
    TOKEN_NUMBER,       // Un nombre
    TOKEN_PLUS,         // Opérateur '+'
    TOKEN_MINUS,        // Opérateur '-'
    TOKEN_MULT,         // Opérateur '*'
    TOKEN_DIV,          // Opérateur '/'
    TOKEN_LPAREN,       // Parenthèse gauche '('
    TOKEN_RPAREN,       // Parenthèse droite ')'
    TOKEN_EQUAL,        // Symbole '='
    TOKEN_PRINT,        // Mot-clé 'print'
    TOKEN_VARIABLE,     // Une variable
    TOKEN_END           // Fin de l'entrée
} TokenType;

// Structure représentant un token
typedef struct {
    TokenType type;     // Type du token
    double value;       // Valeur numérique si applicable
    char name[100];     // Nom de la variable si applicable
} Token;

// Structure pour la liste chaînée de tokens
typedef struct TokenNode {
    Token token;                // Le token actuel
    struct TokenNode *next;     // Pointeur vers le prochain token
} TokenNode;

// Structure pour les noeuds de l'AST
typedef struct ASTNode {
    Token token;                // Le token associé au noeud
    struct ASTNode *left;       // Enfant gauche sous arbre gauche
    struct ASTNode *right;      // Enfant droit sous arbre droite
} ASTNode;

// Structure pour stocker les variables et leurs valeurs
typedef struct VarNode {
    char name[100];             // Nom de la variable
    double value;               // Valeur de la variable
    struct VarNode *next;       // Pointeur vers la prochaine variable
} VarNode;

VarNode *variables = NULL;      // Liste chaînée des variables mis a null car c'est le dernier de la liste 

// Fonction pour vérifier si un caractère est un chiffre ou un point
int is_number(char c) {
    return (c >= '0' && c <= '9') || c == '.';
}

// Fonction pour vérifier si un caractère est une lettre
int is_letter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

// Fonction pour obtenir le prochain token à partir de l'entrée
Token get_next_token(char **input) {
    Token token;
    char *curr = *input;

    // Ignorer les espaces
    while (*curr == ' ') curr++;

    if (is_number(*curr)) {
        // Si c'est un nombre
        token.type = TOKEN_NUMBER;
        token.value = strtod(curr, &curr); // Convertir la chaîne en double
    } else if (*curr == '+') {
        // Opérateur '+'
        token.type = TOKEN_PLUS;
        curr++;
    } else if (*curr == '-') {
        // Opérateur '-'
        token.type = TOKEN_MINUS;
        curr++;
    } else if (*curr == '*') {
        // Opérateur '*'
        token.type = TOKEN_MULT;
        curr++;
    } else if (*curr == '/') {
        // Opérateur '/'
        token.type = TOKEN_DIV;
        curr++;
    } else if (*curr == '(') {
        // Parenthèse gauche
        token.type = TOKEN_LPAREN;
        curr++;
    } else if (*curr == ')') {
        // Parenthèse droite
        token.type = TOKEN_RPAREN;
        curr++;
    } else if (*curr == '=') {
        // Symbole '='
        token.type = TOKEN_EQUAL;
        curr++;
    } else if (is_letter(*curr)) {
        // Identifier les variables et le mot-clé 'print'
        int i = 0;
        while (is_letter(*curr)) {
            token.name[i++] = *curr++; // Construire le nom de la variable ou mot-clé print
        }
        token.name[i] = '\0'; // Terminer la chaîne
        if (strcmp(token.name, "print") == 0) {
            token.type = TOKEN_PRINT; // Mot-clé 'print'
        } else {
            token.type = TOKEN_VARIABLE; // Nom de variable
        }
    } else if (*curr == '\0' || *curr == '\n') {
        // Fin de l'entrée
        token.type = TOKEN_END;
    } else {
        // Caractère inconnu, ignorer
        curr++;
        token.type = TOKEN_END;
    }

    *input = curr; // mis à jour pour refléter la position actuelle après le dernier token extrait.
    return token;
}

// Fonction pour transformer l'entrée en une liste de tokens
TokenNode* tokenize(char *input) {
    TokenNode *head = NULL;
    TokenNode *tail = NULL;
    Token token;

    while (1) {
        token = get_next_token(&input); //Après avoir extrait un token, get_next_token avance le pointeur input pour pointer vers le prochain caractère à analyser.
        // Créer un nouveau noeud pour le token
        TokenNode *node = (TokenNode*)malloc(sizeof(TokenNode));
        node->token = token;//Les données du token (type, valeur ou nom, etc.) sont copiées dans ce nœud
        node->next = NULL;//Le nouveau nœud est initialisé pour qu'il puisse être ajouté à la liste.

        if (head == NULL) {
            // Initialiser la tête de la liste
            head = node;
            tail = node;
        } else {
            // Ajouter à la fin de la liste
            tail->next = node; //Le champ next du nœud pointé par tail est mis à jour pour pointer vers le nouveau nœud (node).Cela relie le nouveau nœud à la fin de la liste chaînée.A -> B -> C -> node

            tail = node;//Le pointeur tail est mis à jour pour pointer vers le nouveau nœud node, qui est maintenant le dernier nœud de la liste.

        }

        if (token.type == TOKEN_END) {
            // Fin de l'entrée
            break;
        }
    }

    return head;
}

// Prototypes des fonctions de parsing
ASTNode* expression(TokenNode** current_token);

// Fonction pour analyser un facteur (nombre, variable ou expression entre parenthèses)
ASTNode* factor(TokenNode** current_token) {
    Token token = (*current_token)->token;
    ASTNode *node = NULL;//prépare le pointeur node pour la création éventuelle d’un nœud d’arbre syntaxique ou pour détecter une absence de nœud en cas d’erreur.

    if (token.type == TOKEN_NUMBER) {
        // Créer un noeud pour un nombre
        node = (ASTNode*)malloc(sizeof(ASTNode));
        node->token = token; //stocker le token
        node->left = NULL;
        node->right = NULL;//NULL car un nombre est une feuille dans l'arbre syntaxique.
        *current_token = (*current_token)->next;//avance au prochain  token suivant
    } else if (token.type == TOKEN_VARIABLE) {
        // Créer un noeud pour une variable
        node = (ASTNode*)malloc(sizeof(ASTNode));
        node->token = token;
        node->left = NULL;
        node->right = NULL;
        *current_token = (*current_token)->next;
    } else if (token.type == TOKEN_LPAREN) {
        // Gérer les expressions entre parenthèses
        *current_token = (*current_token)->next;
        node = expression(current_token);//analyser les termes de l'expression jusqu'à ce qu'elle rencontre un opérateur ou un autre token de fin
        if ((*current_token)->token.type == TOKEN_RPAREN) {//vérifier si nous avons atteint une parenthèse droite
            *current_token = (*current_token)->next;//on passe a l'element nsuivant si on trouve la parenthese droite
        } else {
            printf("Erreur: ')' attendu\n");
        }
    } else {
        printf("Erreur: Token inattendu\n");//
        *current_token = (*current_token)->next;
    }

    return node;//retourne le nœud correspondant au facteur analysé.
}

ASTNode* term(TokenNode** current_token) {
    ASTNode *node = factor(current_token);  // Analyse le premier facteur

    while ((*current_token)->token.type == TOKEN_MULT || (*current_token)->token.type == TOKEN_DIV) {
        Token token = (*current_token)->token;  // Récupère le token (opérateur '*' ou '/')
        ASTNode *new_node = (ASTNode*)malloc(sizeof(ASTNode));  // Alloue de la mémoire pour un nouveau nœud
        new_node->token = token;  // Le token actuel (opérateur) devient le token du nœud
        new_node->left = node;  // Le nœud actuel devient le sous-arbre gauche du nouveau nœud
        *current_token = (*current_token)->next;  // Avance au token suivant
        new_node->right = factor(current_token);  // Le facteur suivant devient le sous-arbre droit du nouveau nœud
        node = new_node;  // Le nouveau nœud devient le nœud actuel
    }

    return node;  // Retourne le nœud racine du sous-arbre pour cette expression de terme
}


// Fonction pour analyser une expression (gère '+' et '-')
ASTNode* expression(TokenNode** current_token) {
    ASTNode *node = term(current_token);  // Analyser le premier terme (peut-être une multiplication ou une division)

    while ((*current_token)->token.type == TOKEN_PLUS || (*current_token)->token.type == TOKEN_MINUS) {
        // Vérifier si l'opérateur suivant est '+' ou '-'
        Token token = (*current_token)->token;  // Récupérer l'opérateur + -
        ASTNode *new_node = (ASTNode*)malloc(sizeof(ASTNode));  // Créer un nouveau nœud pour cet opérateur

        new_node->token = token;  // Le nœud porte l'opérateur
        new_node->left = node;    // Le nœud gauche est l'expression existante
        *current_token = (*current_token)->next;  // Avancer au token suivant

        new_node->right = term(current_token);  // Le nœud droit est un autre terme analysé
        node = new_node;  // Mettre à jour le nœud courant
    }

    return node;  // Retourner le nœud racine de l'AST de l'expression
}


// Fonction pour analyser une assignation (variable = expression)
ASTNode* assignment(TokenNode** current_token) {
    ASTNode *node = NULL;

    if ((*current_token)->token.type == TOKEN_VARIABLE) {
        Token var_token = (*current_token)->token; //Si le token courant est bien une variable, on récupère ce token 
        *current_token = (*current_token)->next;

        if ((*current_token)->token.type == TOKEN_EQUAL) {
            *current_token = (*current_token)->next;
            ASTNode *expr_node = expression(current_token);//pour analyser l'expression qui se trouve a roite de l'assignation
            node = (ASTNode*)malloc(sizeof(ASTNode));//alloue de la mémoire dynamique pour un nouveau nœud de type ASTNode
            node->token = var_token;
            node->left = expr_node;
            node->right = NULL;
        } else {
            printf("Erreur: '=' attendu\n");
        }
    } else {
        // Si ce n'est pas une assignation, traiter comme une expression
        node = expression(current_token);
    }

    return node;
}

// Fonction pour évaluer l'AST
double evaluate(ASTNode *node) {
    if (node == NULL) return 0;//si on atteint un noeud vide

    if (node->token.type == TOKEN_NUMBER) {
        // Retourner la valeur numérique
        return node->token.value;
    } else if (node->token.type == TOKEN_VARIABLE) {
        // Chercher la valeur de la variable
        VarNode *var = variables;
        while (var != NULL) {
            if (strcmp(var->name, node->token.name) == 0) {//comparaison le nom de la variable stockée dans var (var->name) avec le nom de la variable dans l'AST
                return var->value;
            }
            var = var->next;
        }
        printf("Erreur: Variable non définie %s\n", node->token.name);
        return 0;
    } else if (node->token.type == TOKEN_PLUS) {
        // Opération '+'
        return evaluate(node->left) + evaluate(node->right);//évalue les sous-arbres gauche et droit et retourne la somme des résultats.
    } else if (node->token.type == TOKEN_MINUS) {
        // Opération '-'
        return evaluate(node->left) - evaluate(node->right);
    } else if (node->token.type == TOKEN_MULT) {
        // Opération '*'
        return evaluate(node->left) * evaluate(node->right);
    } else if (node->token.type == TOKEN_DIV) {
        // Opération '/'
        return evaluate(node->left) / evaluate(node->right);
    } else {
        printf("Erreur: Type de noeud inconnu\n");
        return 0;
    }
}

// Fonction pour assigner une valeur à une variable
void assign_variable(char *name, double value) {
    VarNode *var = variables;

    while (var != NULL) {
        if (strcmp(var->name, name) == 0) {//verifier si ils sont identifique
            var->value = value; // Mettre à jour la valeur existante
            return;
        }
        var = var->next;
    }

    // Créer une nouvelle variable si elle n'existe pas
    VarNode *new_var = (VarNode*)malloc(sizeof(VarNode));
    strcpy(new_var->name, name);//On copie le nom de la variable passée en argument dans le champ name du nouveau nœud.
    new_var->value = value;
    new_var->next = variables;//ajouter cette nouvelle variable au début de la liste chaînée
    variables = new_var;//mis a jour variables pour que ce soit le nouveau nœud qui devienne la première variable de la liste.
}

// Fonction pour libérer la mémoire allouée à l'AST
void free_ast(ASTNode *node) {
    if (node == NULL) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

// Fonction pour interpréter une ligne d'entrée
void interpret(char *input_line) {
    TokenNode *tokens = tokenize(input_line); // Tokeniser l'entrée
    TokenNode *current_token = tokens;//current_token est un pointeur qui permet de parcourir les tokens un à un au fur et à mesure que l'on les traite.

    if (current_token->token.type == TOKEN_PRINT) {
        // Gérer la fonction 'print'
        current_token = current_token->next;
        ASTNode *expr = expression(&current_token);//pour analyser l'expression à imprimer, et elle renvoie un arbre de syntaxe abstraite (AST) représentant cette expression.
        double result = evaluate(expr);//calculer sa valeur
        printf("%f\n", result); // Afficher le résultat
        free_ast(expr);//liberer la memoire alouee a l'ast
    } else {
        // Gérer les assignations et les expressions
        ASTNode *node = assignment(&current_token);
        if (node != NULL) {
            if (node->token.type == TOKEN_VARIABLE && node->left != NULL) {
                double value = evaluate(node->left);
                assign_variable(node->token.name, value); // Assigner la valeur à la variable
            } else {
                double result = evaluate(node); // Évaluer l'expression
            }
            free_ast(node);
        }
    }

    // Libérer la mémoire des tokens ;Une fois que l'interpréteur a fini de traiter la ligne d'entrée, il libère la mémoire allouée pour la liste des tokens.
    TokenNode *temp;
    while (tokens != NULL) {
        temp = tokens;
        tokens = tokens->next;
        free(temp);
    }
}

// Fonction principale
int main(int argc, char *argv[]) {
    char input[256];

    if (argc > 1) {
        // Lire les instructions depuis un fichier
        FILE *file = fopen(argv[1], "r");
        if (file == NULL) {
            printf("Erreur d'ouverture du fichier\n");
            return 1;
        }
        while (fgets(input, sizeof(input), file) != NULL) {
            interpret(input);
        }
        fclose(file);
    } else {
        // Mode interactif (REPL)
        while (1) {
            printf(">> ");
            if (fgets(input, sizeof(input), stdin) == NULL) {
                break;
            }
            if (strcmp(input, "exit\n") == 0) {
                // Quitter le mode interactif
                break;
            }
            interpret(input);
        }
    }

    return 0;
}
