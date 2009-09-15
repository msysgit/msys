#ifndef _str_tree
#define _str_tree

typedef struct str_tree {
  char *s;
  void *extra;
  struct str_tree *left, *right;
} str_tree;

str_tree *str_tree_add(str_tree **root, const char *s, void *extra);
str_tree *str_tree_find(str_tree *node, const char *s);

#endif /* _str_tree */
