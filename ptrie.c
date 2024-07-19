#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

typedef struct Node {

    long termFrequencyCount;
    long termFrequencyCountChildMax;

    struct NodeChildren {
        char *key;
        struct Node *node;
    } *Children;

    size_t childrenSize;

} Node;

typedef struct {

    long termCount;
    long termCountLoaded;
    Node *trie;

} PruningRadixTrie;

char *strndup(const char *str, size_t n) {

    size_t s_length = strnlen(str, n);
    char *s = malloc(s_length + 1);

    if (!s) return NULL;

    memcpy(s, str, s_length);
    s[s_length] = '\0';

    return s;
}

Node *CreateNode(long termFrequencyCount) {

    Node *node = (Node *) malloc(sizeof(Node));
    node->termFrequencyCount = termFrequencyCount;
    node->termFrequencyCountChildMax = 0;
    node->Children = NULL;
    node->childrenSize = 0;
    return node;
}

PruningRadixTrie *CreatePruningRadixTrie() {

    PruningRadixTrie *trie = (PruningRadixTrie *) malloc(sizeof(PruningRadixTrie));
    trie->termCount = 0;
    trie->termCountLoaded = 0;
    trie->trie = CreateNode(0);
    return trie;
}

void UpdateMaxCounts(Node **nodeList, size_t nodeListSize, long termFrequencyCount) {

    for (size_t i = 0; i < nodeListSize; i++) {
        if (termFrequencyCount > nodeList[i]->termFrequencyCountChildMax) {
            nodeList[i]->termFrequencyCountChildMax = termFrequencyCount;
        }
    }
}

void
AddTerm(Node *curr, const char *term, long termFrequencyCount, int id, int level, Node **nodeList, size_t *nodeListSize,
        PruningRadixTrie *trie) {

    nodeList[*nodeListSize] = curr;
    (*nodeListSize)++;

    int common = 0;
    if (curr->Children != NULL) {
        for (size_t j = 0; j < curr->childrenSize; j++) {
            char *key = curr->Children[j].key;
            Node *node = curr->Children[j].node;

            size_t minLen = strlen(term) < strlen(key) ? strlen(term) : strlen(key);
            for (int i = 0; i < minLen; i++) {
                if (term[i] == key[i]) {
                    common = i + 1;
                } else {
                    break;
                }
            }

            if (common > 0) {
                if ((common == strlen(term)) && (common == strlen(key))) {
                    if (node->termFrequencyCount == 0) {
                        trie->termCount++;
                    }
                    node->termFrequencyCount += termFrequencyCount;
                    UpdateMaxCounts(nodeList, *nodeListSize, node->termFrequencyCount);
                } else if (common == strlen(term)) {
                    Node *child = CreateNode(termFrequencyCount);
                    child->Children = (struct NodeChildren *) malloc(sizeof(struct NodeChildren));
                    child->Children[0].key = strdup(key + common);
                    child->Children[0].node = node;
                    child->childrenSize = 1;
                    child->termFrequencyCountChildMax =
                            node->termFrequencyCount > node->termFrequencyCountChildMax ? node->termFrequencyCount
                                                                                        : node->termFrequencyCountChildMax;
                    UpdateMaxCounts(nodeList, *nodeListSize, termFrequencyCount);

                    curr->Children[j].key = strndup(term, common);
                    curr->Children[j].node = child;
                    trie->termCount++;
                } else if (common == strlen(key)) {
                    AddTerm(node, term + common, termFrequencyCount, id, level + 1, nodeList, nodeListSize, trie);
                } else {
                    Node *child = CreateNode(0);
                    child->Children = (struct NodeChildren *) malloc(2 * sizeof(struct NodeChildren));
                    child->Children[0].key = strdup(key + common);
                    child->Children[0].node = node;
                    child->Children[1].key = strdup(term + common);
                    child->Children[1].node = CreateNode(termFrequencyCount);
                    child->childrenSize = 2;
                    child->termFrequencyCountChildMax =
                            node->termFrequencyCount > termFrequencyCount ? (node->termFrequencyCount >
                                                                             node->termFrequencyCountChildMax
                                                                             ? node->termFrequencyCount
                                                                             : node->termFrequencyCountChildMax) : (
                                    termFrequencyCount > node->termFrequencyCountChildMax ? termFrequencyCount
                                                                                          : node->termFrequencyCountChildMax);
                    UpdateMaxCounts(nodeList, *nodeListSize, termFrequencyCount);

                    curr->Children[j].key = strndup(term, common);
                    curr->Children[j].node = child;
                    trie->termCount++;
                }
                return;
            }
        }
    }

    if (curr->Children == NULL) {
        curr->Children = (struct NodeChildren *) malloc(sizeof(struct NodeChildren));
        curr->Children[0].key = strdup(term);
        curr->Children[0].node = CreateNode(termFrequencyCount);
        curr->childrenSize = 1;
    } else {
        curr->Children = (struct NodeChildren *) realloc(curr->Children,
                                                         (curr->childrenSize + 1) * sizeof(struct NodeChildren));
        curr->Children[curr->childrenSize].key = strdup(term);
        curr->Children[curr->childrenSize].node = CreateNode(termFrequencyCount);
        curr->childrenSize++;
    }

    trie->termCount++;
    UpdateMaxCounts(nodeList, *nodeListSize, termFrequencyCount);
}

void AddTermToTrie(PruningRadixTrie *trie, const char *term, long termFrequencyCount) {

    Node *nodeList[1000];
    size_t nodeListSize = 0;
    AddTerm(trie->trie, term, termFrequencyCount, 0, 0, nodeList, &nodeListSize, trie);
}

int main() {

    PruningRadixTrie *trie = CreatePruningRadixTrie();
    AddTermToTrie(trie, "hello", 1);
    AddTermToTrie(trie, "world", 2);
    AddTermToTrie(trie, "hello", 1);
    AddTermToTrie(trie, "hell", 1);

    printf("Total terms: %ld\n", trie->termCount);

    return 0;
}

// TODO: extract and read from terms.zip to test
