#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Les Differents type de tokens
typedef enum {
    TOKEN_NUMBER,       
    TOKEN_PLUS,         
    TOKEN_MOINS,       
    TOKEN_FOIS,         
    TOKEN_DIVISER,          
    TOKEN_PARENTHESE_GAUCHE,       
    TOKEN_PARENTHESE_DROITE,       
    TOKEN_EGAL,        
    TOKEN_PRINT,        
    TOKEN_VARIABLE,     
    TOKEN_END           
} TokenType;

// struct du token : ses attributs
typedef struct { 
    TokenType type;    
    double value;//valeur de la variable       
    char name[100];     //nom de la variable
} Token;

// liste chainée pour pointer vers le prochain token
typedef struct TokenNode {
    Token token;                
    struct TokenNode *next;     
} TokenNode;

// liste chainee pour creer l'abre de priorité
typedef struct ASTNode {
    Token token;                
    struct ASTNode *left;       
    struct ASTNode *right;      
} ASTNode;

// Stocker les variable et leurs valeurs la dedans (liste chainée)
typedef struct VarNode {
    char name[100];             
    double value;              
    struct VarNode *next;       
} VarNode;

VarNode *variables = NULL;    // Liste chaînée des variables mis a null car c'est le dernier de la liste 
 


int is_number(char c) {
    return (c >= '0' && c <= '9') || c == '.';
}


int is_letter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

// lire l'input et verifier tous les tokens dedans
Token get_next_token(char **input) {//avance le pointeur input pour pointer vers le prochain caractère à analyser.

    Token token;
    char *curr = *input;

    
    while (*curr == ' ') curr++;

    if (is_number(*curr)) {
        
        token.type = TOKEN_NUMBER;
        token.value = strtod(curr, &curr); // Convertir la chaîne en double
    } else if (*curr == '+') {
        
        token.type = TOKEN_PLUS;
        curr++;
    } else if (*curr == '-') {
        
        token.type = TOKEN_MOINS;
        curr++;
    } else if (*curr == '*') {
        
        token.type = TOKEN_FOIS;
        curr++;
    } else if (*curr == '/') {
        
        token.type = TOKEN_DIVISER;
        curr++;
    } else if (*curr == '(') {
        
        token.type = TOKEN_PARENTHESE_GAUCHE;
        curr++;
    } else if (*curr == ')') {
        
        token.type = TOKEN_PARENTHESE_DROITE;
        curr++;
    } else if (*curr == '=') {
        
        token.type = TOKEN_EGAL;
        curr++;
    } else if (is_letter(*curr)) {
         // Identifier les variables et le mot-clé 'print'
        int i = 0;
        while (is_letter(*curr)) {
            token.name[i++] = *curr++;// Construire le nom de la variable ou mot-clé print
        }
        token.name[i] = '\0'; // Terminer la chaîne
        if (strcmp(token.name, "print") == 0) {
            token.type = TOKEN_PRINT; 
        } else {
            token.type = TOKEN_VARIABLE; 
        }
    } else if (*curr == '\0' || *curr == '\n') {
        
        token.type = TOKEN_END;
    } else {
        
        curr++;
        token.type = TOKEN_END;
    }

    *input = curr;  // mis à jour pour refléter la position actuelle après le dernier token extrait.
    return token;
}

// implementer la liste chainee de token
TokenNode* tokenize(char *input) {
    TokenNode *debut = NULL;
    TokenNode *fin = NULL;//[debut,, , , fin], debut = token 1 ...
    Token token;

    while (1) {
        token = get_next_token(&input);
        
        TokenNode *node = (TokenNode*)malloc(sizeof(TokenNode)); ////Après avoir extrait un token, get_next_token avance le pointeur input
        // pour pointer vers le prochain caractère à analyser.
        node->token = token;//les donnees du token sont copies dans ce noeud
        node->next = NULL;//ajou a la liste

        if (debut == NULL) {
            // Initialiser le debut de la liste
            debut = node;
            fin = node;
        } else {
           
            fin->next = node;//cette ligne ajoute le nouveau noeud a la fin
            fin = node;//le noeud est a la fin
        }

        if (token.type == TOKEN_END) {
           
            break;
        }
    }

    return debut;
}

// Prototype de la fonction
ASTNode* somme(TokenNode** current_token);

// calculer ce qu'il y a dans les parenthese
ASTNode* parenthese_prio(TokenNode** current_token) {
    Token token = (*current_token)->token;
    ASTNode *node = NULL;

    if (token.type == TOKEN_NUMBER) {
       
        node = (ASTNode*)malloc(sizeof(ASTNode));
        node->token = token;
        node->left = NULL;
        node->right = NULL;
        *current_token = (*current_token)->next;
    } else if (token.type == TOKEN_VARIABLE) {
        
        node = (ASTNode*)malloc(sizeof(ASTNode));
        node->token = token;//stocke le token
        node->left = NULL;
        node->right = NULL;//NULL car un nombre est une feuille dans l'arbre syntaxique.
        *current_token = (*current_token)->next;
    } else if (token.type == TOKEN_PARENTHESE_GAUCHE) {
        
        *current_token = (*current_token)->next;
        node = somme(current_token);
        if ((*current_token)->token.type == TOKEN_PARENTHESE_DROITE) {
            *current_token = (*current_token)->next;
        } else {
            printf("Error: ')' expected\n");
        }
    } else {
        printf("Errorr: Token false\n");
        *current_token = (*current_token)->next;
    }

    return node;
}

// Fonction qui gere * /
ASTNode* produit(TokenNode** current_token) {
    ASTNode *node = parenthese_prio(current_token);

    while ((*current_token)->token.type == TOKEN_FOIS || (*current_token)->token.type == TOKEN_DIVISER) {
        Token token = (*current_token)->token;//recupere l'operateur * ou 
        ASTNode *new_node = (ASTNode*)malloc(sizeof(ASTNode));
        new_node->token = token;
        new_node->left = node;
        *current_token = (*current_token)->next;
        new_node->right = parenthese_prio(current_token);// Le facteur suivant devient le sous-arbre droit du nouveau nœud
        node = new_node;
    }

    return node;
}

// Fonction qui gère + et -
ASTNode* somme(TokenNode** current_token) {
    ASTNode *node = produit(current_token);

    while ((*current_token)->token.type == TOKEN_PLUS || (*current_token)->token.type == TOKEN_MOINS) {
        Token token = (*current_token)->token;
        ASTNode *new_node = (ASTNode*)malloc(sizeof(ASTNode));
        new_node->token = token;
        new_node->left = node;
        *current_token = (*current_token)->next;
        new_node->right = produit(current_token);
        node = new_node;
    }

    return node;
}

// variable = somme ? ast
ASTNode* assign(TokenNode** current_token) {
    ASTNode *node = NULL;

    if ((*current_token)->token.type == TOKEN_VARIABLE) {
        Token var_token = (*current_token)->token;
        *current_token = (*current_token)->next;

        if ((*current_token)->token.type == TOKEN_EGAL) {
            *current_token = (*current_token)->next;
            ASTNode *expr_node = somme(current_token);
            node = (ASTNode*)malloc(sizeof(ASTNode));
            node->token = var_token;
            node->left = expr_node;
            node->right = NULL;
        } else {
            printf("Error: '=' expected\n");
        }
    } else {
        
        node = somme(current_token);
    }

    return node;
}

// moi
double calculate(ASTNode *node) {
    if (node == NULL) return 0;

    if (node->token.type == TOKEN_NUMBER) {
       
        return node->token.value;
    } else if (node->token.type == TOKEN_VARIABLE) {
        
        VarNode *var = variables;//listes des var
        while (var != NULL) {
            if (strcmp(var->name, node->token.name) == 0) {//camparaison
                return var->value;
            }
            var = var->next;
        }
        printf("Error: Variable doesn't exist %s\n", node->token.name);
        return 0;
    } else if (node->token.type == TOKEN_PLUS) {
        
        return calculate(node->left) + calculate(node->right);//somme
    } else if (node->token.type == TOKEN_MOINS) {
        
        return calculate(node->left) - calculate(node->right);
    } else if (node->token.type == TOKEN_FOIS) {
        
        return calculate(node->left) * calculate(node->right);
    } else if (node->token.type == TOKEN_DIVISER) {
        
        return calculate(node->left) / calculate(node->right);
    } else {
        printf("Error: unknown node\n");
        return 0;
    }
}


void assign_variable(char *name, double value) {
    VarNode *var = variables;

    while (var != NULL) {
        if (strcmp(var->name, name) == 0) {
            var->value = value; 
            return;
        }
        var = var->next;
    }

    
    VarNode *new_var = (VarNode*)malloc(sizeof(VarNode));
    strcpy(new_var->name, name);
    new_var->value = value;
    new_var->next = variables;
    variables = new_var;
}


void free_ast(ASTNode *node) {
    if (node == NULL) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

//moi
void interpret(char *input_line) {
    TokenNode *tokens = tokenize(input_line); 
    TokenNode *current_token = tokens;

    if (current_token->token.type == TOKEN_PRINT) {
       
        current_token = current_token->next;
        ASTNode *expr = somme(&current_token);
        double result = calculate(expr);
        printf("%.2f\n", result); 
        free_ast(expr);
    } else {
        
        ASTNode *node = assign(&current_token);
        if (node != NULL) {
            if (node->token.type == TOKEN_VARIABLE && node->left != NULL) {
                double value = calculate(node->left);
                assign_variable(node->token.name, value); 
            } else {
                double result = calculate(node); 
            }
            free_ast(node);
        }
    }

    
    TokenNode *temp;
    while (tokens != NULL) {
        temp = tokens;
        tokens = tokens->next;
        free(temp);
    }
}


int main(int argc, char *argv[]) {
    char input[256];

    if (argc > 1) {
       
        FILE *file = fopen(argv[1], "r");
        if (file == NULL) {
            printf("Error : can't open the file\n");
            return 1;
        }
        while (fgets(input, sizeof(input), file) != NULL) {
            interpret(input);
        }
        fclose(file);
    } else {
        
        while (1) {
            printf(">> ");
            if (fgets(input, sizeof(input), stdin) == NULL) {
                break;
            }
            if (strcmp(input, "exit\n") == 0) {
                
                break;
            }
            interpret(input);
        }
    }

    return 0;
}
