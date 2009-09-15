#include <string.h>
#include <stdlib.h>

#include "str_tree.h"

static str_tree *
new_leaf(const char *s, void *extra)
{
  str_tree *leaf;
  leaf = (str_tree *) malloc(sizeof(str_tree));
  if (!leaf)
    return NULL;
  leaf->s = strdup(s);
  if (!leaf->s)
    {
      free(leaf);
      return NULL;
    }
  leaf->extra = extra;
  leaf->left = leaf->right = NULL;
  return leaf;
}

str_tree *
str_tree_add(str_tree **root, const char *s, void *extra)
{
  if (!*root)
    return (*root = new_leaf(s, extra));
  else if (strcmp(s, (*root)->s) < 0)
    return str_tree_add(&(*root)->left, s, extra);
  else
    return str_tree_add(&(*root)->right, s, extra);
}

str_tree *
str_tree_find(str_tree *node, const char *s)
{
  if (node == NULL)
    return NULL;
  if (strcmp(s, node->s) == 0)
    return node;
  else if (strcmp(s, node->s) < 0)
    return str_tree_find(node->left, s);
  else
    return str_tree_find(node->right, s);
}
