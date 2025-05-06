#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_SYMBOLS 100
#define MAX_PRODS 100
#define MAX_PROD_LEN 50

typedef struct {
    char non_terminal;
    char productions[MAX_PRODS][MAX_PROD_LEN];
    int prod_count;
} NonTerminal;

typedef struct {
    char symbols[MAX_SYMBOLS];
    int count;
} SymbolSet;

NonTerminal grammar[MAX_SYMBOLS];
int grammar_size = 0;

SymbolSet first_sets[MAX_SYMBOLS];
SymbolSet follow_sets[MAX_SYMBOLS];

bool is_terminal(char c) {
    return !isupper(c) && c != '\0' && c != '|' && c != ' ' && c != '\t' && c != '\n';
}

bool is_non_terminal(char c) {
    return isupper(c);
}

int find_non_terminal_index(char c) {
    for (int i = 0; i < grammar_size; i++) {
        if (grammar[i].non_terminal == c) {
            return i;
        }
    }
    return -1;
}

void add_to_set(SymbolSet *set, char c) {
    if (c == '\0' || c == '|') return;

    for (int i = 0; i < set->count; i++) {
        if (set->symbols[i] == c) {
            return;
        }
    }

    set->symbols[set->count++] = c;
}

void union_sets(SymbolSet *dest, SymbolSet src) {
    for (int i = 0; i < src.count; i++) {
        add_to_set(dest, src.symbols[i]);
    }
}

bool set_contains(SymbolSet set, char c) {
    for (int i = 0; i < set.count; i++) {
        if (set.symbols[i] == c) {
            return true;
        }
    }
    return false;
}

void compute_first(char non_terminal);

void compute_first_for_string(char *str, SymbolSet *result) {
    if (str[0] == '\0' || str[0] == '|') {
        add_to_set(result, '\0');
        return;
    }

    char c = str[0];
    if (is_terminal(c)) {
        add_to_set(result, c);
        return;
    }

    int nt_index = find_non_terminal_index(c);
    if (nt_index == -1) {
        printf("Error: Non-terminal %c not found\n", c);
        exit(1);
    }

    SymbolSet first = first_sets[nt_index];
    if (first.count == 0) {
        compute_first(c);
        first = first_sets[nt_index];
    }

    union_sets(result, first);

    if (set_contains(first, '\0')) {
        compute_first_for_string(str + 1, result);
    }
}

void compute_first(char non_terminal) {
    int nt_index = find_non_terminal_index(non_terminal);
    if (nt_index == -1) {
        printf("Error: Non-terminal %c not found\n", non_terminal);
        exit(1);
    }

    if (first_sets[nt_index].count > 0) {
        return;
    }

    NonTerminal nt = grammar[nt_index];
    for (int i = 0; i < nt.prod_count; i++) {
        compute_first_for_string(nt.productions[i], &first_sets[nt_index]);
    }
}

void compute_follow(char non_terminal) {
    int nt_index = find_non_terminal_index(non_terminal);
    if (nt_index == -1) {
        printf("Error: Non-terminal %c not found\n", non_terminal);
        exit(1);
    }

    if (nt_index == 0) {
        add_to_set(&follow_sets[nt_index], '$');
    }

    for (int i = 0; i < grammar_size; i++) {
        for (int j = 0; j < grammar[i].prod_count; j++) {
            char *prod = grammar[i].productions[j];
            int len = strlen(prod);

            for (int k = 0; k < len; k++) {
                if (prod[k] == non_terminal) {
                    if (k + 1 < len) {
                        SymbolSet first_beta;
                        first_beta.count = 0;
                        compute_first_for_string(prod + k + 1, &first_beta);

                        bool has_epsilon = set_contains(first_beta, '\0');

                        for (int m = 0; m < first_beta.count; m++) {
                            if (first_beta.symbols[m] != '\0') {
                                add_to_set(&follow_sets[nt_index], first_beta.symbols[m]);
                            }
                        }

                        if (has_epsilon) {
                            if (grammar[i].non_terminal != non_terminal) {
                                int follow_i = find_non_terminal_index(grammar[i].non_terminal);
                                if (follow_i == -1) {
                                    printf("Error: Non-terminal %c not found\n", grammar[i].non_terminal);
                                    exit(1);
                                }

                                if (follow_sets[follow_i].count == 0) {
                                    compute_follow(grammar[i].non_terminal);
                                }
                                union_sets(&follow_sets[nt_index], follow_sets[follow_i]);
                            }
                        }
                    } else {
                        if (grammar[i].non_terminal != non_terminal) {
                            int follow_i = find_non_terminal_index(grammar[i].non_terminal);
                            if (follow_i == -1) {
                                printf("Error: Non-terminal %c not found\n", grammar[i].non_terminal);
                                exit(1);
                            }

                            if (follow_sets[follow_i].count == 0) {
                                compute_follow(grammar[i].non_terminal);
                            }
                            union_sets(&follow_sets[nt_index], follow_sets[follow_i]);
                        }
                    }
                }
            }
        }
    }
}

void print_set(SymbolSet set) {
    printf("{ ");
    for (int i = 0; i < set.count; i++) {
        printf("%c ", set.symbols[i]);
    }
    printf("}");
}

void print_first_sets() {
    printf("FIRST sets:\n");
    for (int i = 0; i < grammar_size; i++) {
        printf("FIRST(%c) = ", grammar[i].non_terminal);
        print_set(first_sets[i]);
        printf("\n");
    }
}

void print_follow_sets() {
    printf("FOLLOW sets:\n");
    for (int i = 0; i < grammar_size; i++) {
        printf("FOLLOW(%c) = ", grammar[i].non_terminal);
        print_set(follow_sets[i]);
        printf("\n");
    }
}

void initialize_grammar() {
    grammar_size = 5;

    grammar[0].non_terminal = 'E';
    strcpy(grammar[0].productions[0], "TX");
    grammar[0].prod_count = 1;

    grammar[1].non_terminal = 'X';
    strcpy(grammar[1].productions[0], "+TX");
    strcpy(grammar[1].productions[1], "");
    grammar[1].prod_count = 2;

    grammar[2].non_terminal = 'T';
    strcpy(grammar[2].productions[0], "FY");
    grammar[2].prod_count = 1;

    grammar[3].non_terminal = 'Y';
    strcpy(grammar[3].productions[0], "*FY");
    strcpy(grammar[3].productions[1], "");
    grammar[3].prod_count = 2;

    grammar[4].non_terminal = 'F';
    strcpy(grammar[4].productions[0], "(E)");
    strcpy(grammar[4].productions[1], "i");
    grammar[4].prod_count = 2;
}

int main() {
    initialize_grammar();

    for (int i = 0; i < grammar_size; i++) {
        first_sets[i].count = 0;
        follow_sets[i].count = 0;
    }

    for (int i = 0; i < grammar_size; i++) {
        compute_first(grammar[i].non_terminal);
    }
    print_first_sets();

    for (int i = 0; i < grammar_size; i++) {
        compute_follow(grammar[i].non_terminal);
    }
    print_follow_sets();

    return 0;
}
